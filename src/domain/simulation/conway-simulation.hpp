#pragma once

#include <domain/field/field.hpp>

namespace lifeGame::domain {

    class ConwaySimulation {
      public:
        /// Advances one finite Conway Life generation without allocating per cell.
        /// Every out-of-bounds neighbor is dead, and the complete next buffer is published
        /// only after all cells have been evaluated from the prior buffer.
        static void advance(Field& field) noexcept;
    };

} // namespace lifeGame::domain
