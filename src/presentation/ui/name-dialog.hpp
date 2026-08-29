#pragma once

#include <optional>
#include <string>
#include <string_view>

#include <application/session/session-service.hpp>
#include <foundation/error-code.hpp>
#include <foundation/result.hpp>
#include <presentation/camera/coordinate-converter.hpp>
#include <presentation/input/input-router.hpp>
#include <presentation/ui/text-field.hpp>
#include <raylib.h>

namespace lifeGame::presentation {

    struct NameDialogLayout {
        Rectangle panel;
        Rectangle value;
        Rectangle validation;
        Rectangle save;
        Rectangle cancel;
    };

    enum class NameDialogAction {
        None,
        Created,
        Cancelled,
        Rejected,
    };

    struct NameDialogRenderPlan {
        std::string_view value;
        std::string_view validation;
        bool saveEnabled;
    };

    class NameDialog {
      public:
        explicit NameDialog(application::SessionService& sessionService);

        [[nodiscard]] static auto calculateLayout(int viewportWidth, int viewportHeight) noexcept
            -> NameDialogLayout;
        [[nodiscard]] NameDialogRenderPlan calculateRenderPlan() const noexcept;

        void open(std::string initialText = {});
        void close() noexcept;
        void cancel() noexcept;
        [[nodiscard]] bool isOpen() const noexcept;

        void setText(std::string text);
        [[nodiscard]] const std::string& text() const noexcept;
        [[nodiscard]] const TextField& textField() const noexcept;
        [[nodiscard]] const std::string& validationMessage() const noexcept;
        [[nodiscard]] bool canSave() const noexcept;
        [[nodiscard]] const std::optional<foundation::ErrorCode>& lastError() const noexcept;

        [[nodiscard]] auto commit()
            -> foundation::Result<domain::SessionId, foundation::ErrorCode>;
        [[nodiscard]] std::optional<domain::SessionId> takeCreatedSession() noexcept;
        [[nodiscard]] NameDialogAction handleInput(int viewportWidth, int viewportHeight,
                                                   PointerSample pointer,
                                                   const TextInput& textInput);

        void render(int viewportWidth, int viewportHeight) const;

      private:
        void refreshValidation();

        application::SessionService& sessionService_;
        TextField field_;
        std::optional<foundation::ErrorCode> lastError_;
        std::optional<domain::SessionId> createdSession_;
        bool open_ = false;
        bool valid_ = false;
    };

} // namespace lifeGame::presentation
