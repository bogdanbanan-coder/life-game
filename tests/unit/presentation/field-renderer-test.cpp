#include <catch2/catch_test_macros.hpp>
#include <domain/field/field.hpp>
#include <presentation/rendering/field-renderer.hpp>

namespace {

    using lifeGame::domain::Field;
    using lifeGame::presentation::FieldRenderer;

    TEST_CASE("Field render plan covers the 50 by 50 MVP surface") {
        const auto field = Field::create(50, 50);
        REQUIRE(field);

        const auto plan = FieldRenderer::calculateRenderPlan(field.value(), 1280, 720);

        CHECK(plan.fieldRectangle.x == 32.0F);
        CHECK(plan.fieldRectangle.y == 10.0F);
        CHECK(plan.fieldRectangle.width == 700.0F);
        CHECK(plan.fieldRectangle.height == 700.0F);
        CHECK(plan.cellSize == 14);
        CHECK(plan.showGrid);
        CHECK(plan.deadCell.r == 0);
        CHECK(plan.deadCell.g == 0);
        CHECK(plan.deadCell.b == 0);
        CHECK(plan.liveCell.r == 255);
        CHECK(plan.liveCell.g == 255);
        CHECK(plan.liveCell.b == 255);
        CHECK(plan.outOfField.r == 128);
        CHECK(plan.outOfField.g == 128);
        CHECK(plan.outOfField.b == 128);
    }

    TEST_CASE("Field render plan hides grid below the approved threshold") {
        const auto field = Field::create(240, 180);
        REQUIRE(field);

        const auto planAtFourPixels = FieldRenderer::calculateRenderPlan(field.value(), 960, 720);
        const auto planAtThreePixels = FieldRenderer::calculateRenderPlan(field.value(), 959, 719);

        CHECK(planAtFourPixels.cellSize == 4);
        CHECK(planAtFourPixels.showGrid);
        CHECK(planAtThreePixels.cellSize == 3);
        CHECK_FALSE(planAtThreePixels.showGrid);
    }

} // namespace
