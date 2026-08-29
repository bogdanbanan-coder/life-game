#pragma once

#include <domain/field/field.hpp>

namespace lifeGame::presentation {

    struct CameraState {
        float x = 0.0F;
        float y = 0.0F;

        [[nodiscard]] constexpr auto operator==(const CameraState&) const noexcept -> bool =
            default;
    };

    class CameraController {
      public:
        CameraController() noexcept = default;

        [[nodiscard]] static constexpr auto defaultState() noexcept -> CameraState {
            return CameraState{0.0F, 0.0F};
        }

        [[nodiscard]] CameraState state() const noexcept;

        void reset() noexcept;
        void pan(const domain::Field& field, int viewportWidth, int viewportHeight,
                 float deltaX, float deltaY) noexcept;
        void constrain(const domain::Field& field, int viewportWidth, int viewportHeight) noexcept;

      private:
        [[nodiscard]] static auto clampState(const domain::Field& field, int viewportWidth,
                                              int viewportHeight, CameraState state) noexcept
            -> CameraState;

        CameraState state_ = defaultState();
    };

} // namespace lifeGame::presentation
