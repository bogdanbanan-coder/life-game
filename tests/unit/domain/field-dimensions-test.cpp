#include <limits>

#include <catch2/catch_test_macros.hpp>
#include <domain/field/field-dimensions.hpp>
#include <foundation/error-code.hpp>

namespace {

    using lifeGame::domain::FieldDimensions;
    using lifeGame::foundation::ErrorCode;

    TEST_CASE("FieldDimensions accepts valid dimensions") {
        const auto dimensions = FieldDimensions::create(50, 50);

        REQUIRE(dimensions);
        CHECK(dimensions.value().width == 50);
        CHECK(dimensions.value().height == 50);
        CHECK(dimensions.value().cellCount() == 2'500);
    }

    TEST_CASE("FieldDimensions accepts legal side boundaries") {
        const auto oneByOne = FieldDimensions::create(1, 1);
        const auto maxWidth = FieldDimensions::create(4'096, 1);
        const auto maxHeight = FieldDimensions::create(1, 4'096);

        REQUIRE(oneByOne);
        CHECK(oneByOne.value().cellCount() == 1);

        REQUIRE(maxWidth);
        CHECK(maxWidth.value().cellCount() == 4'096);

        REQUIRE(maxHeight);
        CHECK(maxHeight.value().cellCount() == 4'096);
    }

    TEST_CASE("FieldDimensions accepts the total-cell limit") {
        const auto dimensions = FieldDimensions::create(2'048, 2'048);

        REQUIRE(dimensions);
        CHECK(dimensions.value().cellCount() == FieldDimensions::MAX_TOTAL_CELLS);
    }

    TEST_CASE("FieldDimensions rejects zero and over-limit sides") {
        CHECK(FieldDimensions::create(0, 50).error() == ErrorCode::InvalidArgument);
        CHECK(FieldDimensions::create(50, 0).error() == ErrorCode::InvalidArgument);
        CHECK(FieldDimensions::create(4'097, 1).error() == ErrorCode::InvalidArgument);
        CHECK(FieldDimensions::create(1, 4'097).error() == ErrorCode::InvalidArgument);
    }

    TEST_CASE("FieldDimensions rejects an over-limit total-cell count") {
        const auto dimensions = FieldDimensions::create(2'049, 2'049);

        REQUIRE_FALSE(dimensions);
        CHECK(dimensions.error() == ErrorCode::InvalidArgument);
    }

    TEST_CASE("FieldDimensions rejects values before multiplication can overflow") {
        const auto dimensions = FieldDimensions::create(std::numeric_limits<std::size_t>::max(),
                                                        std::numeric_limits<std::size_t>::max());

        REQUIRE_FALSE(dimensions);
        CHECK(dimensions.error() == ErrorCode::InvalidArgument);
    }

} // namespace
