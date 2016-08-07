#pragma once
//------------------------------------------------------------------------------
/**
    @class Oryol::NKUISetup.h
    @brief configuration settings for Nuklear UI
*/
#include "Core/Types.h"

namespace Oryol {

class NKUISetup {
public:
    /// global UI alpha value
    float GlobalAlpha = 1.0f;
    /// antialiased lines?
    bool LineAA = true;
    /// antialiased shaped?
    bool ShapeAA = true;
    /// circle segment count (for vectorization)
    int CircleSegmentCount = 22;
    /// curve segment count (for vectorization)
    int CurveSegmentCount = 22;
    /// arc segment count (for vectorization)
    int ArcSegmentCount = 22;
};

} // namespace Oryol
