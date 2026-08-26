#include <catch2/catch_test_macros.hpp>
#include <vector>

#include <domain/field/field.hpp>
#include <presentation/rendering/field-renderer.hpp>

namespace {

    struct DrawCall {
        int x;
        int y;
        int width;
        int height;
        Color color;
    };

    std::vector<DrawCall> drawCalls;

} // namespace

extern "C" void DrawRectangle(int posX, int posY, int width, int height, Color color) {
    drawCalls.push_back(DrawCall{posX, posY, width, height, color});
}

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

    TEST_CASE("Field render plan leaves finite-field surroundings as gray space") {
        const auto field = Field::create(50, 50);
        REQUIRE(field);

        const auto plan = FieldRenderer::calculateRenderPlan(field.value(), 1280, 720);
        const auto right = plan.fieldRectangle.x + plan.fieldRectangle.width;
        const auto bottom = plan.fieldRectangle.y + plan.fieldRectangle.height;

        CHECK(plan.outOfField.r == 128);
        CHECK(plan.outOfField.g == 128);
        CHECK(plan.outOfField.b == 128);
        CHECK(plan.fieldRectangle.x > 0.0F);
        CHECK(plan.fieldRectangle.y > 0.0F);
        CHECK(right < 1280.0F);
        CHECK(bottom < 720.0F);
    }

    TEST_CASE("Field renderer fills the viewport with gray outside the finite field") {
        auto field = Field::create(2, 2);
        REQUIRE(field);
        REQUIRE(field.value().setLive({1, 0}, true));

        drawCalls.clear();
        const auto plan = FieldRenderer::calculateRenderPlan(field.value(), 1280, 720);
        FieldRenderer{}.render(field.value(), 1280, 720);

        REQUIRE_FALSE(drawCalls.empty());
        const auto& background = drawCalls.front();
        CHECK(background.x == 0);
        CHECK(background.y == 0);
        CHECK(background.width == 1280);
        CHECK(background.height == 720);
        CHECK(background.color.r == plan.outOfField.r);
        CHECK(background.color.g == plan.outOfField.g);
        CHECK(background.color.b == plan.outOfField.b);
        CHECK(background.color.a == 255);

        REQUIRE(drawCalls.size() >= 5);
        const auto originX = static_cast<int>(plan.fieldRectangle.x);
        const auto originY = static_cast<int>(plan.fieldRectangle.y);
        for (std::size_t index = 0; index < 4; ++index) {
            const auto x = static_cast<int>(index % 2);
            const auto y = static_cast<int>(index / 2);
            const auto& cellDraw = drawCalls[index + 1];
            const auto& expectedColor = index == 1 ? plan.liveCell : plan.deadCell;
            CHECK(cellDraw.x == originX + x * plan.cellSize);
            CHECK(cellDraw.y == originY + y * plan.cellSize);
            CHECK(cellDraw.width == plan.cellSize);
            CHECK(cellDraw.height == plan.cellSize);
            CHECK(cellDraw.color.r == expectedColor.r);
            CHECK(cellDraw.color.g == expectedColor.g);
            CHECK(cellDraw.color.b == expectedColor.b);
            CHECK(cellDraw.color.a == expectedColor.a);
        }

        const auto fieldRight = static_cast<int>(plan.fieldRectangle.x) +
                                static_cast<int>(plan.fieldRectangle.width);
        const auto fieldBottom = static_cast<int>(plan.fieldRectangle.y) +
                                 static_cast<int>(plan.fieldRectangle.height);
        for (std::size_t index = 1; index < drawCalls.size(); ++index) {
            const auto& drawCall = drawCalls[index];
            CHECK(drawCall.x >= static_cast<int>(plan.fieldRectangle.x));
            CHECK(drawCall.y >= static_cast<int>(plan.fieldRectangle.y));
            CHECK(drawCall.x + drawCall.width <= fieldRight);
            CHECK(drawCall.y + drawCall.height <= fieldBottom);
        }
    }

} // namespace
