#include <cstdint>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <domain/field/field-dimensions.hpp>
#include <domain/session/session.hpp>

namespace {

    using lifeGame::domain::FieldDimensions;
    using lifeGame::domain::Session;

    TEST_CASE("A session owns a fresh fixed rectangular Field") {
        const auto dimensions = FieldDimensions::create(7, 3);
        REQUIRE(dimensions);

        const auto session = Session::create("Experiment", dimensions.value(), 17);

        REQUIRE(session);
        CHECK(session.value().id() == 17);
        CHECK(session.value().name().displayName() == "Experiment");
        CHECK(session.value().dimensions().width == 7);
        CHECK(session.value().dimensions().height == 3);
        CHECK(session.value().field().cells().size() == 21);
        CHECK(session.value().field().cells() == std::vector<std::uint8_t>(21, 0));
    }

    TEST_CASE("A session rejects an invalid name before creating its Field") {
        const auto dimensions = FieldDimensions::create(5, 5);
        REQUIRE(dimensions);

        const auto session = Session::create("   ", dimensions.value());

        CHECK_FALSE(session);
    }

} // namespace
