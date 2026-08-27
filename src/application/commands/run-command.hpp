#pragma once

namespace lifeGame::application {

    enum class RunState {
        Running,
        Paused,
    };

    struct PauseCommand {};

    struct ResumeCommand {};

} // namespace lifeGame::application
