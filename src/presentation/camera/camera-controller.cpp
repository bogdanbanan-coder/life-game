#include <presentation/camera/camera-controller.hpp>

#include <cmath>

#include <presentation/rendering/field-renderer.hpp>

namespace lifeGame::presentation {

    CameraState CameraController::state() const noexcept { return state_; }

    void CameraController::reset() noexcept { state_ = defaultState(); }

    void CameraController::pan(const domain::Field& field, int viewportWidth, int viewportHeight,
                               float deltaX, float deltaY) noexcept {
        if (!std::isfinite(deltaX) || !std::isfinite(deltaY)) {
            return;
        }

        const auto candidateX = state_.x + deltaX;
        const auto candidateY = state_.y + deltaY;
        if (!std::isfinite(candidateX) || !std::isfinite(candidateY)) {
            return;
        }

        state_ = clampState(field, viewportWidth, viewportHeight,
                            CameraState{candidateX, candidateY});
    }

    void CameraController::constrain(const domain::Field& field, int viewportWidth,
                                     int viewportHeight) noexcept {
        state_ = clampState(field, viewportWidth, viewportHeight, state_);
    }

    CameraState CameraController::clampState(const domain::Field& field, int viewportWidth,
                                             int viewportHeight, CameraState state) noexcept {
        return FieldRenderer::calculateRenderPlan(field, viewportWidth, viewportHeight, state)
            .camera;
    }

} // namespace lifeGame::presentation
