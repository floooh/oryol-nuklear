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

namespace Oryol {

class NKUI {
public:
    /// setup the NKUI module
    static void Setup();
    /// shutdown the NKUI module
    static void Discard();
    /// test if NKUI module has been setup
    static bool IsValid();
}

} // namespace NKUI

