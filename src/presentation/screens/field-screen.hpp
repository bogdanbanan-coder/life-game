#pragma once

#include <application/commands/field-command.hpp>
#include <application/commands/run-command.hpp>
#include <domain/field/field.hpp>
#include <presentation/camera/camera-controller.hpp>
#include <presentation/rendering/field-renderer.hpp>
#include <presentation/ui/toolbar.hpp>

namespace lifeGame::presentation {

    class FieldScreen {
      public:
        void render(const domain::Field& field, int viewportWidth, int viewportHeight,
                    application::PaintMode paintMode,
                    application::RunState runState);

        [[nodiscard]] CameraState cameraState() const noexcept;

        void applyCameraPan(const domain::Field& field, int viewportWidth, int viewportHeight,
                            float deltaX, float deltaY) noexcept;
        void applyZoom(const domain::Field& field, int viewportWidth, int viewportHeight,
                       application::ZoomDirection direction, float anchorX,
                       float anchorY) noexcept;
        void resetNavigation() noexcept;

      private:
        FieldRenderer fieldRenderer_;
        Toolbar toolbar_;
        CameraController cameraController_;
    };

} // namespace lifeGame::presentation
