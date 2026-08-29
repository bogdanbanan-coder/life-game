#pragma once

#include <cstddef>
#include <string>
#include <string_view>

#include <foundation/error-code.hpp>
#include <foundation/result.hpp>

namespace lifeGame::domain {

    class SessionName {
      public:
        [[nodiscard]] static auto create(std::string_view input)
            -> foundation::Result<SessionName, foundation::ErrorCode>;

        [[nodiscard]] const std::string& value() const noexcept;
        [[nodiscard]] const std::string& displayName() const noexcept;
        [[nodiscard]] const std::string& normalized() const noexcept;
        [[nodiscard]] const std::string& uniquenessKey() const noexcept;
        [[nodiscard]] std::size_t codePointCount() const noexcept;

        [[nodiscard]] bool equalsCaseInsensitive(const SessionName& other) const noexcept;

        friend bool operator==(const SessionName& left, const SessionName& right) noexcept {
            return left.uniquenessKey_ == right.uniquenessKey_;
        }

      private:
        SessionName(std::string displayName, std::string uniquenessKey,
                    std::size_t codePointCount);

        std::string displayName_;
        std::string uniquenessKey_;
        std::size_t codePointCount_;
    };

} // namespace lifeGame::domain
