#pragma once

#include <application/commands/field-command.hpp>
#include <domain/field/field.hpp>

namespace lifeGame::application {

    class FieldCommandExecutor {
      public:
        static void execute(domain::Field& field, const PaintLiveCommand& command) noexcept;
    };

} // namespace lifeGame::application
