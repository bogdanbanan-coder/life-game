#include <string_view>
#include <application/configuration/settings-service.hpp>
#include <application/session/session-service.hpp>
#include <presentation/application/raylib-application.hpp>

int main(int argc, char* argv[]) {
    if (argc == 2 && std::string_view{argv[1]} == "--smoke-test") {
        return 0;
    }

    lifeGame::application::SettingsService settingsService;
    lifeGame::application::SessionService sessionService{settingsService};
    lifeGame::presentation::RaylibApplication application{settingsService, sessionService};
    return application.run();
}
