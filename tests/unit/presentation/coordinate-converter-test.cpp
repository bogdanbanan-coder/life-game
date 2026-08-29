#include <catch2/catch_test_macros.hpp>
#include <limits>

#include <domain/field/field.hpp>
#include <presentation/camera/coordinate-converter.hpp>
#include <presentation/rendering/field-renderer.hpp>

namespace {

    using lifeGame::domain::Field;
    using lifeGame::presentation::CameraState;
    using lifeGame::presentation::CoordinateConverter;
    using lifeGame::presentation::FieldRenderer;
    using lifeGame::presentation::LogicalPoint;
    using lifeGame::presentation::ZoomLevel;

    TEST_CASE("Coordinate conversion honors logical cell boundaries") {
        const auto field = Field::create(50, 50);
        REQUIRE(field);

        const auto plan = FieldRenderer::calculateRenderPlan(field.value(), 1280, 720);
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
            field.value(),
            LogicalPoint{plan.fieldRectangle.x + plan.fieldRectangle.width / 2.0F,
                         plan.fieldRectangle.y - 1.0F},
            1280, 720));
        CHECK_FALSE(CoordinateConverter::toCell(
            field.value(),
            LogicalPoint{plan.fieldRectangle.x + plan.fieldRectangle.width + 1.0F,
                         plan.fieldRectangle.y + plan.fieldRectangle.height / 2.0F},
            1280, 720));
        CHECK_FALSE(CoordinateConverter::toCell(
            field.value(),
            LogicalPoint{plan.fieldRectangle.x + plan.fieldRectangle.width / 2.0F,
                         plan.fieldRectangle.y + plan.fieldRectangle.height + 1.0F},
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

    TEST_CASE("Coordinate conversion preserves half-open boundaries at fractional zoom") {
        const auto field = Field::create(50, 50);
        REQUIRE(field);
        const auto camera = CameraState{0.0F, 0.0F, ZoomLevel::Percent75};
        const auto plan = FieldRenderer::calculateRenderPlan(field.value(), 1280, 720, camera);
        REQUIRE(plan.cellSize == 10.5F);

        const auto firstCell = CoordinateConverter::toCell(
            field.value(), LogicalPoint{plan.fieldRectangle.x + 10.5F,
                                        plan.fieldRectangle.y + 10.5F},
            1280, 720, camera);
        const auto secondCell = CoordinateConverter::toCell(
            field.value(), LogicalPoint{plan.fieldRectangle.x + 21.0F,
                                        plan.fieldRectangle.y + 21.0F},
            1280, 720, camera);
        const auto atSnappedBoundary = CoordinateConverter::toCell(
            field.value(), LogicalPoint{plan.fieldRectangle.x + 10.0F,
                                        plan.fieldRectangle.y + 5.25F},
            1280, 720, camera);
        const auto atRightEdge = CoordinateConverter::toCell(
            field.value(),
            LogicalPoint{plan.fieldRectangle.x + plan.fieldRectangle.width,
                         plan.fieldRectangle.y + plan.fieldRectangle.height / 2.0F},
            1280, 720, camera);

        REQUIRE(firstCell);
        CHECK(firstCell->x == 1);
        CHECK(firstCell->y == 1);
        REQUIRE(secondCell);
        CHECK(secondCell->x == 2);
        CHECK(secondCell->y == 2);
        REQUIRE(atSnappedBoundary);
        CHECK(atSnappedBoundary->x == 1);
        CHECK(atSnappedBoundary->y == 0);
        CHECK_FALSE(atRightEdge);
    }

} // namespace
