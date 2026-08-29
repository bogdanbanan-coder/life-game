#pragma once

#include <variant>

#include <domain/field/cell-coordinate.hpp>

namespace lifeGame::application {

    enum class PaintMode {
        Live,
        Die,
        Move,
    };

    struct PaintLiveCommand {
        domain::CellCoordinate coordinate;
    };

    struct PaintDeadCommand {
        domain::CellCoordinate coordinate;
    };

    using PaintCommand = std::variant<PaintLiveCommand, PaintDeadCommand>;

    struct PanCameraCommand {
        float deltaX;
        float deltaY;
    };

} // namespace lifeGame::application
