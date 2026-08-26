#include <domain/field/field.hpp>

#include <new>
#include <utility>

namespace lifeGame::domain {

    Field::Field(FieldDimensions dimensions)
        : dimensions_{dimensions}, cells_(dimensions.cellCount(), std::uint8_t{0}) {}

    auto Field::create(std::size_t width, std::size_t height)
        -> foundation::Result<Field, foundation::ErrorCode> {
        const auto dimensions = FieldDimensions::create(width, height);
        if (!dimensions) {
            return foundation::Result<Field, foundation::ErrorCode>::failure(dimensions.error());
        }

        return create(dimensions.value());
    }

    auto Field::create(FieldDimensions dimensions)
        -> foundation::Result<Field, foundation::ErrorCode> {
        try {
            return foundation::Result<Field, foundation::ErrorCode>::success(
                Field{std::move(dimensions)});
        } catch (const std::bad_alloc&) {
            return foundation::Result<Field, foundation::ErrorCode>::failure(
                foundation::ErrorCode::InternalFailure);
        }
    }

    const FieldDimensions& Field::dimensions() const noexcept { return dimensions_; }

    std::size_t Field::width() const noexcept { return dimensions_.width; }

    std::size_t Field::height() const noexcept { return dimensions_.height; }

    bool Field::contains(CellCoordinate coordinate) const noexcept {
        return coordinate.x < width() && coordinate.y < height();
    }

    bool Field::isLive(CellCoordinate coordinate) const noexcept {
        return contains(coordinate) && cells_[indexOf(coordinate)] != 0;
    }

    bool Field::readCell(CellCoordinate coordinate, std::uint8_t& value) const noexcept {
        if (!contains(coordinate)) {
            return false;
        }

        value = cells_[indexOf(coordinate)];
        return true;
    }

    bool Field::setLive(CellCoordinate coordinate, bool live) noexcept {
        if (!contains(coordinate)) {
            return false;
        }

        cells_[indexOf(coordinate)] = live ? std::uint8_t{1} : std::uint8_t{0};
        return true;
    }

    const std::vector<std::uint8_t>& Field::cells() const noexcept { return cells_; }

    std::size_t Field::indexOf(CellCoordinate coordinate) const noexcept {
        return coordinate.y * width() + coordinate.x;
    }

} // namespace lifeGame::domain
