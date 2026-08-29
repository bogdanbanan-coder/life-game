#include <application/configuration/settings-service.hpp>
#include <application/session/session-service.hpp>
#include <catch2/catch_test_macros.hpp>

namespace {

    using lifeGame::application::SessionService;
    using lifeGame::application::SettingsService;

    TEST_CASE("Session creation captures current dimensions and enforces normalized uniqueness") {
        SettingsService settings;
        SessionService sessions{settings};

        const auto first = sessions.create("  Alpha  ");
        REQUIRE(first);
        auto* firstSession = sessions.find(first.value());
        REQUIRE(firstSession != nullptr);
        REQUIRE(firstSession->field().setLive({1, 1}, true));

        CHECK_FALSE(sessions.create("alpha"));
        CHECK(sessions.sessions().size() == 1);

        REQUIRE(settings.saveFromText("3", "2", "0.5"));
        const auto second = sessions.create("Beta");
        REQUIRE(second);
        firstSession = sessions.find(first.value());
        const auto* secondSession = sessions.find(second.value());
        REQUIRE(secondSession != nullptr);
        REQUIRE(firstSession != nullptr);
        CHECK(firstSession->dimensions().width == 50);
        CHECK(firstSession->dimensions().height == 50);
        CHECK(firstSession->field().isLive({1, 1}));
        CHECK(secondSession->dimensions().width == 3);
        CHECK(secondSession->dimensions().height == 2);
    }

    TEST_CASE("SessionService rejects Unicode-equivalent duplicate names") {
        SettingsService settings;
        SessionService sessions{settings};

        REQUIRE(sessions.create("Stra\xC3\x9F" "e"));
        CHECK_FALSE(sessions.create("STRASSE"));

        REQUIRE(sessions.create("\xCE\xB1\xCC\x81"));
        CHECK_FALSE(sessions.create("\xCE\xAC"));
        CHECK(sessions.sessions().size() == 2);
    }

    TEST_CASE("Session summaries are stably case-insensitive sorted") {
        SettingsService settings;
        SessionService sessions{settings};

        REQUIRE(sessions.create("zeta"));
        REQUIRE(sessions.create("Alpha"));
        REQUIRE(sessions.create("beta"));

        const auto summaries = sessions.summaries();
        REQUIRE(summaries.size() == 3);
        CHECK(summaries[0].displayName == "Alpha");
        CHECK(summaries[1].displayName == "beta");
        CHECK(summaries[2].displayName == "zeta");
    }

    TEST_CASE("Opening a session returns the same mutable aggregate") {
        SettingsService settings;
        SessionService sessions{settings};
        const auto created = sessions.create("Open me");
        REQUIRE(created);

        const auto opened = sessions.open(created.value());
        REQUIRE(opened);
        REQUIRE(opened.value()->field().setLive({4, 4}, true));

        const auto reopened = sessions.open(" open ME ");
        REQUIRE(reopened);
        CHECK(reopened.value()->field().isLive({4, 4}));
    }

    TEST_CASE("Session handles remain stable while sessions are added within the capacity") {
        SettingsService settings;
        SessionService sessions{settings};
        const auto first = sessions.create("First");
        REQUIRE(first);
        auto* firstSession = sessions.find(first.value());
        REQUIRE(firstSession != nullptr);

        REQUIRE(sessions.create("Second"));

        CHECK(firstSession == sessions.find(first.value()));
        CHECK(firstSession->name().displayName() == "First");
    }

} // namespace
