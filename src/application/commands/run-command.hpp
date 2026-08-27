#pragma once

namespace lifeGame::application {

    enum class RunState {
        Running,
        Paused,
    };

    struct PauseCommand {};

} // namespace lifeGame::application
