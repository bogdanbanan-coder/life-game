#include <chrono>

#include <application/configuration/settings-service.hpp>
#include <application/session/session-service.hpp>
#include <catch2/catch_test_macros.hpp>
#include <presentation/application/raylib-application.hpp>
#include <presentation/rendering/field-renderer.hpp>
#include <presentation/ui/toolbar.hpp>

namespace {

    using namespace std::chrono_literals;
    using lifeGame::application::SettingsService;
    using lifeGame::application::SessionService;
    using lifeGame::presentation::FrameInput;
    using lifeGame::presentation::FieldRenderer;
    using lifeGame::presentation::LogicalPoint;
    using lifeGame::presentation::PointerSample;
    using lifeGame::presentation::RaylibApplication;
    using lifeGame::presentation::StartScreen;
    using lifeGame::presentation::Toolbar;

    auto clickAt(LogicalPoint point) -> FrameInput {
        return FrameInput{1280, 720, PointerSample{point, true, true, false}};
    }

    TEST_CASE("RaylibApplication starts at StartScreen and opens a configured session") {
        SettingsService settings;
        SessionService sessions{settings};
        RaylibApplication application{settings, sessions};

        REQUIRE_FALSE(application.hasActiveSession());
        REQUIRE(application.startScreen() != nullptr);
        const auto startLayout = StartScreen::calculateLayout(1280, 720);
        static_cast<void>(application.processIteration(
            2s, clickAt(LogicalPoint{startLayout.settings.x + 4.0F,
                                      startLayout.settings.y + 4.0F})));

        auto* screen = application.startScreen();
        REQUIRE(screen != nullptr);
        REQUIRE(screen->settingsOpen());
        screen->settingsPanel().setWidthText("4");
        screen->settingsPanel().setHeightText("3");
        screen->settingsPanel().setIntervalText("0.5");
        const auto settingsLayout =
            lifeGame::presentation::SettingsPanel::calculateLayout(1280, 720);
        static_cast<void>(application.processIteration(
            0ms, clickAt(LogicalPoint{settingsLayout.save.x + 4.0F,
                                      settingsLayout.save.y + 4.0F})));

        static_cast<void>(application.processIteration(
            0ms, clickAt(LogicalPoint{startLayout.create.x + 4.0F,
                                      startLayout.create.y + 4.0F})));
        screen = application.startScreen();
        REQUIRE(screen != nullptr);
        screen->nameDialog().setText("Configured");
        const auto nameLayout = lifeGame::presentation::NameDialog::calculateLayout(1280, 720);
        static_cast<void>(application.processIteration(
            0ms, clickAt(LogicalPoint{nameLayout.save.x + 4.0F, nameLayout.save.y + 4.0F})));

        REQUIRE(application.hasActiveSession());
        CHECK(application.paintMode() == lifeGame::application::PaintMode::Live);
        CHECK(application.runState() == lifeGame::application::RunState::Running);
        REQUIRE(application.activeField() != nullptr);
        CHECK(application.activeField()->dimensions().width == 4);
        CHECK(application.activeField()->dimensions().height == 3);
    }

    TEST_CASE("Creating a session starts its configured interval fresh") {
        SettingsService settings;
        SessionService sessions{settings};
        RaylibApplication application{settings, sessions};
        const auto startLayout = StartScreen::calculateLayout(1280, 720);
        static_cast<void>(application.processIteration(
            0ms, clickAt(LogicalPoint{startLayout.settings.x + 4.0F,
                                      startLayout.settings.y + 4.0F})));

        auto* screen = application.startScreen();
        REQUIRE(screen != nullptr);
        screen->settingsPanel().setWidthText("4");
        screen->settingsPanel().setHeightText("3");
        screen->settingsPanel().setIntervalText("0.5");
        const auto settingsLayout =
            lifeGame::presentation::SettingsPanel::calculateLayout(1280, 720);
        static_cast<void>(application.processIteration(
            0ms, clickAt(LogicalPoint{settingsLayout.save.x + 4.0F,
                                      settingsLayout.save.y + 4.0F})));

        static_cast<void>(application.processIteration(
            0ms, clickAt(LogicalPoint{startLayout.create.x + 4.0F,
                                      startLayout.create.y + 4.0F})));
        screen = application.startScreen();
        REQUIRE(screen != nullptr);
        screen->nameDialog().setText("Fresh interval");
        const auto nameLayout = lifeGame::presentation::NameDialog::calculateLayout(1280, 720);
        static_cast<void>(application.processIteration(
            0ms, clickAt(LogicalPoint{nameLayout.save.x + 4.0F, nameLayout.save.y + 4.0F})));

        const auto activeSessionId = application.activeSessionId();
        REQUIRE(activeSessionId);
        auto* session = sessions.find(*activeSessionId);
        REQUIRE(session != nullptr);
        REQUIRE(session->field().setLive({1, 1}, true));

        const FrameInput noInput{1280, 720, PointerSample{{0.0F, 0.0F}, false, false, false}};
        static_cast<void>(application.processIteration(499ms, noInput));
        CHECK(session->field().isLive({1, 1}));
        static_cast<void>(application.processIteration(1ms, noInput));
        CHECK_FALSE(session->field().isLive({1, 1}));
    }

    TEST_CASE("Opening an existing session preserves cells and uses current interval") {
        SettingsService settings;
        SessionService sessions{settings};
        RaylibApplication application{settings, sessions};
        const auto id = sessions.create("Existing");
        REQUIRE(id);
        REQUIRE(sessions.find(id.value())->field().setLive({1, 1}, true));
        REQUIRE(settings.saveFromText("50", "50", "0.5"));

        const auto card = StartScreen::sessionCardBounds(
            StartScreen::calculateLayout(1280, 720), 0);
        static_cast<void>(application.processIteration(
            0ms, clickAt(LogicalPoint{card.x + 4.0F, card.y + 4.0F})));
        REQUIRE(application.hasActiveSession());
        CHECK(application.activeField()->dimensions().width == 50);
        CHECK(application.activeField()->isLive({1, 1}));

        const FrameInput noInput{1280, 720, PointerSample{{0.0F, 0.0F}, false, false, false}};
        static_cast<void>(application.processIteration(499ms, noInput));
        CHECK(application.activeField()->isLive({1, 1}));
        static_cast<void>(application.processIteration(1ms, noInput));
        CHECK_FALSE(application.activeField()->isLive({1, 1}));

        const auto toolbar = Toolbar::calculateLayout(1280, 720);
        const auto exit = toolbar.controls[Toolbar::EXIT_CONTROL_INDEX];
        static_cast<void>(application.processIteration(
            0ms, clickAt(LogicalPoint{exit.x + 4.0F, exit.y + 4.0F})));
        CHECK_FALSE(application.hasActiveSession());
        CHECK_FALSE(sessions.find(id.value())->field().isLive({1, 1}));
    }

    TEST_CASE("Opening a session resets camera and field mode without changing its cells") {
        SettingsService settings;
        SessionService sessions{settings};
        const auto first = sessions.create("First");
        const auto second = sessions.create("Second");
        REQUIRE(first);
        REQUIRE(second);
        REQUIRE(sessions.find(second.value())->field().setLive({1, 1}, true));
        RaylibApplication application{settings, sessions};

        const auto startLayout = StartScreen::calculateLayout(1280, 720);
        const auto firstCard = StartScreen::sessionCardBounds(startLayout, 0);
        static_cast<void>(application.processIteration(
            0ms, clickAt(LogicalPoint{firstCard.x + 4.0F, firstCard.y + 4.0F})));
        REQUIRE(application.hasActiveSession());

        const auto move = Toolbar::calculateLayout(1280, 720)
                              .controls[Toolbar::MOVE_CONTROL_INDEX];
        static_cast<void>(application.processIteration(
            0ms, clickAt(LogicalPoint{move.x + 4.0F, move.y + 4.0F})));
        const auto plan = FieldRenderer::calculateRenderPlan(
            *application.activeField(), 1280, 720, application.cameraState());
        const auto cellSize = static_cast<float>(plan.cellSize);
        const auto start = LogicalPoint{plan.fieldRectangle.x + 20.5F * cellSize,
                                        plan.fieldRectangle.y + 20.5F * cellSize};
        const auto finish = LogicalPoint{start.x + 2.0F * cellSize,
                                         start.y + cellSize};
        static_cast<void>(application.processIteration(
            0ms, FrameInput{1280, 720, PointerSample{start, true, true, false}}));
        static_cast<void>(application.processIteration(
            0ms, FrameInput{1280, 720, PointerSample{finish, false, true, false}}));
        static_cast<void>(application.processIteration(
            0ms, FrameInput{1280, 720, PointerSample{finish, false, false, true}}));
        REQUIRE(application.cameraState().x != 0.0F);
        REQUIRE(application.cameraState().y != 0.0F);
        REQUIRE(application.paintMode() == lifeGame::application::PaintMode::Move);

        const auto exit = Toolbar::calculateLayout(1280, 720)
                              .controls[Toolbar::EXIT_CONTROL_INDEX];
        static_cast<void>(application.processIteration(
            0ms, clickAt(LogicalPoint{exit.x + 4.0F, exit.y + 4.0F})));
        REQUIRE_FALSE(application.hasActiveSession());

        const auto secondCard = StartScreen::sessionCardBounds(startLayout, 1);
        static_cast<void>(application.processIteration(
            0ms, clickAt(LogicalPoint{secondCard.x + 4.0F, secondCard.y + 4.0F})));

        REQUIRE(application.hasActiveSession());
        CHECK(application.activeSessionId().value() == second.value());
        CHECK(application.cameraState().x == 0.0F);
        CHECK(application.cameraState().y == 0.0F);
        CHECK(application.paintMode() == lifeGame::application::PaintMode::Live);
        CHECK(application.activeField()->dimensions().width == 50);
        CHECK(application.activeField()->dimensions().height == 50);
        CHECK(application.activeField()->isLive({1, 1}));
    }

} // namespace
