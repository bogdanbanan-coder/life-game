#include <presentation/screens/field-screen.hpp>

namespace lifeGame::presentation {

    void FieldScreen::render(const domain::Field& field, int viewportWidth, int viewportHeight,
                             application::PaintMode paintMode,
                             application::RunState runState) const {
        fieldRenderer_.render(field, viewportWidth, viewportHeight);
        toolbar_.render(viewportWidth, viewportHeight, paintMode, runState);
    }

} // namespace lifeGame::presentation
