//------------------------------------------------------------------------------
//  nkuiWrapper.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#define NK_IMPLEMENTATION
#include "nkuiWrapper.h"
#include "Core/Memory/Memory.h"
#include "Input/Input.h"
#include "NKUIShaders.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace Oryol {
namespace _priv {

//------------------------------------------------------------------------------
void
nkuiWrapper::Setup(const NKUISetup& setup) {
    o_assert_dbg(!this->isValid);
    this->isValid = true;

    Memory::Clear(&this->config, sizeof(this->config));
    this->config.global_alpha = setup.GlobalAlpha;
    this->config.line_AA = setup.LineAA ? NK_ANTI_ALIASING_ON:NK_ANTI_ALIASING_OFF;
    this->config.shape_AA = setup.ShapeAA ? NK_ANTI_ALIASING_ON:NK_ANTI_ALIASING_OFF;
    this->config.circle_segment_count = setup.CircleSegmentCount;
    this->config.curve_segment_count = setup.CurveSegmentCount;
    this->config.arc_segment_count = setup.ArcSegmentCount;

    this->freeImageSlots.Reserve(MaxImages);
    for (int i = MaxImages-1; i>=0; i--) {
        this->freeImageSlots.Add(i);
    }

    this->createResources(setup);

    nk_init_default(&this->ctx, &this->defaultFont->handle);
    nk_buffer_init_default(&this->cmds);
    nk_buffer_init_fixed(&vbuf, this->vertexData, sizeof(this->vertexData));
    nk_buffer_init_fixed(&ibuf, this->indexData, sizeof(this->indexData));
}

//------------------------------------------------------------------------------
void
nkuiWrapper::Discard() {
    o_assert_dbg(this->isValid);
    this->defaultFont = nullptr;
    nk_buffer_free(&this->cmds);
    nk_font_atlas_clear(&this->defaultAtlas);
    nk_free(&this->ctx);
    Gfx::DestroyResources(this->gfxResLabel);
    this->isValid = false;
}

//------------------------------------------------------------------------------
void
nkuiWrapper::createResources(const NKUISetup& setup) {
    o_assert_dbg(nullptr == this->defaultFont);

    // push a new resource label and store it (needed to destroy resources later)
    this->gfxResLabel = Gfx::PushResourceLabel();

    // create a white placeholder texture for images that have been allocated
    // but not yet bound, this allows to load images asynchronously, and
    // already draw UI with a white placeholder texture even though
    // the actual image data hasn't finished loading yet
    const int w = 4;
    const int h = 4;
    uint32 pixels[w * h];
    Memory::Fill(pixels, sizeof(pixels), 0xFF);
    this->whiteTexture = Gfx::CreateTexture(TextureDesc()
        .Type(TextureType::Texture2D)
        .Width(w)
        .Height(h)
        .NumMipMaps(1)
        .Format(PixelFormat::RGBA8)
        .WrapU(TextureWrapMode::Repeat)
        .WrapV(TextureWrapMode::Repeat)
        .MinFilter(TextureFilterMode::Nearest)
        .MagFilter(TextureFilterMode::Nearest)
        .MipSize(0, 0, sizeof(pixels))
        .MipContent(0, 0, pixels));

    // render default font into an Oryol texture
    nk_font_atlas_init_default(&this->defaultAtlas);
    nk_font_atlas_begin(&this->defaultAtlas);
    this->defaultFont = nk_font_atlas_add_default(&this->defaultAtlas, setup.DefaultFontHeight, 0);
    int imgWidth, imgHeight;
    const void* imgData = nk_font_atlas_bake(&this->defaultAtlas, &imgWidth, &imgHeight, NK_FONT_ATLAS_RGBA32);
    const int imgSize = imgWidth*imgHeight * PixelFormat::ByteSize(PixelFormat::RGBA8);
    Id fontTex = Gfx::CreateTexture(TextureDesc()
        .Type(TextureType::Texture2D)
        .Width(imgWidth)
        .Height(imgHeight)
        .NumMipMaps(1)
        .Format(PixelFormat::RGBA8)
        .WrapU(TextureWrapMode::ClampToEdge)
        .WrapV(TextureWrapMode::ClampToEdge)
        .MinFilter(TextureFilterMode::Nearest)
        .MagFilter(TextureFilterMode::Nearest)
        .MipSize(0, 0, imgSize)
        .MipContent(0, 0, imgData));
    struct nk_image img = this->AllocImage();
    this->BindImage(img, fontTex);
    nk_font_atlas_end(&this->defaultAtlas, img.handle, &this->config.null);

    // create dynamic vertex- and index-buffer
    this->drawState.VertexBuffers[0] = Gfx::CreateBuffer(BufferDesc()
        .Type(BufferType::VertexBuffer)
        .Size(MaxNumVertices * sizeof(struct nk_draw_vertex))
        .Usage(Usage::Stream));
    this->drawState.IndexBuffer = Gfx::CreateBuffer(BufferDesc()
        .Type(BufferType::IndexBuffer)
        .Size(MaxNumIndices * sizeof(uint16_t))
        .Usage(Usage::Stream));

    // create pipeline state object
    Id shd = Gfx::CreateShader(NKUIShader::Desc());
    this->drawState.Pipeline = Gfx::CreatePipeline(PipelineDesc()
        .Shader(shd)
        .Layout(0, {
            { "position", VertexFormat::Float2 },
            { "texcoord0", VertexFormat::Float2 },
            { "color0", VertexFormat::UByte4N }
        })
        .IndexType(IndexType::UInt16)
        .DepthWriteEnabled(false)
        .DepthCmpFunc(CompareFunc::Always)
        .BlendEnabled(true)
        .BlendSrcFactorRGB(BlendFactor::SrcAlpha)
        .BlendDstFactorRGB(BlendFactor::OneMinusSrcAlpha)
        .ColorWriteMask(PixelChannel::RGB)
        .CullFaceEnabled(false)
        .ColorFormat(Gfx::DisplayAttrs().ColorFormat)
        .DepthFormat(Gfx::DisplayAttrs().DepthFormat)
        .SampleCount(Gfx::DisplayAttrs().SampleCount));
    Gfx::PopResourceLabel();
}

//------------------------------------------------------------------------------
struct nk_image
nkuiWrapper::AllocImage() {
    o_assert_dbg(!this->freeImageSlots.Empty());
    return nk_image_id(this->freeImageSlots.PopBack());
}

//------------------------------------------------------------------------------
void
nkuiWrapper::FreeImage(const struct nk_image& image) {
    int slot = image.handle.id;
    o_assert_dbg(this->images[slot].IsValid());
    this->images[slot].Invalidate();
}

//------------------------------------------------------------------------------
void
nkuiWrapper::BindImage(const struct nk_image& image, Id texId) {
    int slot = image.handle.id;
    o_assert_dbg(!this->images[slot].IsValid());
    this->images[slot] = texId;
}

//------------------------------------------------------------------------------
void
nkuiWrapper::BeginFontAtlas() {
    nk_font_atlas* atlas = &this->fontAtlases[this->curFontAtlas];
    nk_font_atlas_init_default(atlas);
    nk_font_atlas_begin(atlas);
}

//------------------------------------------------------------------------------
nk_font*
nkuiWrapper::AddFont(const MemoryBuffer& ttfData, float fontHeight) {
    nk_font_atlas* atlas = &this->fontAtlases[this->curFontAtlas];
    struct nk_font_config cfg = nk_font_config(0);
    cfg.oversample_h = 3; cfg.oversample_v = 2;
    nk_font* font = nk_font_atlas_add_from_memory(atlas, (void*)ttfData.Data(), ttfData.Size(), fontHeight, &cfg);
    return font;
}

//------------------------------------------------------------------------------
void
nkuiWrapper::EndFontAtlas() {
    nk_font_atlas* atlas = &this->fontAtlases[this->curFontAtlas++];
    int imgWidth, imgHeight;
    const void* imgData = nk_font_atlas_bake(atlas, &imgWidth, &imgHeight, NK_FONT_ATLAS_RGBA32);
    const int imgSize = imgWidth*imgHeight * PixelFormat::ByteSize(PixelFormat::RGBA8);
    Id tex = Gfx::CreateTexture(TextureDesc()
        .Type(TextureType::Texture2D)
        .Width(imgWidth)
        .Height(imgHeight)
        .NumMipMaps(1)
        .Format(PixelFormat::RGBA8)
        .WrapU(TextureWrapMode::ClampToEdge)
        .WrapV(TextureWrapMode::ClampToEdge)
        .MinFilter(TextureFilterMode::Nearest)
        .MagFilter(TextureFilterMode::Nearest)
        .MipSize(0, 0, imgSize)
        .MipContent(0, 0, imgData));
    struct nk_image img = this->AllocImage();
    this->BindImage(img, tex);
    nk_font_atlas_end(atlas, img.handle, &this->config.null);
}

//------------------------------------------------------------------------------
void
nkuiWrapper::NewFrame() {
    nk_input_begin(&this->ctx);
    if (Input::KeyboardAttached()) {
        nk_input_key(&this->ctx, NK_KEY_DEL, Input::KeyDown(Key::Delete));
        nk_input_key(&this->ctx, NK_KEY_ENTER, Input::KeyDown(Key::Enter));
        nk_input_key(&this->ctx, NK_KEY_TAB, Input::KeyDown(Key::Tab));
        nk_input_key(&this->ctx, NK_KEY_BACKSPACE, Input::KeyDown(Key::BackSpace));
        nk_input_key(&this->ctx, NK_KEY_LEFT, Input::KeyDown(Key::Left));
        nk_input_key(&this->ctx, NK_KEY_RIGHT, Input::KeyDown(Key::Right));
        nk_input_key(&this->ctx, NK_KEY_UP, Input::KeyDown(Key::Up));
        nk_input_key(&this->ctx, NK_KEY_DOWN, Input::KeyDown(Key::Down));
        if (Input::KeyPressed(Key::LeftControl) || Input::KeyPressed(Key::RightControl)) {
            nk_input_key(&this->ctx, NK_KEY_COPY, Input::KeyDown(Key::C));
            nk_input_key(&this->ctx, NK_KEY_PASTE, Input::KeyDown(Key::V));
            nk_input_key(&this->ctx, NK_KEY_CUT, Input::KeyDown(Key::X));
        }
        else {
            nk_input_key(&this->ctx, NK_KEY_COPY, 0);
            nk_input_key(&this->ctx, NK_KEY_PASTE, 0);
            nk_input_key(&this->ctx, NK_KEY_CUT, 0);
        }
        const wchar_t* text = Input::Text();
        while (wchar_t c = *text++) {
            nk_input_unicode(&this->ctx, nk_rune(c));
        }
    }
    if (Input::MouseAttached()) {
        const glm::vec2& mousePos = Input::MousePosition();
        int x = int(mousePos.x);
        int y = int(mousePos.y);
        bool lmb = Input::MouseButtonPressed(MouseButton::Left)|Input::MouseButtonDown(MouseButton::Left);
        bool mmb = Input::MouseButtonPressed(MouseButton::Middle)|Input::MouseButtonDown(MouseButton::Middle);
        bool rmb = Input::MouseButtonPressed(MouseButton::Right)|Input::MouseButtonDown(MouseButton::Right);
        nk_input_motion(&this->ctx, x, y);
        nk_input_scroll(&this->ctx, Input::MouseScroll().y);
        nk_input_button(&this->ctx, NK_BUTTON_LEFT, x, y, lmb);
        nk_input_button(&this->ctx, NK_BUTTON_MIDDLE, x, y, mmb);
        nk_input_button(&this->ctx, NK_BUTTON_RIGHT, x, y, rmb);
    }
    nk_input_end(&this->ctx);
}

//------------------------------------------------------------------------------
void
nkuiWrapper::Draw() {

    // generate and upload vertex and index data
    nk_buffer_clear(&this->vbuf);
    nk_buffer_clear(&this->ibuf);
    nk_convert(&this->ctx, &this->cmds, &this->vbuf, &this->ibuf, &this->config);
    if (this->vbuf.needed > 0) {
        Gfx::UpdateBuffer(this->drawState.VertexBuffers[0], this->vertexData, int(this->vbuf.needed));
    }
    if (this->ibuf.needed > 0) {
        Gfx::UpdateBuffer(this->drawState.IndexBuffer, this->indexData, int(this->ibuf.needed));
    }

    // compute projection matrix
    const DisplayAttrs& attrs = Gfx::DisplayAttrs();
    const float width = float(attrs.Width);
    const float height = float(attrs.Height);
    NKUIShader::vsParams vsParams;
    vsParams.proj = glm::ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f);

    // render draw commands
    Id curTexture;
    const struct nk_draw_command* cmd = nullptr;
    int elm_offset = 0;
    nk_draw_foreach(cmd, &this->ctx, &this->cmds) {
        int texSlot = int(cmd->texture.id);
        const Id& newTexture = this->images[texSlot];
        if (curTexture != newTexture) {
            if (newTexture.IsValid()) {
                this->drawState.FSTexture[NKUIShader::tex] = newTexture;
            }
            else {
                this->drawState.FSTexture[NKUIShader::tex] = this->whiteTexture;
            }
            curTexture = newTexture;
            Gfx::ApplyDrawState(this->drawState);
            Gfx::ApplyUniformBlock(vsParams);
        }
        Gfx::ApplyScissorRect((int)cmd->clip_rect.x,
                              (int)(height - (cmd->clip_rect.y + cmd->clip_rect.h)),
                              (int)(cmd->clip_rect.w),
                              (int)(cmd->clip_rect.h));
        Gfx::Draw(PrimitiveGroup(elm_offset, cmd->elem_count));
        elm_offset += cmd->elem_count;
    }
    nk_clear(&this->ctx);
}

} // namespace _priv
} // namespace Oryol
