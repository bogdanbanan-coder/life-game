#pragma once

#include <chrono>
#include <cstddef>
#include <string_view>

#include <domain/field/field-dimensions.hpp>
#include <foundation/error-code.hpp>
#include <foundation/result.hpp>

namespace lifeGame::application {

    class SimulationSettings {
      public:
        using Duration = std::chrono::duration<long double>;

        static constexpr std::size_t DEFAULT_WIDTH = 50;
        static constexpr std::size_t DEFAULT_HEIGHT = 50;
        static constexpr long double DEFAULT_INTERVAL_SECONDS = 0.25L;

        SimulationSettings();

        [[nodiscard]] static auto defaults() -> SimulationSettings;

        [[nodiscard]] static auto create(std::size_t width, std::size_t height,
                                          Duration generationInterval)
            -> foundation::Result<SimulationSettings, foundation::ErrorCode>;

        [[nodiscard]] static auto createFromText(std::string_view width,
                                                 std::string_view height,
                                                 std::string_view generationIntervalSeconds)
            -> foundation::Result<SimulationSettings, foundation::ErrorCode>;

        [[nodiscard]] static auto parseDimension(std::string_view text)
            -> foundation::Result<std::size_t, foundation::ErrorCode>;

        [[nodiscard]] static auto parseGenerationInterval(std::string_view text)
            -> foundation::Result<Duration, foundation::ErrorCode>;

        [[nodiscard]] const domain::FieldDimensions& dimensions() const noexcept;
        [[nodiscard]] std::size_t width() const noexcept;
        [[nodiscard]] std::size_t height() const noexcept;
        [[nodiscard]] Duration generationInterval() const noexcept;
        [[nodiscard]] Duration interval() const noexcept;
        [[nodiscard]] long double generationIntervalSeconds() const noexcept;

        [[nodiscard]] bool operator==(const SimulationSettings& other) const noexcept;

      private:
        SimulationSettings(domain::FieldDimensions dimensions, Duration generationInterval);

        domain::FieldDimensions dimensions_;
        Duration generationInterval_;
    };

} // namespace lifeGame::application
