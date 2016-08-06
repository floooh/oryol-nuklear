#pragma once
//------------------------------------------------------------------------------
/**
    @class Oryol::_priv::nkuiWrapper
    @brief internal nuklear UI wrapper
*/
#include "Core/Types.h"
#include "Core/Assertion.h"
#include "Core/Containers/StaticArray.h"
#include "NKUI/NKUISetup.h"
#include "Gfx/Gfx.h"

#if __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
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

    nk_context ctx;

private:
    /// create the default font
    void createDefaultFont(const NKUISetup& setup);
    /// create dynamic mesh and pipeline object
    void createMeshAndPipeline();

    static const int MaxNumVertices = 64 * 1024;
    static const int MaxNumIndices = 128 * 1024;

    bool isValid = false;
    nk_font_atlas atlas;
    nk_font* defaultFont = nullptr;
    nk_draw_null_texture nullTex;
    nk_buffer cmds;

    ResourceLabel gfxResLabel;
    static const int MaxTextures = 16;
    int curTexId = 0;
    StaticArray<Id, MaxTextures> textures;
    DrawState drawState;

    struct nk_draw_vertex vertexData[MaxNumVertices];
    nk_draw_index indexData[MaxNumIndices];
};

} // namespace _priv
} // namespace Oryol
