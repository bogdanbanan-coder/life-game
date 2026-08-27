#include <presentation/screens/field-screen.hpp>

namespace lifeGame::presentation {

    void FieldScreen::render(const domain::Field& field, int viewportWidth, int viewportHeight,
                             application::PaintMode paintMode) const {
        fieldRenderer_.render(field, viewportWidth, viewportHeight);
        toolbar_.render(viewportWidth, viewportHeight, paintMode);
    }

} // namespace lifeGame::presentation
