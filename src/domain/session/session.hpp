#pragma once

#include <string_view>

#include <domain/field/field-dimensions.hpp>
#include <domain/field/field.hpp>
#include <domain/session/session-id.hpp>
#include <domain/session/session-name.hpp>
#include <foundation/error-code.hpp>
#include <foundation/result.hpp>

namespace lifeGame::domain {

    class Session {
      public:
        [[nodiscard]] static auto create(SessionName name, FieldDimensions dimensions,
                                          SessionId id = 0)
            -> foundation::Result<Session, foundation::ErrorCode>;

        [[nodiscard]] static auto create(std::string_view name, FieldDimensions dimensions,
                                          SessionId id = 0)
            -> foundation::Result<Session, foundation::ErrorCode>;

        [[nodiscard]] SessionId id() const noexcept;
        [[nodiscard]] const SessionName& name() const noexcept;
        [[nodiscard]] const FieldDimensions& dimensions() const noexcept;
        [[nodiscard]] const Field& field() const noexcept;
        [[nodiscard]] Field& field() noexcept;

      private:
        Session(SessionId id, SessionName name, Field field);

        SessionId id_;
        SessionName name_;
        Field field_;
    };

} // namespace lifeGame::domain
