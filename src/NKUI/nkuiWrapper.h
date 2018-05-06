#pragma once
//------------------------------------------------------------------------------
/**
    @class Oryol::_priv::nkuiWrapper
    @brief internal nuklear UI wrapper
*/
#include "Core/Types.h"
#include "Core/Assertion.h"
#include "Core/Containers/StaticArray.h"
#include "Core/Containers/MemoryBuffer.h"
#include "NKUI/NKUISetup.h"
#include "Gfx/Gfx.h"

#if __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
#include <stdio.h>
#include "NKUI/nuklear_config.h"
#include "nuklear/nuklear.h"
#if __GNUC__
#pragma GCC diagnostic pop
#endif

namespace Oryol {
namespace _priv {

class nkuiWrapper {
public:
    /// setup the wrapper
    void Setup(const NKUISetup& setup);
    /// discard the wrapper
    void Discard();
    /// start a new frame
    void NewFrame();
    /// draw current frame
    void Draw();
    /// grab a new image handle
    struct nk_image AllocImage();
    /// free an image handle
    void FreeImage(const struct nk_image& image);
    /// bind an Oryol texture to an image handle
    void BindImage(const struct nk_image& image, Id texId);
    /// begin a font atlas
    void BeginFontAtlas();
    /// add a font to current font atlas
    nk_font* AddFont(const MemoryBuffer& ttfData, float fontHeight);
    /// end defining font atlas
    void EndFontAtlas();

    nk_context ctx;

private:
    /// create Oryol render resources
    void createResources(const NKUISetup& setup);

    static const int MaxNumVertices = 64 * 1024;
    static const int MaxNumIndices = 128 * 1024;
    static const int MaxNumFontAtlases = 4;

    bool isValid = false;
    nk_font_atlas defaultAtlas;
    nk_font* defaultFont = nullptr;
    nk_buffer cmds;
    nk_buffer vbuf;
    nk_buffer ibuf;
    nk_convert_config config;

    ResourceLabel gfxResLabel;
    Id whiteTexture;
    DrawState drawState;
    static const int MaxImages = 256;
    StaticArray<Id, MaxImages> images;
    Array<int> freeImageSlots;
    int curFontAtlas = 0;
    StaticArray<nk_font_atlas, MaxNumFontAtlases> fontAtlases;

    struct nk_draw_vertex vertexData[MaxNumVertices];
    nk_draw_index indexData[MaxNumIndices];
};

} // namespace _priv
} // namespace Oryol
