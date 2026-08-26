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

    class InputRouter {
      public:
        [[nodiscard]] auto sample(const domain::Field& field, int viewportWidth,
                                  int viewportHeight, PointerSample pointer,
                                  bool modalOwnsInput = false)
            -> std::vector<application::PaintLiveCommand>;

      private:
        [[nodiscard]] bool isToolbarOwner(LogicalPoint point, int viewportWidth,
                                          int viewportHeight) const noexcept;
        void clearGesture() noexcept;

        bool fieldGestureActive_ = false;
        std::optional<domain::CellCoordinate> lastCell_;
    };

} // namespace lifeGame::presentation
