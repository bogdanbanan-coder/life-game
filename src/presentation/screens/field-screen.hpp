#pragma once

#include <application/commands/field-command.hpp>
#include <application/commands/run-command.hpp>
#include <domain/field/field.hpp>
#include <presentation/rendering/field-renderer.hpp>
#include <presentation/ui/toolbar.hpp>

namespace lifeGame::presentation {

    class FieldScreen {
      public:
        void render(const domain::Field& field, int viewportWidth, int viewportHeight,
                    application::PaintMode paintMode,
                    application::RunState runState) const;

      private:
        FieldRenderer fieldRenderer_;
        Toolbar toolbar_;
    };

} // namespace lifeGame::presentation
