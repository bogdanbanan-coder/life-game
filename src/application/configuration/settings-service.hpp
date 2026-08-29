#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <string_view>

#include <application/configuration/simulation-settings.hpp>
#include <foundation/error-code.hpp>
#include <foundation/result.hpp>

namespace lifeGame::application {

    class SettingsService {
      public:
        SettingsService();
        explicit SettingsService(SimulationSettings initialSettings);

        [[nodiscard]] const SimulationSettings& activeSettings() const noexcept;
        [[nodiscard]] const SimulationSettings& settings() const noexcept;

        [[nodiscard]] auto save(const SimulationSettings& settings)
            -> foundation::Result<void, foundation::ErrorCode>;
        [[nodiscard]] auto save(std::size_t width, std::size_t height,
                                SimulationSettings::Duration generationInterval)
            -> foundation::Result<void, foundation::ErrorCode>;
        [[nodiscard]] auto saveFromText(std::string_view width, std::string_view height,
                                        std::string_view generationIntervalSeconds)
            -> foundation::Result<void, foundation::ErrorCode>;

        void failNextSave(foundation::ErrorCode error);

      private:
        std::unique_ptr<SimulationSettings> activeSettings_;
        std::optional<foundation::ErrorCode> nextSaveFailure_;
    };

} // namespace lifeGame::application
