#include <application/configuration/simulation-settings.hpp>

#include <cmath>
#include <cstdint>
#include <limits>
#include <utility>

namespace lifeGame::application {

    namespace {

        [[nodiscard]] auto invalidSettings()
            -> foundation::Result<SimulationSettings, foundation::ErrorCode> {
            return foundation::Result<SimulationSettings, foundation::ErrorCode>::failure(
                foundation::ErrorCode::InvalidArgument);
        }

        [[nodiscard]] auto defaultDimensions() -> domain::FieldDimensions {
            const auto dimensions = domain::FieldDimensions::create(
                SimulationSettings::DEFAULT_WIDTH, SimulationSettings::DEFAULT_HEIGHT);
            return dimensions.value();
        }

    } // namespace

    SimulationSettings::SimulationSettings()
        : dimensions_{defaultDimensions()},
          generationInterval_{Duration{DEFAULT_INTERVAL_SECONDS}} {}

    SimulationSettings::SimulationSettings(domain::FieldDimensions dimensions,
                                           Duration generationInterval)
        : dimensions_{std::move(dimensions)}, generationInterval_{generationInterval} {}

    auto SimulationSettings::defaults() -> SimulationSettings { return SimulationSettings{}; }

    auto SimulationSettings::create(std::size_t width, std::size_t height,
                                    Duration generationInterval)
        -> foundation::Result<SimulationSettings, foundation::ErrorCode> {
        if (!std::isfinite(generationInterval.count()) ||
            generationInterval <= Duration::zero()) {
            return invalidSettings();
        }

        const auto dimensions = domain::FieldDimensions::create(width, height);
        if (!dimensions) {
            return foundation::Result<SimulationSettings, foundation::ErrorCode>::failure(
                dimensions.error());
        }

        return foundation::Result<SimulationSettings, foundation::ErrorCode>::success(
            SimulationSettings{std::move(dimensions).value(), generationInterval});
    }

    auto SimulationSettings::createFromText(std::string_view width, std::string_view height,
                                            std::string_view generationIntervalSeconds)
        -> foundation::Result<SimulationSettings, foundation::ErrorCode> {
        const auto parsedWidth = parseDimension(width);
        if (!parsedWidth) {
            return invalidSettings();
        }

        const auto parsedHeight = parseDimension(height);
        if (!parsedHeight) {
            return invalidSettings();
        }

        const auto parsedInterval = parseGenerationInterval(generationIntervalSeconds);
        if (!parsedInterval) {
            return invalidSettings();
        }

        return create(parsedWidth.value(), parsedHeight.value(), parsedInterval.value());
    }

    auto SimulationSettings::parseDimension(std::string_view text)
        -> foundation::Result<std::size_t, foundation::ErrorCode> {
        if (text.empty()) {
            return foundation::Result<std::size_t, foundation::ErrorCode>::failure(
                foundation::ErrorCode::InvalidArgument);
        }

        std::size_t value = 0;
        for (const auto character : text) {
            if (character < '0' || character > '9') {
                return foundation::Result<std::size_t, foundation::ErrorCode>::failure(
                    foundation::ErrorCode::InvalidArgument);
            }

            const auto digit = static_cast<std::size_t>(character - '0');
            if (value > (std::numeric_limits<std::size_t>::max() - digit) / 10U) {
                return foundation::Result<std::size_t, foundation::ErrorCode>::failure(
                    foundation::ErrorCode::InvalidArgument);
            }
            value = value * 10U + digit;
        }

        return foundation::Result<std::size_t, foundation::ErrorCode>::success(value);
    }

    auto SimulationSettings::parseGenerationInterval(std::string_view text)
        -> foundation::Result<Duration, foundation::ErrorCode> {
        if (text.empty()) {
            return foundation::Result<Duration, foundation::ErrorCode>::failure(
                foundation::ErrorCode::InvalidArgument);
        }

        bool foundDecimalSeparator = false;
        bool foundDigit = false;
        long double value = 0.0L;
        long double fractionalScale = 1.0L;
        for (const auto character : text) {
            if (character == '.') {
                if (foundDecimalSeparator) {
                    return foundation::Result<Duration, foundation::ErrorCode>::failure(
                        foundation::ErrorCode::InvalidArgument);
                }
                foundDecimalSeparator = true;
                continue;
            }

            if (character < '0' || character > '9') {
                return foundation::Result<Duration, foundation::ErrorCode>::failure(
                    foundation::ErrorCode::InvalidArgument);
            }

            foundDigit = true;
            const auto digit = static_cast<long double>(character - '0');
            if (!foundDecimalSeparator) {
                value = value * 10.0L + digit;
            } else {
                fractionalScale *= 10.0L;
                if (!std::isfinite(fractionalScale)) {
                    return foundation::Result<Duration, foundation::ErrorCode>::failure(
                        foundation::ErrorCode::InvalidArgument);
                }
                value += digit / fractionalScale;
            }

            if (!std::isfinite(value)) {
                return foundation::Result<Duration, foundation::ErrorCode>::failure(
                    foundation::ErrorCode::InvalidArgument);
            }
        }

        if (!foundDigit || !std::isfinite(value) || value <= 0.0L) {
            return foundation::Result<Duration, foundation::ErrorCode>::failure(
                foundation::ErrorCode::InvalidArgument);
        }

        return foundation::Result<Duration, foundation::ErrorCode>::success(Duration{value});
    }

    const domain::FieldDimensions& SimulationSettings::dimensions() const noexcept {
        return dimensions_;
    }

    std::size_t SimulationSettings::width() const noexcept { return dimensions_.width; }

    std::size_t SimulationSettings::height() const noexcept { return dimensions_.height; }

    SimulationSettings::Duration SimulationSettings::generationInterval() const noexcept {
        return generationInterval_;
    }

    SimulationSettings::Duration SimulationSettings::interval() const noexcept {
        return generationInterval_;
    }

    long double SimulationSettings::generationIntervalSeconds() const noexcept {
        return generationInterval_.count();
    }

    bool SimulationSettings::operator==(const SimulationSettings& other) const noexcept {
        return dimensions_.width == other.dimensions_.width &&
               dimensions_.height == other.dimensions_.height &&
               generationInterval_ == other.generationInterval_;
    }

} // namespace lifeGame::application
