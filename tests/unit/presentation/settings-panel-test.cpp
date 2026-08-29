#include <application/configuration/settings-service.hpp>
#include <application/session/session-service.hpp>
#include <catch2/catch_test_macros.hpp>
#include <presentation/ui/settings-panel.hpp>
#include <presentation/ui/numeric-field.hpp>

namespace {

    using lifeGame::application::SessionService;
    using lifeGame::application::SettingsService;
    using lifeGame::presentation::NumericField;
    using lifeGame::presentation::NumericFieldKind;
    using lifeGame::presentation::SettingsPanel;

    TEST_CASE("SettingsPanel shows defaults and disables Save for invalid edits") {
        SettingsService settings;
        SettingsPanel panel{settings};
        panel.open();

        CHECK(panel.widthText() == "50");
        CHECK(panel.heightText() == "50");
        CHECK(panel.intervalText() == "0.25");
        CHECK(panel.canSave());

        panel.setWidthText("not a dimension");

        CHECK(panel.widthText() == "not a dimension");
        CHECK_FALSE(panel.canSave());
        CHECK_FALSE(panel.widthField().validationMessage().empty());
        CHECK(settings.activeSettings().width() == 50);
    }

    TEST_CASE("SettingsPanel render plan exposes all global fields and local validation") {
        SettingsService settings;
        SettingsPanel panel{settings};
        panel.open();

        const auto defaults = panel.calculateRenderPlan();
        CHECK(defaults.labels[0] == "Field width (cells)");
        CHECK(defaults.labels[1] == "Field height (cells)");
        CHECK(defaults.labels[2] == "Generation interval (seconds)");
        CHECK(defaults.values[0] == "50");
        CHECK(defaults.values[1] == "50");
        CHECK(defaults.values[2] == "0.25");
        CHECK(defaults.saveEnabled);

        panel.setHeightText("0");
        panel.setIntervalText("0");
        const auto invalid = panel.calculateRenderPlan();
        CHECK_FALSE(invalid.saveEnabled);
        CHECK_FALSE(invalid.validation[1].empty());
        CHECK_FALSE(invalid.validation[2].empty());
    }

    TEST_CASE("NumericField validates dimension bounds independently") {
        NumericField field{NumericFieldKind::Dimension};

        field.setText("0");
        CHECK_FALSE(field.isValid());
        field.setText("4097");
        CHECK_FALSE(field.isValid());
        field.setText("1");
        CHECK(field.isValid());
    }

    TEST_CASE("SettingsPanel keeps all invalid numeric candidates local") {
        SettingsService settings;
        SettingsPanel panel{settings};
        panel.open();

        panel.setWidthText("4097");
        CHECK_FALSE(panel.canSave());
        panel.setWidthText("4096");
        panel.setHeightText("1025");
        CHECK_FALSE(panel.canSave());

        panel.setWidthText("50");
        panel.setHeightText("50");
        for (const auto* invalidInterval : {"-1", "nan", "inf"}) {
            panel.setIntervalText(invalidInterval);
            CHECK_FALSE(panel.canSave());
        }

        CHECK(settings.activeSettings().width() == 50);
        CHECK(settings.activeSettings().height() == 50);
        CHECK(settings.activeSettings().generationIntervalSeconds() == 0.25L);
    }

    TEST_CASE("SettingsPanel Cancel discards staged values") {
        SettingsService settings;
        SettingsPanel panel{settings};
        panel.open();
        panel.setWidthText("12");
        panel.setHeightText("8");
        panel.setIntervalText("1.5");
        panel.cancel();

        CHECK_FALSE(panel.isOpen());
        CHECK(settings.activeSettings().width() == 50);
        CHECK(settings.activeSettings().height() == 50);
        CHECK(settings.activeSettings().generationIntervalSeconds() == 0.25L);
    }

    TEST_CASE("SettingsPanel preserves valid staged values when Save fails") {
        SettingsService settings;
        SessionService sessions{settings};
        SettingsPanel panel{settings};
        panel.open();
        panel.setWidthText("12");
        panel.setHeightText("8");
        panel.setIntervalText("1.5");
        settings.failNextSave(lifeGame::foundation::ErrorCode::PersistenceWriteFailed);

        const auto result = panel.save();

        REQUIRE_FALSE(result);
        CHECK(panel.isOpen());
        CHECK(panel.widthText() == "12");
        CHECK(panel.heightText() == "8");
        CHECK(panel.intervalText() == "1.5");
        CHECK(settings.activeSettings().width() == 50);
        CHECK(sessions.sessions().empty());
    }

} // namespace
