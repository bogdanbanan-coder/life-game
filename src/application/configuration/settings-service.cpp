#include <application/configuration/settings-service.hpp>

#include <new>
#include <utility>

namespace lifeGame::application {

    SettingsService::SettingsService()
        : activeSettings_{std::make_unique<SimulationSettings>(SimulationSettings::defaults())} {}

    SettingsService::SettingsService(SimulationSettings initialSettings)
        : activeSettings_{std::make_unique<SimulationSettings>(std::move(initialSettings))} {}

    const SimulationSettings& SettingsService::activeSettings() const noexcept {
        return *activeSettings_;
    }

    const SimulationSettings& SettingsService::settings() const noexcept {
        return activeSettings();
    }

    auto SettingsService::save(const SimulationSettings& settings)
        -> foundation::Result<void, foundation::ErrorCode> {
        std::unique_ptr<SimulationSettings> candidate;
        try {
            candidate = std::make_unique<SimulationSettings>(settings);
        } catch (const std::bad_alloc&) {
            return foundation::Result<void, foundation::ErrorCode>::failure(
                foundation::ErrorCode::InternalFailure);
        }

        if (nextSaveFailure_) {
            const auto error = *nextSaveFailure_;
            nextSaveFailure_.reset();
            return foundation::Result<void, foundation::ErrorCode>::failure(error);
        }

        activeSettings_.swap(candidate);
        return foundation::Result<void, foundation::ErrorCode>::success();
    }

    auto SettingsService::save(std::size_t width, std::size_t height,
                               SimulationSettings::Duration generationInterval)
        -> foundation::Result<void, foundation::ErrorCode> {
        const auto candidate =
            SimulationSettings::create(width, height, generationInterval);
        if (!candidate) {
            return foundation::Result<void, foundation::ErrorCode>::failure(candidate.error());
        }

        return save(candidate.value());
    }

    auto SettingsService::saveFromText(std::string_view width, std::string_view height,
                                       std::string_view generationIntervalSeconds)
        -> foundation::Result<void, foundation::ErrorCode> {
        const auto candidate =
            SimulationSettings::createFromText(width, height, generationIntervalSeconds);
        if (!candidate) {
            return foundation::Result<void, foundation::ErrorCode>::failure(candidate.error());
        }

        return save(candidate.value());
    }

    void SettingsService::failNextSave(foundation::ErrorCode error) {
        nextSaveFailure_ = error;
    }

} // namespace lifeGame::application
