#pragma once

#include <cstddef>
#include <string>
#include <string_view>

#include <foundation/error-code.hpp>
#include <foundation/result.hpp>
#include <presentation/ui/text-field.hpp>

namespace lifeGame::presentation {

    enum class NumericFieldKind {
        Dimension,
        GenerationInterval,
    };

    class NumericField {
      public:
        explicit NumericField(NumericFieldKind kind);

        void setText(std::string text);
        [[nodiscard]] const std::string& text() const noexcept;
        [[nodiscard]] NumericFieldKind kind() const noexcept;

        [[nodiscard]] auto validate()
            -> foundation::Result<void, foundation::ErrorCode>;
        void setValidationMessage(std::string message);
        void clearValidationMessage();
        [[nodiscard]] const std::string& validationMessage() const noexcept;
        [[nodiscard]] bool isValid() const noexcept;
        [[nodiscard]] bool focused() const noexcept;

        [[nodiscard]] bool handleInput(Rectangle bounds, LogicalPoint pointer, bool pressed,
                                       const TextInput& input);
        void render(Rectangle bounds) const;

      private:
        NumericFieldKind kind_;
        TextField field_;
    };

} // namespace lifeGame::presentation
