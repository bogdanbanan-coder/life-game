#include <application/field-command-executor.hpp>

#include <variant>

namespace lifeGame::application {

    void FieldCommandExecutor::execute(domain::Field& field,
                                        const PaintLiveCommand& command) noexcept {
        static_cast<void>(field.setLive(command.coordinate, true));
    }

    void FieldCommandExecutor::execute(domain::Field& field,
                                       const PaintDeadCommand& command) noexcept {
        static_cast<void>(field.setLive(command.coordinate, false));
    }

    void FieldCommandExecutor::execute(domain::Field& field,
                                       const PaintCommand& command) noexcept {
        std::visit(
            [&field](const auto& typedCommand) {
                FieldCommandExecutor::execute(field, typedCommand);
            },
            command);
    }

} // namespace lifeGame::application
