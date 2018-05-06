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

//------------------------------------------------------------------------------
struct nk_image
NKUI::AllocImage() {
    o_assert_dbg(IsValid());
    return state->nkuiWrapper.AllocImage();
}

//------------------------------------------------------------------------------
void
NKUI::FreeImage(const struct nk_image& image) {
    o_assert_dbg(IsValid());
    state->nkuiWrapper.FreeImage(image);
}

//------------------------------------------------------------------------------
void
NKUI::BindImage(const struct nk_image& image, Id texId) {
    o_assert_dbg(IsValid());
    state->nkuiWrapper.BindImage(image, texId);
}

//------------------------------------------------------------------------------
void
NKUI::BeginFontAtlas() {
    o_assert_dbg(IsValid());
    state->nkuiWrapper.BeginFontAtlas();
}

//------------------------------------------------------------------------------
nk_font*
NKUI::AddFont(const MemoryBuffer& ttfData, float fontHeight) {
    o_assert_dbg(IsValid());
    return state->nkuiWrapper.AddFont(ttfData, fontHeight);
}

//------------------------------------------------------------------------------
void
NKUI::EndFontAtlas() {
    o_assert_dbg(IsValid());
    state->nkuiWrapper.EndFontAtlas();
}

} // namespace Oryol

