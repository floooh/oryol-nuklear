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
    /// access to Nuklear context (needed for calling nuklear UI functions)
    static nk_context* Ctx();
    /// start a new frame (handles nuklear input)
    static nk_context* NewFrame();
    /// draw the nuklear UI
    static void Draw();

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

