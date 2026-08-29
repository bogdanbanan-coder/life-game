#include <application/configuration/settings-service.hpp>
#include <application/session/session-service.hpp>
#include <catch2/catch_test_macros.hpp>
#include <presentation/screens/start-screen.hpp>

namespace {

    using lifeGame::application::SessionService;
    using lifeGame::application::SettingsService;
    using lifeGame::presentation::LogicalPoint;
    using lifeGame::presentation::ErrorDialog;
    using lifeGame::presentation::NameDialog;
    using lifeGame::presentation::PointerSample;
    using lifeGame::presentation::SettingsPanel;
    using lifeGame::presentation::StartScreen;
    using lifeGame::presentation::StartScreenActionKind;

    auto clickAt(LogicalPoint point) -> PointerSample {
        return PointerSample{point, true, true, false};
    }

    TEST_CASE("StartScreen exposes the empty session surface and modal ownership") {
        SettingsService settings;
        SessionService sessions{settings};
        StartScreen screen{settings, sessions};
        const auto layout = StartScreen::calculateLayout(1280, 720);

        CHECK(screen.calculateRenderPlan().sessionCount == 0);
        CHECK_FALSE(screen.modalOwnsInput());

        static_cast<void>(screen.processInput(
            1280, 720,
            clickAt(LogicalPoint{layout.settings.x + 4.0F, layout.settings.y + 4.0F})));
        CHECK(screen.settingsOpen());
        CHECK(screen.modalOwnsInput());
    }

    TEST_CASE("StartScreen settings save and Create are direct, synchronous flows") {
        SettingsService settings;
        SessionService sessions{settings};
        StartScreen screen{settings, sessions};
        const auto startLayout = StartScreen::calculateLayout(1280, 720);

        static_cast<void>(screen.processInput(
            1280, 720,
            clickAt(LogicalPoint{startLayout.settings.x + 4.0F, startLayout.settings.y + 4.0F})));
        screen.settingsPanel().setWidthText("4");
        screen.settingsPanel().setHeightText("3");
        screen.settingsPanel().setIntervalText("0.5");
        const auto settingsLayout = SettingsPanel::calculateLayout(1280, 720);
        static_cast<void>(screen.processInput(
            1280, 720,
            clickAt(LogicalPoint{settingsLayout.save.x + 4.0F, settingsLayout.save.y + 4.0F})));

        CHECK_FALSE(screen.settingsOpen());
        CHECK(settings.activeSettings().width() == 4);
        CHECK(settings.activeSettings().height() == 3);
        CHECK(screen.statusMessage().visible());
        CHECK(screen.statusMessage().text() == "Settings saved.");

        static_cast<void>(screen.processInput(
            1280, 720,
            clickAt(LogicalPoint{startLayout.create.x + 4.0F, startLayout.create.y + 4.0F})));
        screen.nameDialog().setText("  Small  ");
        const auto nameLayout = NameDialog::calculateLayout(1280, 720);
        const auto action = screen.processInput(
            1280, 720,
            clickAt(LogicalPoint{nameLayout.save.x + 4.0F, nameLayout.save.y + 4.0F}));

        REQUIRE(action.kind == StartScreenActionKind::OpenSession);
        REQUIRE(action.sessionId);
        REQUIRE(sessions.find(*action.sessionId) != nullptr);
        CHECK(sessions.find(*action.sessionId)->dimensions().width == 4);
        CHECK(sessions.find(*action.sessionId)->dimensions().height == 3);
    }

    TEST_CASE("StartScreen keeps invalid and duplicate names in the dialog") {
        SettingsService settings;
        SessionService sessions{settings};
        StartScreen screen{settings, sessions};
        const auto layout = StartScreen::calculateLayout(1280, 720);

        static_cast<void>(screen.processInput(
            1280, 720,
            clickAt(LogicalPoint{layout.create.x + 4.0F, layout.create.y + 4.0F})));
        screen.nameDialog().setText("First");
        REQUIRE(screen.nameDialog().commit());

        screen.nameDialog().open();
        screen.nameDialog().setText("first");
        CHECK_FALSE(screen.nameDialog().canSave());
        CHECK(screen.nameDialog().text() == "first");
        CHECK(sessions.sessions().size() == 1);
    }

    TEST_CASE("StartScreen routes card clicks using sorted session order") {
        SettingsService settings;
        SessionService sessions{settings};
        StartScreen screen{settings, sessions};
        const auto zeta = sessions.create("zeta");
        const auto alpha = sessions.create("Alpha");
        REQUIRE(zeta);
        REQUIRE(alpha);

        const auto layout = StartScreen::calculateLayout(1280, 720);
        const auto firstCard = StartScreen::sessionCardBounds(layout, 0);
        const auto action = screen.processInput(
            1280, 720,
            clickAt(LogicalPoint{firstCard.x + 4.0F, firstCard.y + 4.0F}));

        REQUIRE(action.kind == StartScreenActionKind::OpenSession);
        REQUIRE(action.sessionId);
        CHECK(*action.sessionId == alpha.value());
    }

    TEST_CASE("StartScreen keeps rejected Create clicks in the modal") {
        SettingsService settings;
        SessionService sessions{settings};
        StartScreen screen{settings, sessions};
        const auto layout = StartScreen::calculateLayout(1280, 720);
        static_cast<void>(screen.processInput(
            1280, 720,
            clickAt(LogicalPoint{layout.create.x + 4.0F, layout.create.y + 4.0F})));
        const auto dialogLayout = NameDialog::calculateLayout(1280, 720);

        const auto invalidAction = screen.processInput(
            1280, 720,
            clickAt(LogicalPoint{dialogLayout.save.x + 4.0F, dialogLayout.save.y + 4.0F}));
        CHECK(invalidAction.kind == StartScreenActionKind::None);
        CHECK(screen.createDialogOpen());
        CHECK(sessions.sessions().empty());

        screen.nameDialog().setText("First");
        const auto createdAction = screen.processInput(
            1280, 720,
            clickAt(LogicalPoint{dialogLayout.save.x + 4.0F, dialogLayout.save.y + 4.0F}));
        REQUIRE(createdAction.kind == StartScreenActionKind::OpenSession);
        REQUIRE(createdAction.sessionId);

        screen.nameDialog().open("first");
        const auto duplicateAction = screen.processInput(
            1280, 720,
            clickAt(LogicalPoint{dialogLayout.save.x + 4.0F, dialogLayout.save.y + 4.0F}));
        CHECK(duplicateAction.kind == StartScreenActionKind::None);
        CHECK(screen.createDialogOpen());
        CHECK(screen.nameDialog().text() == "first");
        CHECK(sessions.sessions().size() == 1);
    }

    TEST_CASE("ErrorDialog exposes actionable Settings Save failure copy") {
        CHECK(ErrorDialog::messageFor(lifeGame::foundation::ErrorCode::PersistenceWriteFailed) ==
              "The settings could not be saved.");
    }

    TEST_CASE("StartScreen Cancel closes Create without committing or leaking input") {
        SettingsService settings;
        SessionService sessions{settings};
        StartScreen screen{settings, sessions};
        const auto layout = StartScreen::calculateLayout(1280, 720);

        static_cast<void>(screen.processInput(
            1280, 720,
            clickAt(LogicalPoint{layout.create.x + 4.0F, layout.create.y + 4.0F})));
        screen.nameDialog().setText("Pending");
        REQUIRE(screen.createDialogOpen());

        const auto dialogLayout = NameDialog::calculateLayout(1280, 720);
        const auto action = screen.processInput(
            1280, 720,
            clickAt(LogicalPoint{dialogLayout.cancel.x + 4.0F, dialogLayout.cancel.y + 4.0F}));

        CHECK(action.kind == StartScreenActionKind::None);
        CHECK_FALSE(screen.createDialogOpen());
        CHECK_FALSE(screen.modalOwnsInput());
        CHECK(sessions.sessions().empty());
        CHECK(settings.activeSettings().width() == 50);
        CHECK(settings.activeSettings().height() == 50);
    }

    TEST_CASE("StartScreen retries or cancels a failed Settings Save atomically") {
        SettingsService settings;
        SessionService sessions{settings};
        StartScreen screen{settings, sessions};
        const auto startLayout = StartScreen::calculateLayout(1280, 720);
        const auto settingsLayout = SettingsPanel::calculateLayout(1280, 720);

        static_cast<void>(screen.processInput(
            1280, 720,
            clickAt(LogicalPoint{startLayout.settings.x + 4.0F, startLayout.settings.y + 4.0F})));
        screen.settingsPanel().setWidthText("12");
        screen.settingsPanel().setHeightText("8");
        screen.settingsPanel().setIntervalText("1.5");
        settings.failNextSave(lifeGame::foundation::ErrorCode::PersistenceWriteFailed);
        static_cast<void>(screen.processInput(
            1280, 720,
            clickAt(LogicalPoint{settingsLayout.save.x + 4.0F, settingsLayout.save.y + 4.0F})));

        REQUIRE(screen.errorDialog().isOpen());
        REQUIRE(screen.settingsOpen());
        CHECK(settings.activeSettings().width() == 50);
        CHECK(screen.settingsPanel().widthText() == "12");

        const auto errorLayout = ErrorDialog::calculateLayout(1280, 720);
        static_cast<void>(screen.processInput(
            1280, 720,
            clickAt(LogicalPoint{errorLayout.retry.x + 4.0F, errorLayout.retry.y + 4.0F})));

        CHECK_FALSE(screen.errorDialog().isOpen());
        CHECK_FALSE(screen.settingsOpen());
        CHECK(settings.activeSettings().width() == 12);
        CHECK(settings.activeSettings().height() == 8);

        static_cast<void>(screen.processInput(
            1280, 720,
            clickAt(LogicalPoint{startLayout.settings.x + 4.0F, startLayout.settings.y + 4.0F})));
        screen.settingsPanel().setWidthText("20");
        screen.settingsPanel().setHeightText("10");
        screen.settingsPanel().setIntervalText("2");
        settings.failNextSave(lifeGame::foundation::ErrorCode::PersistenceWriteFailed);
        static_cast<void>(screen.processInput(
            1280, 720,
            clickAt(LogicalPoint{settingsLayout.save.x + 4.0F, settingsLayout.save.y + 4.0F})));
        REQUIRE(screen.errorDialog().isOpen());

        static_cast<void>(screen.processInput(
            1280, 720,
            clickAt(LogicalPoint{errorLayout.cancel.x + 4.0F, errorLayout.cancel.y + 4.0F})));

        CHECK_FALSE(screen.errorDialog().isOpen());
        CHECK_FALSE(screen.settingsOpen());
        CHECK(settings.activeSettings().width() == 12);
        CHECK(settings.activeSettings().height() == 8);
        CHECK(settings.activeSettings().generationIntervalSeconds() == 1.5L);
    }

    TEST_CASE("Closed Settings and Create surfaces cannot commit staged values") {
        SettingsService settings;
        SessionService sessions{settings};
        StartScreen screen{settings, sessions};

        screen.settingsPanel().open();
        screen.settingsPanel().setWidthText("12");
        screen.settingsPanel().cancel();
        const auto settingsResult = screen.settingsPanel().save();
        REQUIRE_FALSE(settingsResult);
        CHECK(settingsResult.error() == lifeGame::foundation::ErrorCode::InvalidState);
        CHECK(settings.activeSettings().width() == 50);

        screen.nameDialog().open("Pending");
        screen.nameDialog().cancel();
        const auto sessionResult = screen.nameDialog().commit();
        REQUIRE_FALSE(sessionResult);
        CHECK(sessionResult.error() == lifeGame::foundation::ErrorCode::InvalidState);
        CHECK(sessions.sessions().empty());
    }

} // namespace
