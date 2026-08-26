#include <application/field-command-executor.hpp>

namespace lifeGame::application {

    void FieldCommandExecutor::execute(domain::Field& field,
                                        const PaintLiveCommand& command) noexcept {
        static_cast<void>(field.setLive(command.coordinate, true));
    }

} // namespace lifeGame::application
