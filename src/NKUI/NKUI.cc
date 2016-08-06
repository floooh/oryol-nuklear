//------------------------------------------------------------------------------
//  NKUI.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "NKUI.h"
#include "Core/Assertion.h"
#include "Core/Memory/Memory.h"

namespace Oryol {

NKUI::_state* NKUI::state = nullptr;

//------------------------------------------------------------------------------
void
NKUI::Setup(const NKUISetup& setup) {
    o_assert_dbg(!IsValid());
    state = Memory::New<_state>();
    state->nkuiWrapper.Setup(setup);
}

//------------------------------------------------------------------------------
void
NKUI::Discard() {
    o_assert_dbg(IsValid());
    state->nkuiWrapper.Discard();
    Memory::Delete(state);
    state = nullptr;
}

//------------------------------------------------------------------------------
bool
NKUI::IsValid() {
    return nullptr != state;
}

//------------------------------------------------------------------------------
nk_context*
NKUI::NewFrame() {
    o_assert_dbg(IsValid());
    state->nkuiWrapper.NewFrame();
    return &state->nkuiWrapper.ctx;
}

//------------------------------------------------------------------------------
void
NKUI::Draw() {
    o_assert_dbg(IsValid());
    state->nkuiWrapper.Draw();
}

} // namespace Oryol

