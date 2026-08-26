#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include <domain/field/cell-coordinate.hpp>
#include <domain/field/field-dimensions.hpp>
#include <foundation/error-code.hpp>
#include <foundation/result.hpp>

namespace lifeGame::domain {

    class ConwaySimulation;

    class Field {
      public:
        [[nodiscard]] static auto create(std::size_t width, std::size_t height)
            -> foundation::Result<Field, foundation::ErrorCode>;

        [[nodiscard]] static auto create(FieldDimensions dimensions)
            -> foundation::Result<Field, foundation::ErrorCode>;

        [[nodiscard]] const FieldDimensions& dimensions() const noexcept;
        [[nodiscard]] std::size_t width() const noexcept;
        [[nodiscard]] std::size_t height() const noexcept;
        [[nodiscard]] bool contains(CellCoordinate coordinate) const noexcept;
        [[nodiscard]] bool isLive(CellCoordinate coordinate) const noexcept;
        [[nodiscard]] bool readCell(CellCoordinate coordinate, std::uint8_t& value) const noexcept;
        [[nodiscard]] bool setLive(CellCoordinate coordinate, bool live) noexcept;
        [[nodiscard]] const std::vector<std::uint8_t>& cells() const noexcept;

      private:
        friend class ConwaySimulation;

        explicit Field(FieldDimensions dimensions);

        [[nodiscard]] std::size_t indexOf(CellCoordinate coordinate) const noexcept;

        FieldDimensions dimensions_;
        std::vector<std::uint8_t> cells_;
        std::vector<std::uint8_t> nextCells_;
    };

} // namespace lifeGame::domain
