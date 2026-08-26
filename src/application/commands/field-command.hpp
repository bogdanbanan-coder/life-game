#pragma once

#include <domain/field/cell-coordinate.hpp>

namespace lifeGame::application {

    struct PaintLiveCommand {
        domain::CellCoordinate coordinate;
    };

} // namespace lifeGame::application
