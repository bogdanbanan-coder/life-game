#pragma once

#include <cstddef>

#include <foundation/error-code.hpp>
#include <foundation/result.hpp>

namespace lifeGame::domain {

    class FieldDimensions {
      public:
        static constexpr std::size_t MIN_SIDE = 1;
        static constexpr std::size_t MAX_SIDE = 4096;
        static constexpr std::size_t MAX_TOTAL_CELLS = 4'194'304;

        [[nodiscard]] static auto create(std::size_t width, std::size_t height)
            -> foundation::Result<FieldDimensions, foundation::ErrorCode>;

        [[nodiscard]] std::size_t cellCount() const noexcept;

        const std::size_t width;
        const std::size_t height;

      private:
        FieldDimensions(std::size_t widthValue, std::size_t heightValue,
                        std::size_t cellCountValue) noexcept;

        std::size_t cellCount_;
    };

} // namespace lifeGame::domain
