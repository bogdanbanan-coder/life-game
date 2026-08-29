#include <presentation/screens/field-screen.hpp>

namespace lifeGame::presentation {

    void FieldScreen::render(const domain::Field& field, int viewportWidth, int viewportHeight,
                             application::PaintMode paintMode,
                             application::RunState runState) {
        cameraController_.constrain(field, viewportWidth, viewportHeight);
        fieldRenderer_.render(field, viewportWidth, viewportHeight,
                              cameraController_.state());
        toolbar_.render(viewportWidth, viewportHeight, paintMode, runState);
    }

    CameraState FieldScreen::cameraState() const noexcept { return cameraController_.state(); }

    void FieldScreen::applyCameraPan(const domain::Field& field, int viewportWidth,
                                     int viewportHeight, float deltaX, float deltaY) noexcept {
        cameraController_.pan(field, viewportWidth, viewportHeight, deltaX, deltaY);
    }

    void FieldScreen::applyZoom(const domain::Field& field, int viewportWidth, int viewportHeight,
                                application::ZoomDirection direction, float anchorX,
                                float anchorY) noexcept {
        if (direction == application::ZoomDirection::In) {
            cameraController_.zoom(field, viewportWidth, viewportHeight, true, anchorX, anchorY);
        } else if (direction == application::ZoomDirection::Out) {
            cameraController_.zoom(field, viewportWidth, viewportHeight, false, anchorX, anchorY);
        }
    }

    void FieldScreen::resetNavigation() noexcept { cameraController_.reset(); }

} // namespace lifeGame::presentation
