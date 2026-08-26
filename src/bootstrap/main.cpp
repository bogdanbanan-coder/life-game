#include <string_view>
#include <utility>

#include <domain/field/field.hpp>
#include <presentation/application/raylib-application.hpp>

int main(int argc, char* argv[]) {
    auto field = lifeGame::domain::Field::create(50, 50);
    if (!field) {
        return 1;
    }

    if (argc == 2 && std::string_view{argv[1]} == "--smoke-test") {
        return 0;
    }

    lifeGame::presentation::RaylibApplication application{std::move(field).value()};
    return application.run();
}
