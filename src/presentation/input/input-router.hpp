#pragma once

#include <optional>
#include <vector>

#include <application/commands/field-command.hpp>
#include <application/commands/run-command.hpp>
#include <domain/field/field.hpp>
#include <presentation/camera/camera-controller.hpp>
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
        std::optional<application::PauseCommand> pauseRequest;
        std::optional<application::ResumeCommand> resumeRequest;
        std::optional<application::ExitSessionCommand> exitRequest;
        std::vector<application::PaintCommand> paintCommands;
        std::vector<application::PanCameraCommand> panCommands;
    };

    class InputRouter {
      public:
        [[nodiscard]] auto sample(const domain::Field& field, int viewportWidth,
                                  int viewportHeight, PointerSample pointer,
                                  bool modalOwnsInput = false)
            -> std::vector<application::PaintLiveCommand>;
        // Compatibility overload for callers that only need the prior Running-state behavior.
        [[nodiscard]] auto sample(const domain::Field& field, int viewportWidth,
                                  int viewportHeight, PointerSample pointer,
                                  application::PaintMode paintMode,
                                  bool modalOwnsInput = false) -> InputCommands;
        // State-aware overload for callers that handle pause and resume controls.
        [[nodiscard]] auto sample(const domain::Field& field, int viewportWidth,
                                  int viewportHeight, PointerSample pointer,
                                  application::PaintMode paintMode,
                                  application::RunState runState,
                                  bool modalOwnsInput = false) -> InputCommands;
        // Camera-aware overload. Camera deltas are expressed in logical cell units.
        [[nodiscard]] auto sample(const domain::Field& field, int viewportWidth,
                                  int viewportHeight, PointerSample pointer,
                                  application::PaintMode paintMode,
                                  application::RunState runState, CameraState camera,
                                  bool modalOwnsInput = false) -> InputCommands;

        void reset() noexcept;

      private:
        [[nodiscard]] auto sampleForMode(const domain::Field& field, int viewportWidth,
                                         int viewportHeight, PointerSample pointer,
                                         application::PaintMode paintMode,
                                         application::RunState runState,
                                         CameraState camera,
                                         bool modalOwnsInput) -> InputCommands;
        [[nodiscard]] bool isToolbarOwner(LogicalPoint point, int viewportWidth,
                                          int viewportHeight) const noexcept;
        void clearGesture() noexcept;

        enum class GestureKind {
            None,
            Paint,
            Move,
        };

        GestureKind gestureKind_ = GestureKind::None;
        std::optional<domain::CellCoordinate> lastCell_;
        std::optional<LogicalPoint> lastPointer_;
    };

} // namespace lifeGame::presentation
