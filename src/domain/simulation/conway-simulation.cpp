#include <domain/simulation/conway-simulation.hpp>

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace lifeGame::domain {

    namespace {

        void countNeighbors(const std::vector<std::uint8_t>& cells, std::size_t width,
                            std::size_t height, std::size_t x, std::size_t y,
                            std::uint8_t& count) noexcept {
            count = 0;

            if (y > 0) {
                const auto row = (y - 1) * width;
                if (x > 0 && cells[row + x - 1] != 0) {
                    ++count;
                }
                if (cells[row + x] != 0) {
                    ++count;
                }
                if (x + 1 < width && cells[row + x + 1] != 0) {
                    ++count;
                }
            }

            const auto row = y * width;
            if (x > 0 && cells[row + x - 1] != 0) {
                ++count;
            }
            if (x + 1 < width && cells[row + x + 1] != 0) {
                ++count;
            }

            if (y + 1 < height) {
                const auto rowBelow = (y + 1) * width;
                if (x > 0 && cells[rowBelow + x - 1] != 0) {
                    ++count;
                }
                if (cells[rowBelow + x] != 0) {
                    ++count;
                }
                if (x + 1 < width && cells[rowBelow + x + 1] != 0) {
                    ++count;
                }
            }
        }

    } // namespace

    void ConwaySimulation::advance(Field& field) noexcept {
        const auto width = field.width();
        const auto height = field.height();
        const auto& current = field.cells_;
        auto& next = field.nextCells_;

        for (std::size_t y = 0; y < height; ++y) {
            for (std::size_t x = 0; x < width; ++x) {
                const auto index = y * width + x;
                std::uint8_t neighbors = 0;
                countNeighbors(current, width, height, x, y, neighbors);

                const auto alive = current[index] != 0;
                const auto survives = alive ? (neighbors == 2 || neighbors == 3)
                                            : neighbors == 3;
                next[index] = survives ? std::uint8_t{1} : std::uint8_t{0};
            }
        }

        std::swap(field.cells_, field.nextCells_);
    }

} // namespace lifeGame::domain
