#include <presentation/ui/name-dialog.hpp>

#include <algorithm>
#include <utility>

#include <domain/session/session-name.hpp>
#include <raygui.h>

namespace lifeGame::presentation {

    namespace {

        constexpr int PANEL_WIDTH = 500;
        constexpr int PANEL_HEIGHT = 190;
        constexpr int FIELD_HEIGHT = 36;
        constexpr int BUTTON_WIDTH = 128;

        [[nodiscard]] auto contains(Rectangle bounds, LogicalPoint point) noexcept -> bool {
            return point.x >= bounds.x && point.x < bounds.x + bounds.width &&
                   point.y >= bounds.y && point.y < bounds.y + bounds.height;
        }

    } // namespace

    NameDialog::NameDialog(application::SessionService& sessionService)
        : sessionService_{sessionService}, field_{4096} {}

    auto NameDialog::calculateLayout(int viewportWidth, int viewportHeight) noexcept
        -> NameDialogLayout {
        const auto safeWidth = std::max(viewportWidth, 1);
        const auto safeHeight = std::max(viewportHeight, 1);
        const auto panelX = std::max((safeWidth - PANEL_WIDTH) / 2, 16);
        const auto panelY = std::max((safeHeight - PANEL_HEIGHT) / 2, 16);
        const Rectangle panel{static_cast<float>(panelX), static_cast<float>(panelY),
                              static_cast<float>(PANEL_WIDTH), static_cast<float>(PANEL_HEIGHT)};
        return NameDialogLayout{
            panel,
            Rectangle{static_cast<float>(panelX + 20), static_cast<float>(panelY + 42),
                      static_cast<float>(PANEL_WIDTH - 40), static_cast<float>(FIELD_HEIGHT)},
            Rectangle{static_cast<float>(panelX + 20), static_cast<float>(panelY + 80),
                      static_cast<float>(PANEL_WIDTH - 40), 24.0F},
            Rectangle{static_cast<float>(panelX + PANEL_WIDTH - 2 * BUTTON_WIDTH - 28),
                      static_cast<float>(panelY + PANEL_HEIGHT - FIELD_HEIGHT - 14),
                      static_cast<float>(BUTTON_WIDTH), static_cast<float>(FIELD_HEIGHT)},
            Rectangle{static_cast<float>(panelX + PANEL_WIDTH - BUTTON_WIDTH - 20),
                      static_cast<float>(panelY + PANEL_HEIGHT - FIELD_HEIGHT - 14),
                      static_cast<float>(BUTTON_WIDTH), static_cast<float>(FIELD_HEIGHT)}};
    }

    NameDialogRenderPlan NameDialog::calculateRenderPlan() const noexcept {
        return NameDialogRenderPlan{field_.text(), field_.validationMessage(), valid_};
    }

    void NameDialog::open(std::string initialText) {
        field_.setText(std::move(initialText));
        field_.clearValidationMessage();
        lastError_.reset();
        createdSession_.reset();
        open_ = true;
        refreshValidation();
    }

    void NameDialog::close() noexcept { open_ = false; }

    void NameDialog::cancel() noexcept {
        open_ = false;
        lastError_.reset();
    }

    bool NameDialog::isOpen() const noexcept { return open_; }

    void NameDialog::setText(std::string text) {
        field_.setText(std::move(text));
        refreshValidation();
    }

    const std::string& NameDialog::text() const noexcept { return field_.text(); }

    const TextField& NameDialog::textField() const noexcept { return field_; }

    const std::string& NameDialog::validationMessage() const noexcept {
        return field_.validationMessage();
    }

    bool NameDialog::canSave() const noexcept { return open_ && valid_; }

    const std::optional<foundation::ErrorCode>& NameDialog::lastError() const noexcept {
        return lastError_;
    }

    auto NameDialog::commit()
        -> foundation::Result<domain::SessionId, foundation::ErrorCode> {
        if (!open_) {
            lastError_ = foundation::ErrorCode::InvalidState;
            return foundation::Result<domain::SessionId, foundation::ErrorCode>::failure(
                *lastError_);
        }

        refreshValidation();
        if (!valid_) {
            lastError_ = foundation::ErrorCode::InvalidArgument;
            return foundation::Result<domain::SessionId, foundation::ErrorCode>::failure(
                *lastError_);
        }

        const auto created = sessionService_.create(text());
        if (!created) {
            lastError_ = created.error();
            return created;
        }

        lastError_.reset();
        open_ = false;
        createdSession_ = created.value();
        return created;
    }

    std::optional<domain::SessionId> NameDialog::takeCreatedSession() noexcept {
        const auto created = createdSession_;
        createdSession_.reset();
        return created;
    }

    NameDialogAction NameDialog::handleInput(int viewportWidth, int viewportHeight,
                                             PointerSample pointer,
                                             const TextInput& textInput) {
        if (!open_) {
            return NameDialogAction::None;
        }

        const auto layout = calculateLayout(viewportWidth, viewportHeight);
        static_cast<void>(field_.handleInput(layout.value, pointer.position, pointer.pressed,
                                              textInput));
        // GuiTextBox may update its buffer during the preceding render pass.
        // Revalidate before processing this frame's buttons.
        refreshValidation();

        if (textInput.escape) {
            cancel();
            return NameDialogAction::Cancelled;
        }
        if (textInput.enter && field_.focused()) {
            const auto result = commit();
            return result ? NameDialogAction::Created : NameDialogAction::Rejected;
        }
        if (!pointer.pressed || !pointer.down) {
            return NameDialogAction::None;
        }
        if (contains(layout.cancel, pointer.position)) {
            cancel();
            return NameDialogAction::Cancelled;
        }
        if (contains(layout.save, pointer.position)) {
            const auto result = commit();
            return result ? NameDialogAction::Created : NameDialogAction::Rejected;
        }
        return NameDialogAction::None;
    }

    void NameDialog::render(int viewportWidth, int viewportHeight) const {
        if (!open_) {
            return;
        }
        const auto layout = calculateLayout(viewportWidth, viewportHeight);
        GuiPanel(layout.panel, nullptr);
        GuiLabel(Rectangle{layout.panel.x + 20.0F, layout.panel.y + 12.0F, 280.0F, 24.0F},
                 "Create session");
        field_.render(layout.value);
        if (!field_.validationMessage().empty()) {
            GuiLabel(layout.validation, field_.validationMessage().c_str());
        }
        if (canSave()) {
            GuiButton(layout.save, "Create");
        } else {
            GuiDisable();
            GuiButton(layout.save, "Create");
            GuiEnable();
        }
        GuiButton(layout.cancel, "Cancel");
    }

    void NameDialog::refreshValidation() {
        field_.clearValidationMessage();
        valid_ = false;

        const auto name = domain::SessionName::create(field_.text());
        if (!name) {
            field_.setValidationMessage("Use 1–64 Unicode characters.");
            return;
        }
        if (sessionService_.containsName(name.value().displayName())) {
            field_.setValidationMessage("A session with this name already exists.");
            return;
        }
        valid_ = true;
    }

} // namespace lifeGame::presentation
