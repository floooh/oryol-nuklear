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

    // create resources
    this->gfxResLabel = Gfx::PushResourceLabel();
    this->createDefaultFont(setup);
    this->createMeshAndPipeline();
    Gfx::PopResourceLabel();

    // initialize nuklear
    nk_init_default(&this->ctx, &this->defaultFont->handle);
    nk_buffer_init_default(&this->cmds);
}

//------------------------------------------------------------------------------
void
nkuiWrapper::Discard() {
    o_assert_dbg(this->isValid);
    this->defaultFont = nullptr;
    nk_buffer_free(&this->cmds);
    nk_font_atlas_clear(&this->atlas);
    nk_free(&this->ctx);
    Gfx::DestroyResources(this->gfxResLabel);
    this->isValid = false;
}

//------------------------------------------------------------------------------
void
nkuiWrapper::createDefaultFont(const NKUISetup& setup) {
    o_assert_dbg(nullptr == this->defaultFont);

    // let Nuklear create a font bitmap
    nk_font_atlas_init_default(&this->atlas);
    nk_font_atlas_begin(&this->atlas);
    this->defaultFont = nk_font_atlas_add_default(&this->atlas, 13, 0);
    int imgWidth, imgHeight;
    const void* imgData = nk_font_atlas_bake(&this->atlas, &imgWidth, &imgHeight, NK_FONT_ATLAS_RGBA32);

    // create Oryol Gfx texture from font image data
    auto texSetup = TextureSetup::FromPixelData(imgWidth, imgHeight, 1, TextureType::Texture2D, PixelFormat::RGBA8);
    texSetup.Sampler.WrapU = TextureWrapMode::ClampToEdge;
    texSetup.Sampler.WrapV = TextureWrapMode::ClampToEdge;
    texSetup.Sampler.MinFilter = TextureFilterMode::Nearest;
    texSetup.Sampler.MagFilter = TextureFilterMode::Nearest;
    const int imgSize = imgWidth*imgHeight * PixelFormat::ByteSize(PixelFormat::RGBA8);
    texSetup.ImageData.Sizes[0][0] = imgSize;
    int texId = this->curTexId++;
    this->textures[texId] = Gfx::CreateResource(texSetup, imgData, imgSize);
    nk_font_atlas_end(&this->atlas, nk_handle_id(texId), &this->nullTex);
}

//------------------------------------------------------------------------------
void
nkuiWrapper::createMeshAndPipeline() {

    // create mesh with dynamic vertex- and index-buffer
    auto mshSetup = MeshSetup::Empty(MaxNumVertices, Usage::Stream, IndexType::Index16, MaxNumIndices, Usage::Stream);
    mshSetup.Layout
        .Add(VertexAttr::Position, VertexFormat::Float2)
        .Add(VertexAttr::TexCoord0, VertexFormat::Float2)
        .Add(VertexAttr::Color0, VertexFormat::UByte4N);
    o_assert_dbg(mshSetup.Layout.ByteSize() == sizeof(struct nk_draw_vertex));
    this->drawState.Mesh[0] = Gfx::CreateResource(mshSetup);

    // create pipeline state object
    Id shd = Gfx::CreateResource(NKUIShader::Setup());
    auto ps = PipelineSetup::FromLayoutAndShader(mshSetup.Layout, shd);
    ps.DepthStencilState.DepthWriteEnabled = false;
    ps.DepthStencilState.DepthCmpFunc = CompareFunc::Always;
    ps.BlendState.BlendEnabled = true;
    ps.BlendState.SrcFactorRGB = BlendFactor::SrcAlpha;
    ps.BlendState.DstFactorRGB = BlendFactor::OneMinusSrcAlpha;
    ps.BlendState.ColorFormat = Gfx::DisplayAttrs().ColorPixelFormat;
    ps.BlendState.DepthFormat = Gfx::DisplayAttrs().DepthPixelFormat;
    ps.BlendState.ColorWriteMask = PixelChannel::RGB;
    ps.RasterizerState.ScissorTestEnabled = true;
    ps.RasterizerState.CullFaceEnabled = false;
    ps.RasterizerState.SampleCount = Gfx::DisplayAttrs().SampleCount;
    this->drawState.Pipeline = Gfx::CreateResource(ps);
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
    }
    if (Input::MouseAttached()) {
        const glm::vec2& mousePos = Input::MousePosition();
        int x = int(mousePos.x);
        int y = int(mousePos.y);
        bool lmb = Input::MouseButtonPressed(MouseButton::Left)|Input::MouseButtonDown(MouseButton::Left);
        bool mmb = Input::MouseButtonPressed(MouseButton::Middle)|Input::MouseButtonDown(MouseButton::Middle);
        bool rmb = Input::MouseButtonPressed(MouseButton::Right)|Input::MouseButtonDown(MouseButton::Right);
        nk_input_motion(&this->ctx, x, y);
        nk_input_button(&this->ctx, NK_BUTTON_LEFT, x, y, lmb);
        nk_input_button(&this->ctx, NK_BUTTON_MIDDLE, x, y, mmb);
        nk_input_button(&this->ctx, NK_BUTTON_RIGHT, x, y, rmb);
    }
    nk_input_end(&this->ctx);
}

//------------------------------------------------------------------------------
void
nkuiWrapper::Draw() {

    // upload vertex and index data
    nk_convert_config config = { };
    config.global_alpha = 1.0f;
    config.shape_AA = NK_ANTI_ALIASING_ON;
    config.line_AA = NK_ANTI_ALIASING_ON;
    config.circle_segment_count = 22;
    config.curve_segment_count = 22;
    config.arc_segment_count = 22;
    config.null = this->nullTex;
    struct nk_buffer vbuf, ibuf;
    nk_buffer_init_fixed(&vbuf, this->vertexData, sizeof(this->vertexData));
    nk_buffer_init_fixed(&ibuf, this->indexData, sizeof(this->indexData));
    nk_convert(&this->ctx, &this->cmds, &vbuf, &ibuf, &config);
    if (vbuf.needed > 0) {
        Gfx::UpdateVertices(this->drawState.Mesh[0], this->vertexData, vbuf.needed);
    }
    if (ibuf.needed > 0) {
        Gfx::UpdateIndices(this->drawState.Mesh[0], this->indexData, ibuf.needed);
    }

    // compute projection matrix
    const DisplayAttrs& attrs = Gfx::DisplayAttrs();
    const float width = float(attrs.FramebufferWidth);
    const float height = float(attrs.FramebufferHeight);
    NKUIShader::VSParams vsParams;
    vsParams.Proj = glm::ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f);

    // render draw commands
    Id curTexture;
    const struct nk_draw_command* cmd = nullptr;
    int elm_offset = 0;
    nk_draw_foreach(cmd, &this->ctx, &this->cmds) {
        const Id& newTexture = this->textures[(int)cmd->texture.id];
        if (curTexture != newTexture) {
            this->drawState.FSTexture[NKUITextures::Texture] = newTexture;
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
