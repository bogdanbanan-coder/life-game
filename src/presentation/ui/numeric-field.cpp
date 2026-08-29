#include <presentation/ui/numeric-field.hpp>

#include <utility>

#include <application/configuration/simulation-settings.hpp>
#include <domain/field/field-dimensions.hpp>

namespace lifeGame::presentation {

    NumericField::NumericField(NumericFieldKind kind) : kind_{kind} {}

    void NumericField::setText(std::string text) {
        field_.setText(std::move(text));
        static_cast<void>(validate());
    }

    const std::string& NumericField::text() const noexcept { return field_.text(); }

    NumericFieldKind NumericField::kind() const noexcept { return kind_; }

    auto NumericField::validate() -> foundation::Result<void, foundation::ErrorCode> {
        auto valid = false;
        foundation::ErrorCode error = foundation::ErrorCode::InvalidArgument;
        if (kind_ == NumericFieldKind::Dimension) {
            const auto result = application::SimulationSettings::parseDimension(text());
            valid = result.hasValue();
            if (valid) {
                const auto value = result.value();
                valid = value >= domain::FieldDimensions::MIN_SIDE &&
                        value <= domain::FieldDimensions::MAX_SIDE;
            } else {
                error = result.error();
            }
        } else {
            const auto result = application::SimulationSettings::parseGenerationInterval(text());
            valid = result.hasValue();
            if (!valid) {
                error = result.error();
            }
        }

        if (!valid) {
            field_.setValidationMessage("Enter a valid number.");
            return foundation::Result<void, foundation::ErrorCode>::failure(error);
        }

        field_.clearValidationMessage();
        return foundation::Result<void, foundation::ErrorCode>::success();
    }

    void NumericField::setValidationMessage(std::string message) {
        field_.setValidationMessage(std::move(message));
    }

    void NumericField::clearValidationMessage() { field_.clearValidationMessage(); }

    const std::string& NumericField::validationMessage() const noexcept {
        return field_.validationMessage();
    }

    bool NumericField::isValid() const noexcept { return field_.isValid(); }

    bool NumericField::focused() const noexcept { return field_.focused(); }

    bool NumericField::handleInput(Rectangle bounds, LogicalPoint pointer, bool pressed,
                                   const TextInput& input) {
        return field_.handleInput(bounds, pointer, pressed, input);
    }

    void NumericField::render(Rectangle bounds) const { field_.render(bounds); }

} // namespace lifeGame::presentation
