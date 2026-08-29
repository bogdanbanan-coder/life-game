#include <presentation/camera/camera-controller.hpp>

#include <algorithm>
#include <cmath>
#include <optional>

#include <presentation/camera/coordinate-converter.hpp>
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
                            CameraState{candidateX, candidateY, state_.zoomLevel});
    }

    void CameraController::zoom(const domain::Field& field, int viewportWidth, int viewportHeight,
                                bool zoomIn, float anchorX, float anchorY) noexcept {
        const auto currentPlan =
            FieldRenderer::calculateRenderPlan(field, viewportWidth, viewportHeight, state_);
        state_ = currentPlan.camera;

        const auto levels = zoomLevels();
        auto levelIndex = std::size_t{0};
        for (; levelIndex < levels.size(); ++levelIndex) {
            if (levels[levelIndex] == state_.zoomLevel) {
                break;
            }
        }
        if (levelIndex == levels.size() ||
            (zoomIn && levelIndex + 1 >= levels.size()) ||
            (!zoomIn && levelIndex == 0)) {
            return;
        }

        const auto safeWidth = std::max(viewportWidth, 1);
        const auto safeHeight = std::max(viewportHeight, 1);
        const auto centerX = static_cast<float>(safeWidth) / 2.0F;
        const auto centerY = static_cast<float>(safeHeight) / 2.0F;
        const auto pointerCell = CoordinateConverter::toCell(
            field, LogicalPoint{anchorX, anchorY}, safeWidth, safeHeight, currentPlan.camera);
        const auto usePointerAnchor = pointerCell.has_value();
        const auto screenX = usePointerAnchor ? anchorX : centerX;
        const auto screenY = usePointerAnchor ? anchorY : centerY;
        const auto oldLogicalX =
            (screenX - currentPlan.fieldRectangle.x) / currentPlan.cellSize;
        const auto oldLogicalY =
            (screenY - currentPlan.fieldRectangle.y) / currentPlan.cellSize;
        const auto oldCellX = usePointerAnchor
                                  ? static_cast<long long>(pointerCell->x)
                                  : static_cast<long long>(std::floor(oldLogicalX));
        const auto oldCellY = usePointerAnchor
                                  ? static_cast<long long>(pointerCell->y)
                                  : static_cast<long long>(std::floor(oldLogicalY));

        state_.zoomLevel = zoomIn ? levels[levelIndex + 1] : levels[levelIndex - 1];
        state_ = clampState(field, safeWidth, safeHeight, state_);

        // Reconcile the camera after changing the scale so the same logical point remains at the
        // selected screen anchor. Recheck the cell after each snapped adjustment.
        for (int attempt = 0; attempt < 8; ++attempt) {
            const auto plan =
                FieldRenderer::calculateRenderPlan(field, safeWidth, safeHeight, state_);
            const auto logicalX = (screenX - plan.fieldRectangle.x) / plan.cellSize;
            const auto logicalY = (screenY - plan.fieldRectangle.y) / plan.cellSize;
            const auto deltaX = oldLogicalX - logicalX;
            const auto deltaY = oldLogicalY - logicalY;
            if (!std::isfinite(deltaX) || !std::isfinite(deltaY)) {
                break;
            }
            const auto cellAtAnchor = usePointerAnchor
                                          ? CoordinateConverter::toCell(
                                                field, LogicalPoint{screenX, screenY}, safeWidth,
                                                safeHeight, plan.camera)
                                          : std::nullopt;
            if ((usePointerAnchor && cellAtAnchor &&
                 static_cast<long long>(cellAtAnchor->x) == oldCellX &&
                 static_cast<long long>(cellAtAnchor->y) == oldCellY) ||
                (!usePointerAnchor && std::abs(deltaX) < 0.001F &&
                 std::abs(deltaY) < 0.001F)) {
                break;
            }

            const auto candidateX = state_.x + deltaX;
            const auto candidateY = state_.y + deltaY;
            if (!std::isfinite(candidateX) || !std::isfinite(candidateY)) {
                break;
            }

            const auto nextState = clampState(
                field, safeWidth, safeHeight, CameraState{candidateX, candidateY, state_.zoomLevel});
            if (nextState == state_) {
                break;
            }
            state_ = nextState;
        }
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
