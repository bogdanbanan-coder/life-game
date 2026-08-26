#include <domain/field/field-dimensions.hpp>

#include <limits>

namespace lifeGame::domain {

    FieldDimensions::FieldDimensions(std::size_t widthValue, std::size_t heightValue,
                                     std::size_t cellCountValue) noexcept
        : width{widthValue}, height{heightValue}, cellCount_{cellCountValue} {}

    auto FieldDimensions::create(std::size_t width, std::size_t height)
        -> foundation::Result<FieldDimensions, foundation::ErrorCode> {
        if (width < MIN_SIDE || height < MIN_SIDE || width > MAX_SIDE || height > MAX_SIDE) {
            return foundation::Result<FieldDimensions, foundation::ErrorCode>::failure(
                foundation::ErrorCode::InvalidArgument);
        }

        if (height != 0 && width > MAX_TOTAL_CELLS / height) {
            return foundation::Result<FieldDimensions, foundation::ErrorCode>::failure(
                foundation::ErrorCode::InvalidArgument);
        }

        if (width > std::numeric_limits<std::size_t>::max() / height) {
            return foundation::Result<FieldDimensions, foundation::ErrorCode>::failure(
                foundation::ErrorCode::InvalidArgument);
        }

        const auto cellCount = width * height;
        return foundation::Result<FieldDimensions, foundation::ErrorCode>::success(
            FieldDimensions{width, height, cellCount});
    }

    std::size_t FieldDimensions::cellCount() const noexcept { return cellCount_; }

} // namespace lifeGame::domain
