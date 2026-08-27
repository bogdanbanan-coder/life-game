#pragma once

#include <optional>
#include <vector>

#include <application/commands/field-command.hpp>
#include <domain/field/field.hpp>
#include <presentation/camera/coordinate-converter.hpp>

namespace lifeGame::presentation {

    struct PointerSample {
        LogicalPoint position;
        bool pressed;
        bool down;
        bool released;
    };

    struct InputCommands {
        std::optional<application::PaintMode> selectedPaintMode;
        std::vector<application::PaintCommand> paintCommands;
    };

    class InputRouter {
      public:
        [[nodiscard]] auto sample(const domain::Field& field, int viewportWidth,
                                  int viewportHeight, PointerSample pointer,
                                  bool modalOwnsInput = false)
            -> std::vector<application::PaintLiveCommand>;
        [[nodiscard]] auto sample(const domain::Field& field, int viewportWidth,
                                  int viewportHeight, PointerSample pointer,
                                  application::PaintMode paintMode,
                                  bool modalOwnsInput = false) -> InputCommands;

      private:
        [[nodiscard]] auto sampleForMode(const domain::Field& field, int viewportWidth,
                                         int viewportHeight, PointerSample pointer,
                                         application::PaintMode paintMode,
                                         bool modalOwnsInput) -> InputCommands;
        [[nodiscard]] bool isToolbarOwner(LogicalPoint point, int viewportWidth,
                                          int viewportHeight) const noexcept;
        void clearGesture() noexcept;

        bool fieldGestureActive_ = false;
        std::optional<domain::CellCoordinate> lastCell_;
    };

} // namespace lifeGame::presentation
