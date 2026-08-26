#include <presentation/input/drag-rasterizer.hpp>

#include <algorithm>
#include <cstdint>
#include <limits>

#include <domain/field/field-dimensions.hpp>

namespace lifeGame::presentation {

    namespace {

        constexpr auto MAX_RASTERIZABLE_CELLS = domain::FieldDimensions::MAX_TOTAL_CELLS;

    } // namespace

    auto DragRasterizer::rasterize(domain::CellCoordinate from, domain::CellCoordinate to)
        -> std::vector<domain::CellCoordinate> {
        constexpr auto MAX_INT64 = std::numeric_limits<std::int64_t>::max();
        if (static_cast<std::uintmax_t>(from.x) > static_cast<std::uintmax_t>(MAX_INT64) ||
            static_cast<std::uintmax_t>(from.y) > static_cast<std::uintmax_t>(MAX_INT64) ||
            static_cast<std::uintmax_t>(to.x) > static_cast<std::uintmax_t>(MAX_INT64) ||
            static_cast<std::uintmax_t>(to.y) > static_cast<std::uintmax_t>(MAX_INT64)) {
            return {};
        }

        const auto reverseOutput = from.x > to.x || (from.x == to.x && from.y > to.y);
        if (reverseOutput) {
            std::swap(from, to);
        }

        const auto x0 = static_cast<std::int64_t>(from.x);
        const auto y0 = static_cast<std::int64_t>(from.y);
        const auto x1 = static_cast<std::int64_t>(to.x);
        const auto y1 = static_cast<std::int64_t>(to.y);
        const auto deltaX = x0 < x1 ? x1 - x0 : x0 - x1;
        const auto deltaY = y0 < y1 ? y1 - y0 : y0 - y1;
        const auto maxDelta = std::max(deltaX, deltaY);
        if (maxDelta == MAX_INT64 ||
            static_cast<std::uintmax_t>(maxDelta) >=
                static_cast<std::uintmax_t>(MAX_RASTERIZABLE_CELLS)) {
            return {};
        }

        const auto stepX = x0 < x1 ? std::int64_t{1} : std::int64_t{-1};
        const auto stepY = y0 < y1 ? std::int64_t{1} : std::int64_t{-1};
        auto error = deltaX - deltaY;

        std::vector<domain::CellCoordinate> cells;
        cells.reserve(static_cast<std::size_t>(maxDelta) + 1);
        auto x = x0;
        auto y = y0;
        while (true) {
            cells.push_back(domain::CellCoordinate{static_cast<std::size_t>(x),
                                                   static_cast<std::size_t>(y)});
            if (x == x1 && y == y1) {
                break;
            }

            const auto negativeHalfDeltaY = -(deltaY / 2) - deltaY % 2;
            const auto positiveHalfDeltaX = deltaX / 2 + deltaX % 2;
            const auto stepInX = error > negativeHalfDeltaY;
            const auto stepInY = error < positiveHalfDeltaX;
            if (stepInX) {
                error -= deltaY;
                x += stepX;
            }
            if (stepInY) {
                error += deltaX;
                y += stepY;
            }
        }

        if (reverseOutput) {
            std::reverse(cells.begin(), cells.end());
        }
        return cells;
    }

} // namespace lifeGame::presentation
