#pragma once

#include <array>
#include <cstddef>

#include <domain/field/field.hpp>

namespace lifeGame::presentation {

    enum class ZoomLevel {
        Percent50 = 50,
        Percent75 = 75,
        Percent100 = 100,
        Percent150 = 150,
        Percent200 = 200,
        Percent300 = 300,
        Percent400 = 400,
    };

    struct CameraState {
        float x = 0.0F;
        float y = 0.0F;
        ZoomLevel zoomLevel = ZoomLevel::Percent100;

        [[nodiscard]] constexpr auto operator==(const CameraState&) const noexcept -> bool =
            default;
    };

    class CameraController {
      public:
        CameraController() noexcept = default;

        [[nodiscard]] static constexpr auto defaultState() noexcept -> CameraState {
            return CameraState{0.0F, 0.0F, ZoomLevel::Percent100};
        }

        [[nodiscard]] static constexpr auto zoomLevels() noexcept
            -> std::array<ZoomLevel, 7> {
            return {ZoomLevel::Percent50,  ZoomLevel::Percent75,  ZoomLevel::Percent100,
                    ZoomLevel::Percent150, ZoomLevel::Percent200, ZoomLevel::Percent300,
                    ZoomLevel::Percent400};
        }

        [[nodiscard]] static constexpr auto zoomScale(ZoomLevel zoomLevel) noexcept -> float {
            switch (zoomLevel) {
            case ZoomLevel::Percent50:
                return 0.5F;
            case ZoomLevel::Percent75:
                return 0.75F;
            case ZoomLevel::Percent100:
                return 1.0F;
            case ZoomLevel::Percent150:
                return 1.5F;
            case ZoomLevel::Percent200:
                return 2.0F;
            case ZoomLevel::Percent300:
                return 3.0F;
            case ZoomLevel::Percent400:
                return 4.0F;
            }
            return 1.0F;
        }

        [[nodiscard]] static constexpr auto normalizeZoomLevel(ZoomLevel zoomLevel) noexcept
            -> ZoomLevel {
            switch (zoomLevel) {
            case ZoomLevel::Percent50:
            case ZoomLevel::Percent75:
            case ZoomLevel::Percent100:
            case ZoomLevel::Percent150:
            case ZoomLevel::Percent200:
            case ZoomLevel::Percent300:
            case ZoomLevel::Percent400:
                return zoomLevel;
            }
            return ZoomLevel::Percent100;
        }

        [[nodiscard]] CameraState state() const noexcept;

        void reset() noexcept;
        void pan(const domain::Field& field, int viewportWidth, int viewportHeight,
                 float deltaX, float deltaY) noexcept;
        void zoom(const domain::Field& field, int viewportWidth, int viewportHeight,
                  bool zoomIn, float anchorX, float anchorY) noexcept;
        void constrain(const domain::Field& field, int viewportWidth, int viewportHeight) noexcept;

      private:
        [[nodiscard]] static auto clampState(const domain::Field& field, int viewportWidth,
                                              int viewportHeight, CameraState state) noexcept
            -> CameraState;

        CameraState state_ = defaultState();
    };

} // namespace lifeGame::presentation
