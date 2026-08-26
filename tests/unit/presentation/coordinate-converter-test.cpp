#include <catch2/catch_test_macros.hpp>
#include <limits>

#include <domain/field/field.hpp>
#include <presentation/camera/coordinate-converter.hpp>
#include <presentation/rendering/field-renderer.hpp>

namespace {

    using lifeGame::domain::Field;
    using lifeGame::presentation::CoordinateConverter;
    using lifeGame::presentation::LogicalPoint;

    TEST_CASE("Coordinate conversion honors logical cell boundaries") {
        const auto field = Field::create(50, 50);
        REQUIRE(field);

        const auto plan = lifeGame::presentation::FieldRenderer::calculateRenderPlan(
            field.value(), 1280, 720);
        const auto atOrigin = CoordinateConverter::toCell(
            field.value(), LogicalPoint{plan.fieldRectangle.x, plan.fieldRectangle.y}, 1280, 720);
        const auto atNextCell = CoordinateConverter::toCell(
            field.value(),
            LogicalPoint{plan.fieldRectangle.x + static_cast<float>(plan.cellSize),
                         plan.fieldRectangle.y + static_cast<float>(plan.cellSize)},
            1280, 720);
        const auto atFinalCell = CoordinateConverter::toCell(
            field.value(),
            LogicalPoint{plan.fieldRectangle.x + plan.fieldRectangle.width -
                             static_cast<float>(plan.cellSize) / 2.0F,
                         plan.fieldRectangle.y + plan.fieldRectangle.height -
                             static_cast<float>(plan.cellSize) / 2.0F},
            1280, 720);
        const auto atRightEdge = CoordinateConverter::toCell(
            field.value(),
            LogicalPoint{plan.fieldRectangle.x + plan.fieldRectangle.width,
                         plan.fieldRectangle.y + plan.fieldRectangle.height / 2.0F},
            1280, 720);
        const auto atBottomEdge = CoordinateConverter::toCell(
            field.value(),
            LogicalPoint{plan.fieldRectangle.x + plan.fieldRectangle.width / 2.0F,
                         plan.fieldRectangle.y + plan.fieldRectangle.height},
            1280, 720);

        REQUIRE(atOrigin);
        CHECK(atOrigin->x == 0);
        CHECK(atOrigin->y == 0);
        REQUIRE(atNextCell);
        CHECK(atNextCell->x == 1);
        CHECK(atNextCell->y == 1);
        REQUIRE(atFinalCell);
        CHECK(atFinalCell->x == field.value().width() - 1);
        CHECK(atFinalCell->y == field.value().height() - 1);
        CHECK_FALSE(atRightEdge);
        CHECK_FALSE(atBottomEdge);
    }

    TEST_CASE("Coordinate conversion rejects negative and out-of-field logical points") {
        const auto field = Field::create(50, 50);
        REQUIRE(field);

        const auto plan = lifeGame::presentation::FieldRenderer::calculateRenderPlan(
            field.value(), 1280, 720);
        CHECK_FALSE(CoordinateConverter::toCell(field.value(), LogicalPoint{-1.0F, 10.0F}, 1280,
                                                 720));
        CHECK_FALSE(CoordinateConverter::toCell(
            field.value(),
            LogicalPoint{plan.fieldRectangle.x - 1.0F,
                         plan.fieldRectangle.y + plan.fieldRectangle.height / 2.0F},
            1280, 720));
        CHECK_FALSE(CoordinateConverter::toCell(
            field.value(), LogicalPoint{std::numeric_limits<float>::quiet_NaN(), 10.0F}, 1280,
            720));
        CHECK_FALSE(CoordinateConverter::toCell(
            field.value(), LogicalPoint{10.0F, std::numeric_limits<float>::infinity()}, 1280,
            720));

        const auto logicalPoint = LogicalPoint{plan.fieldRectangle.x + 2.5F,
                                               plan.fieldRectangle.y + 3.5F};
        const auto atOneX = CoordinateConverter::toCell(field.value(), logicalPoint, 1280, 720);
        const auto atTwoX = CoordinateConverter::toCell(field.value(), logicalPoint, 1280, 720);

        REQUIRE(atOneX);
        REQUIRE(atTwoX);
        CHECK(atOneX->x == atTwoX->x);
        CHECK(atOneX->y == atTwoX->y);
    }

    TEST_CASE("Coordinate conversion rejects points outside the logical client viewport") {
        const auto field = Field::create(240, 180);
        REQUIRE(field);

        CHECK_FALSE(CoordinateConverter::toCell(field.value(), LogicalPoint{1280.0F, 10.0F},
                                                 1280, 720));
        CHECK_FALSE(CoordinateConverter::toCell(field.value(), LogicalPoint{10.0F, 720.0F},
                                                 1280, 720));
        CHECK_FALSE(CoordinateConverter::toCell(field.value(), LogicalPoint{0.0F, 0.0F}, 0,
                                                 720));
    }

} // namespace
