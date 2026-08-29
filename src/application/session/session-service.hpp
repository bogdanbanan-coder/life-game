#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include <application/configuration/settings-service.hpp>
#include <domain/session/session.hpp>
#include <foundation/error-code.hpp>
#include <foundation/result.hpp>

namespace lifeGame::application {

    struct SessionSummary {
        domain::SessionId id;
        std::string displayName;
        domain::FieldDimensions dimensions;
    };

    class SessionService {
      public:
        static constexpr std::size_t MAX_SESSIONS = 512;

        explicit SessionService(SettingsService& settingsService);

        [[nodiscard]] auto create(std::string_view name)
            -> foundation::Result<domain::SessionId, foundation::ErrorCode>;
        [[nodiscard]] auto create(domain::SessionName name)
            -> foundation::Result<domain::SessionId, foundation::ErrorCode>;

        [[nodiscard]] auto open(domain::SessionId id)
            -> foundation::Result<domain::Session*, foundation::ErrorCode>;
        [[nodiscard]] auto open(std::string_view name)
            -> foundation::Result<domain::Session*, foundation::ErrorCode>;

        [[nodiscard]] domain::Session* find(domain::SessionId id) noexcept;
        [[nodiscard]] const domain::Session* find(domain::SessionId id) const noexcept;

        [[nodiscard]] auto findByName(std::string_view name) const
            -> foundation::Result<domain::SessionId, foundation::ErrorCode>;
        [[nodiscard]] bool containsName(std::string_view name) const noexcept;
        [[nodiscard]] bool isNameAvailable(std::string_view name) const noexcept;

        [[nodiscard]] const std::vector<domain::Session>& sessions() const noexcept;
        [[nodiscard]] auto summaries() const -> std::vector<SessionSummary>;
        [[nodiscard]] auto list() const -> std::vector<SessionSummary>;
        [[nodiscard]] const SettingsService& settingsService() const noexcept;

      private:
        [[nodiscard]] auto findName(const domain::SessionName& name) const noexcept
            -> const domain::Session*;

        SettingsService& settingsService_;
        std::vector<domain::Session> sessions_;
        domain::SessionId nextId_ = 1;
    };

} // namespace lifeGame::application
