#include <catch2/catch_test_macros.hpp>
#include <presentation/ui/toolbar.hpp>

namespace {

    using lifeGame::presentation::Toolbar;

    TEST_CASE("Toolbar layout stays in the upper-right with pointer-sized controls") {
        const auto layout = Toolbar::calculateLayout(1280, 720);

        CHECK(layout.panel.x + layout.panel.width <= 1280.0F);
        CHECK(layout.panel.y + layout.panel.height <= 720.0F);
        CHECK(layout.panel.x > 1280.0F / 2.0F);
        REQUIRE(layout.controls.size() == 9);
        CHECK(layout.controls.front().x == layout.panel.x + 4.0F);
        CHECK(layout.controls.front().y == layout.panel.y + 4.0F);

        for (const auto& control : layout.controls) {
            CHECK(control.height >= static_cast<float>(Toolbar::MINIMUM_CONTROL_HEIGHT));
            CHECK(control.x >= layout.panel.x);
            CHECK(control.y >= layout.panel.y);
            CHECK(control.x + control.width <= layout.panel.x + layout.panel.width);
            CHECK(control.y + control.height <= layout.panel.y + layout.panel.height);
        }

        CHECK(layout.status.y > layout.controls.back().y);
    }

} // namespace
