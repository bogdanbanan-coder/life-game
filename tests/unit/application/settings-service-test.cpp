#include <chrono>
#include <string>

#include <application/configuration/settings-service.hpp>
#include <catch2/catch_test_macros.hpp>
#include <foundation/error-code.hpp>

namespace {

    using namespace std::chrono_literals;
    using lifeGame::application::SettingsService;
    using lifeGame::application::SimulationSettings;
    using lifeGame::foundation::ErrorCode;

    TEST_CASE("Settings start with the approved defaults") {
        const SettingsService service;

        CHECK(service.activeSettings().width() == 50);
        CHECK(service.activeSettings().height() == 50);
        CHECK(service.activeSettings().generationInterval() == 250ms);
    }

    TEST_CASE("Settings parse positive base-10 seconds without integer rounding") {
        const auto settings = SimulationSettings::createFromText("12", "8", "1.5");

        REQUIRE(settings);
        CHECK(settings.value().width() == 12);
        CHECK(settings.value().height() == 8);
        CHECK(settings.value().generationIntervalSeconds() == 1.5L);
        CHECK_FALSE(SimulationSettings::parseGenerationInterval("0"));
        CHECK_FALSE(SimulationSettings::parseGenerationInterval("-1"));
        CHECK_FALSE(SimulationSettings::parseGenerationInterval("1,5"));
        CHECK_FALSE(SimulationSettings::parseGenerationInterval("nan"));
        CHECK_FALSE(SimulationSettings::parseGenerationInterval("inf"));
    }

    TEST_CASE("Settings reject intervals whose decimal scale cannot be represented") {
        std::string text = "1.";
        text.append(5'000, '0');
        text.back() = '1';

        CHECK_FALSE(SimulationSettings::parseGenerationInterval(text));
    }

    TEST_CASE("Settings Save validates and commits all values atomically") {
        SettingsService service;
        const auto before = service.activeSettings();

        CHECK_FALSE(service.saveFromText("0", "8", "1.5"));
        CHECK(service.activeSettings() == before);

        REQUIRE(service.saveFromText("12", "8", "1.5"));
        CHECK(service.activeSettings().width() == 12);
        CHECK(service.activeSettings().height() == 8);
        CHECK(service.activeSettings().generationIntervalSeconds() == 1.5L);
    }

    TEST_CASE("A failed Settings Save keeps the active value and staged candidate available") {
        SettingsService service;
        service.failNextSave(ErrorCode::PersistenceWriteFailed);
        const auto candidate = SimulationSettings::createFromText("12", "8", "1.5");
        REQUIRE(candidate);

        const auto result = service.save(candidate.value());

        REQUIRE_FALSE(result);
        CHECK(result.error() == ErrorCode::PersistenceWriteFailed);
        CHECK(service.activeSettings().width() == 50);
        CHECK(service.activeSettings().height() == 50);
        CHECK(service.activeSettings().generationInterval() == 250ms);
        REQUIRE(service.save(candidate.value()));
        CHECK(service.activeSettings().width() == 12);
    }

} // namespace
