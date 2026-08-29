#pragma once

#include <cstddef>
#include <string>
#include <string_view>

#include <presentation/camera/coordinate-converter.hpp>
#include <raylib.h>

namespace lifeGame::presentation {

    struct TextInput {
        std::string characters;
        bool backspace = false;
        bool enter = false;
        bool escape = false;
    };

    class TextField {
      public:
        explicit TextField(std::size_t maximumBytes = 256);

        void setText(std::string text);
        [[nodiscard]] const std::string& text() const noexcept;

        void setValidationMessage(std::string message);
        void clearValidationMessage();
        [[nodiscard]] const std::string& validationMessage() const noexcept;
        [[nodiscard]] bool isValid() const noexcept;
        [[nodiscard]] bool focused() const noexcept;

        [[nodiscard]] bool handleInput(Rectangle bounds, LogicalPoint pointer, bool pressed,
                                       const TextInput& input);
        void render(Rectangle bounds) const;

      private:
        std::size_t maximumBytes_;
        mutable std::string text_;
        std::string validationMessage_;
        mutable bool focused_ = false;
        mutable bool suppressGuiEditing_ = false;
    };

} // namespace lifeGame::presentation
