#include <domain/session/session.hpp>

#include <utility>

namespace lifeGame::domain {

    Session::Session(SessionId id, SessionName name, Field field)
        : id_{id}, name_{std::move(name)}, field_{std::move(field)} {}

    auto Session::create(SessionName name, FieldDimensions dimensions, SessionId id)
        -> foundation::Result<Session, foundation::ErrorCode> {
        const auto field = Field::create(dimensions);
        if (!field) {
            return foundation::Result<Session, foundation::ErrorCode>::failure(field.error());
        }

        return foundation::Result<Session, foundation::ErrorCode>::success(
            Session{id, std::move(name), std::move(field).value()});
    }

    auto Session::create(std::string_view name, FieldDimensions dimensions, SessionId id)
        -> foundation::Result<Session, foundation::ErrorCode> {
        const auto validatedName = SessionName::create(name);
        if (!validatedName) {
            return foundation::Result<Session, foundation::ErrorCode>::failure(
                validatedName.error());
        }

        return create(std::move(validatedName).value(), std::move(dimensions), id);
    }

    SessionId Session::id() const noexcept { return id_; }

    const SessionName& Session::name() const noexcept { return name_; }

    const FieldDimensions& Session::dimensions() const noexcept { return field_.dimensions(); }

    const Field& Session::field() const noexcept { return field_; }

    Field& Session::field() noexcept { return field_; }

} // namespace lifeGame::domain
