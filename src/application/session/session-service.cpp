#include <application/session/session-service.hpp>

#include <algorithm>
#include <new>
#include <utility>

namespace lifeGame::application {

    SessionService::SessionService(SettingsService& settingsService)
        : settingsService_{settingsService} {
        sessions_.reserve(MAX_SESSIONS);
    }

    auto SessionService::create(std::string_view name)
        -> foundation::Result<domain::SessionId, foundation::ErrorCode> {
        const auto validatedName = domain::SessionName::create(name);
        if (!validatedName) {
            return foundation::Result<domain::SessionId, foundation::ErrorCode>::failure(
                validatedName.error());
        }

        return create(std::move(validatedName).value());
    }

    auto SessionService::create(domain::SessionName name)
        -> foundation::Result<domain::SessionId, foundation::ErrorCode> {
        if (findName(name) != nullptr) {
            return foundation::Result<domain::SessionId, foundation::ErrorCode>::failure(
                foundation::ErrorCode::AlreadyExists);
        }

        if (sessions_.size() >= MAX_SESSIONS) {
            return foundation::Result<domain::SessionId, foundation::ErrorCode>::failure(
                foundation::ErrorCode::CapacityExceeded);
        }

        const auto session = domain::Session::create(
            std::move(name), settingsService_.activeSettings().dimensions(), nextId_);
        if (!session) {
            return foundation::Result<domain::SessionId, foundation::ErrorCode>::failure(
                session.error());
        }

        try {
            sessions_.push_back(std::move(session).value());
        } catch (const std::bad_alloc&) {
            return foundation::Result<domain::SessionId, foundation::ErrorCode>::failure(
                foundation::ErrorCode::InternalFailure);
        }

        const auto createdId = nextId_;
        ++nextId_;
        return foundation::Result<domain::SessionId, foundation::ErrorCode>::success(createdId);
    }

    auto SessionService::open(domain::SessionId id)
        -> foundation::Result<domain::Session*, foundation::ErrorCode> {
        auto* session = find(id);
        if (session == nullptr) {
            return foundation::Result<domain::Session*, foundation::ErrorCode>::failure(
                foundation::ErrorCode::NotFound);
        }
        return foundation::Result<domain::Session*, foundation::ErrorCode>::success(session);
    }

    auto SessionService::open(std::string_view name)
        -> foundation::Result<domain::Session*, foundation::ErrorCode> {
        const auto validatedName = domain::SessionName::create(name);
        if (!validatedName) {
            return foundation::Result<domain::Session*, foundation::ErrorCode>::failure(
                validatedName.error());
        }

        const auto* matchingSession = findName(validatedName.value());
        if (matchingSession == nullptr) {
            return foundation::Result<domain::Session*, foundation::ErrorCode>::failure(
                foundation::ErrorCode::NotFound);
        }

        return open(matchingSession->id());
    }

    domain::Session* SessionService::find(domain::SessionId id) noexcept {
        for (auto& session : sessions_) {
            if (session.id() == id) {
                return &session;
            }
        }
        return nullptr;
    }

    const domain::Session* SessionService::find(domain::SessionId id) const noexcept {
        for (const auto& session : sessions_) {
            if (session.id() == id) {
                return &session;
            }
        }
        return nullptr;
    }

    auto SessionService::findByName(std::string_view name) const
        -> foundation::Result<domain::SessionId, foundation::ErrorCode> {
        const auto validatedName = domain::SessionName::create(name);
        if (!validatedName) {
            return foundation::Result<domain::SessionId, foundation::ErrorCode>::failure(
                validatedName.error());
        }

        const auto* session = findName(validatedName.value());
        if (session == nullptr) {
            return foundation::Result<domain::SessionId, foundation::ErrorCode>::failure(
                foundation::ErrorCode::NotFound);
        }
        return foundation::Result<domain::SessionId, foundation::ErrorCode>::success(session->id());
    }

    bool SessionService::containsName(std::string_view name) const noexcept {
        const auto validatedName = domain::SessionName::create(name);
        return validatedName && findName(validatedName.value()) != nullptr;
    }

    bool SessionService::isNameAvailable(std::string_view name) const noexcept {
        const auto validatedName = domain::SessionName::create(name);
        return validatedName && findName(validatedName.value()) == nullptr;
    }

    const std::vector<domain::Session>& SessionService::sessions() const noexcept {
        return sessions_;
    }

    auto SessionService::summaries() const -> std::vector<SessionSummary> {
        std::vector<SessionSummary> result;
        result.reserve(sessions_.size());
        for (const auto& session : sessions_) {
            result.push_back(SessionSummary{session.id(), session.name().displayName(),
                                            session.dimensions()});
        }

        std::vector<std::size_t> order;
        order.reserve(result.size());
        for (std::size_t index = 0; index < result.size(); ++index) {
            order.push_back(index);
        }

        std::stable_sort(order.begin(), order.end(), [&result](std::size_t leftIndex,
                                                                std::size_t rightIndex) {
            const auto leftName = domain::SessionName::create(result[leftIndex].displayName);
            const auto rightName = domain::SessionName::create(result[rightIndex].displayName);
            if (!leftName || !rightName) {
                return result[leftIndex].id < result[rightIndex].id;
            }
            return leftName.value().uniquenessKey() < rightName.value().uniquenessKey();
        });

        std::vector<SessionSummary> sorted;
        sorted.reserve(result.size());
        for (const auto index : order) {
            sorted.push_back(std::move(result[index]));
        }
        return sorted;
    }

    auto SessionService::list() const -> std::vector<SessionSummary> { return summaries(); }

    const SettingsService& SessionService::settingsService() const noexcept {
        return settingsService_;
    }

    auto SessionService::findName(const domain::SessionName& name) const noexcept
        -> const domain::Session* {
        for (const auto& session : sessions_) {
            if (session.name().equalsCaseInsensitive(name)) {
                return &session;
            }
        }
        return nullptr;
    }

} // namespace lifeGame::application
