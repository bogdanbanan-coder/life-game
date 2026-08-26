#pragma once

#include <domain/field/field.hpp>
#include <presentation/screens/field-screen.hpp>

namespace lifeGame::presentation {

    class RaylibApplication {
      public:
        explicit RaylibApplication(domain::Field field);

        [[nodiscard]] int run();

      private:
        domain::Field field_;
        FieldScreen fieldScreen_;
    };

} // namespace lifeGame::presentation
