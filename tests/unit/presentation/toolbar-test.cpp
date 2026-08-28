#include <cstddef>
#include <string_view>

#include <catch2/catch_test_macros.hpp>
#include <application/commands/field-command.hpp>
#include <application/commands/run-command.hpp>
#include <presentation/ui/toolbar.hpp>

namespace {

    using lifeGame::presentation::Toolbar;
    using lifeGame::presentation::ToolbarButtonStyle;
    using lifeGame::application::PaintMode;
    using lifeGame::application::RunState;

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

    TEST_CASE("Toolbar labels reflect the application run state") {
        CHECK(Toolbar::pauseControlLabel(RunState::Running) == "Pause");
        CHECK(Toolbar::pauseControlLabel(RunState::Paused) == "Resume");
        CHECK(Toolbar::runStateLabel(RunState::Running) == "Running");
        CHECK(Toolbar::runStateLabel(RunState::Paused) == "Paused");
    }

    TEST_CASE("Toolbar exposes exactly one active paint mode and its textual cue") {
        CHECK(Toolbar::activePaintModeControl(PaintMode::Live) == Toolbar::LIVE_CONTROL_INDEX);
        CHECK(Toolbar::activePaintModeControl(PaintMode::Die) == Toolbar::DIE_CONTROL_INDEX);
        CHECK(Toolbar::activePaintModeControl(PaintMode::Live) !=
              Toolbar::activePaintModeControl(PaintMode::Die));
        CHECK(Toolbar::paintModeLabel(PaintMode::Live) == "Live");
        CHECK(Toolbar::paintModeLabel(PaintMode::Die) == "Die");
    }

    TEST_CASE("Toolbar status names the active tool and run state") {
        CHECK(Toolbar::activeStatusLabel(PaintMode::Live, RunState::Running) ==
              "Active: Live | Running");
        CHECK(Toolbar::activeStatusLabel(PaintMode::Live, RunState::Paused) ==
              "Active: Live | Paused");
        CHECK(Toolbar::activeStatusLabel(PaintMode::Die, RunState::Running) ==
              "Active: Die | Running");
        CHECK(Toolbar::activeStatusLabel(PaintMode::Die, RunState::Paused) ==
              "Active: Die | Paused");
    }

    TEST_CASE("Toolbar render plan marks one mode active with its visible labels") {
        const auto checkPlan = [](PaintMode paintMode, RunState runState,
                                  std::size_t expectedActiveControl,
                                  std::string_view expectedPaintLabel,
                                  std::string_view expectedRunLabel,
                                  std::string_view expectedStatusLabel) {
            const auto plan = Toolbar::calculateRenderPlan(paintMode, runState);
            REQUIRE(expectedActiveControl < plan.controls.size());

            auto activeControlCount = std::size_t{0};
            for (std::size_t index = 0; index < plan.controls.size(); ++index) {
                if (plan.controls[index].style == ToolbarButtonStyle::Active) {
                    ++activeControlCount;
                } else {
                    CHECK(plan.controls[index].style == ToolbarButtonStyle::Rest);
                }
            }

            CHECK(activeControlCount == 1);
            CHECK(plan.controls[Toolbar::LIVE_CONTROL_INDEX].label == "Live");
            CHECK(plan.controls[Toolbar::DIE_CONTROL_INDEX].label == "Die");
            CHECK(plan.controls[expectedActiveControl].style == ToolbarButtonStyle::Active);
            CHECK(plan.controls[expectedActiveControl].label == expectedPaintLabel);
            CHECK(plan.controls[Toolbar::RUN_CONTROL_INDEX].label == expectedRunLabel);
            CHECK(plan.statusLabel == expectedStatusLabel);
        };

        CHECK(Toolbar::ACTIVE_OUTLINE_WIDTH == 2.0F);
        checkPlan(PaintMode::Live, RunState::Running, Toolbar::LIVE_CONTROL_INDEX, "Live",
                  "Pause", "Active: Live | Running");
        checkPlan(PaintMode::Live, RunState::Paused, Toolbar::LIVE_CONTROL_INDEX, "Live",
                  "Resume", "Active: Live | Paused");
        checkPlan(PaintMode::Die, RunState::Running, Toolbar::DIE_CONTROL_INDEX, "Die", "Pause",
                  "Active: Die | Running");
        checkPlan(PaintMode::Die, RunState::Paused, Toolbar::DIE_CONTROL_INDEX, "Die", "Resume",
                  "Active: Die | Paused");
    }

} // namespace
