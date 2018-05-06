#pragma once
//------------------------------------------------------------------------------
/**
    @defgroup NKUI NKUI
    @brief Oryol wrapper module for Nuklear UI

    @class Oryol::NKUI
    @ingroup NKUI
    @brief facade of the NKUI module
*/
#include "Core/Types.h"
#include "NKUI/nkuiWrapper.h"
#include "NKUI/NKUISetup.h"

namespace Oryol {

class NKUI {
public:
    /// setup the NKUI module
    static void Setup(const NKUISetup& setup = NKUISetup());
    /// shutdown the NKUI module
    static void Discard();
    /// test if NKUI module has been setup
    static bool IsValid();

    /// start a new frame (handles nuklear input)
    static nk_context* NewFrame();
    /// get pointer to nuklear context (if needed away from NewFrame())
    static nk_context* Ctx();
    /// draw the nuklear UI
    static void Draw();

    /// allocate a new image handle
    static struct nk_image AllocImage();
    /// free an image handle
    static void FreeImage(const struct nk_image& image);
    /// bind an Oryol texture to an image handle
    static void BindImage(const struct nk_image& image, Id texId);

    /// begin a font atlas
    static void BeginFontAtlas();
    /// add a font to current font atlas
    static nk_font* AddFont(const MemoryBuffer& ttfData, float fontHeight);
    /// end defining font atlas
    static void EndFontAtlas();

private:
    struct _state {
        _priv::nkuiWrapper nkuiWrapper;
    };
    static _state* state;
};

//------------------------------------------------------------------------------
inline nk_context*
NKUI::Ctx() {
    o_assert_dbg(state);
    return &state->nkuiWrapper.ctx;
}

} // namespace NKUI

