#pragma once

#include <vector>

#include <domain/field/cell-coordinate.hpp>

namespace lifeGame::presentation {

    class DragRasterizer {
      public:
        // Returns the inclusive Bresenham path in from-to order. Reversing the endpoints
        // reverses the same cell set, and spans beyond the field safety limit are rejected.
        [[nodiscard]] static auto rasterize(domain::CellCoordinate from,
                                            domain::CellCoordinate to)
            -> std::vector<domain::CellCoordinate>;
    };

} // namespace lifeGame::presentation
