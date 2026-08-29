#pragma once

namespace lifeGame::application {

    enum class RunState {
        Running,
        Paused,
    };

    struct PauseCommand {};

    struct ResumeCommand {};

    struct ExitSessionCommand {};

} // namespace lifeGame::application
