#include <presentation/screens/field-screen.hpp>

namespace lifeGame::presentation {

    void FieldScreen::render(const domain::Field& field, int viewportWidth,
                             int viewportHeight) const {
        fieldRenderer_.render(field, viewportWidth, viewportHeight);
        toolbar_.render(viewportWidth, viewportHeight);
    }

} // namespace lifeGame::presentation
