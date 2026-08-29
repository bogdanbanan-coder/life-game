#include <presentation/ui/settings-panel.hpp>

#include <algorithm>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <string>
#include <utility>

#include <raygui.h>

namespace lifeGame::presentation {

    namespace {

        constexpr int PANEL_WIDTH = 560;
        constexpr int PANEL_HEIGHT = 306;
        constexpr int PANEL_MARGIN = 16;
        constexpr int LABEL_WIDTH = 244;
        constexpr int VALUE_WIDTH = 238;
        constexpr int ROW_HEIGHT = 56;
        constexpr int FIELD_HEIGHT = 36;
        constexpr int BUTTON_WIDTH = 128;

        [[nodiscard]] auto contains(Rectangle bounds, LogicalPoint point) noexcept -> bool {
            return point.x >= bounds.x && point.x < bounds.x + bounds.width &&
                   point.y >= bounds.y && point.y < bounds.y + bounds.height;
        }

        [[nodiscard]] auto formatInterval(application::SimulationSettings::Duration interval)
            -> std::string {
            std::ostringstream stream;
            stream.imbue(std::locale::classic());
            stream << std::setprecision(std::numeric_limits<long double>::max_digits10)
                   << std::fixed << interval.count();
            auto result = stream.str();
            while (!result.empty() && result.back() == '0') {
                result.pop_back();
            }
            if (!result.empty() && result.back() == '.') {
                result.pop_back();
            }
            return result;
        }

    } // namespace

    SettingsPanel::SettingsPanel(application::SettingsService& settingsService)
        : settingsService_{settingsService} {}

    auto SettingsPanel::calculateLayout(int viewportWidth, int viewportHeight) noexcept
        -> SettingsPanelLayout {
        const auto safeWidth = std::max(viewportWidth, 1);
        const auto safeHeight = std::max(viewportHeight, 1);
        const auto panelX = std::max((safeWidth - PANEL_WIDTH) / 2, PANEL_MARGIN);
        const auto panelY = std::max((safeHeight - PANEL_HEIGHT) / 2, PANEL_MARGIN);
        const Rectangle panel{static_cast<float>(panelX), static_cast<float>(panelY),
                              static_cast<float>(PANEL_WIDTH), static_cast<float>(PANEL_HEIGHT)};

        std::array<Rectangle, SettingsPanelLayout::ROW_COUNT> labels{};
        std::array<Rectangle, SettingsPanelLayout::ROW_COUNT> values{};
        std::array<Rectangle, SettingsPanelLayout::ROW_COUNT> validation{};
        const auto firstRowY = panelY + 36;
        for (std::size_t row = 0; row < SettingsPanelLayout::ROW_COUNT; ++row) {
            const auto y = firstRowY + static_cast<int>(row) * ROW_HEIGHT;
            labels[row] = Rectangle{static_cast<float>(panelX + 20), static_cast<float>(y),
                                    static_cast<float>(LABEL_WIDTH),
                                    static_cast<float>(FIELD_HEIGHT)};
            values[row] = Rectangle{static_cast<float>(panelX + 286), static_cast<float>(y),
                                    static_cast<float>(VALUE_WIDTH),
                                    static_cast<float>(FIELD_HEIGHT)};
            validation[row] = Rectangle{static_cast<float>(panelX + 286),
                                         static_cast<float>(y + FIELD_HEIGHT),
                                         static_cast<float>(VALUE_WIDTH), 16.0F};
        }

        const auto buttonY = panelY + PANEL_HEIGHT - 48;
        return SettingsPanelLayout{
            panel,
            labels,
            values,
            validation,
            Rectangle{static_cast<float>(panelX + PANEL_WIDTH - 2 * BUTTON_WIDTH - 28),
                      static_cast<float>(buttonY), static_cast<float>(BUTTON_WIDTH),
                      static_cast<float>(FIELD_HEIGHT)},
            Rectangle{static_cast<float>(panelX + PANEL_WIDTH - BUTTON_WIDTH - 16),
                      static_cast<float>(buttonY), static_cast<float>(BUTTON_WIDTH),
                      static_cast<float>(FIELD_HEIGHT)},
            Rectangle{static_cast<float>(panelX + 20), static_cast<float>(panelY + 12),
                      static_cast<float>(PANEL_WIDTH - 40), 20.0F}};
    }

    SettingsPanelRenderPlan SettingsPanel::calculateRenderPlan() const noexcept {
        return SettingsPanelRenderPlan{
            {"Field width (cells)", "Field height (cells)", "Generation interval (seconds)"},
            {width_.text(), height_.text(), interval_.text()},
            {width_.validationMessage(), height_.validationMessage(),
             interval_.validationMessage()},
            open_ && draftValid_,
        };
    }

    void SettingsPanel::open() {
        if (open_) {
            return;
        }

        const auto& settings = settingsService_.activeSettings();
        width_.setText(std::to_string(settings.width()));
        height_.setText(std::to_string(settings.height()));
        interval_.setText(formatInterval(settings.generationInterval()));
        lastError_.reset();
        status_.clear();
        open_ = true;
        refreshValidation();
    }

    void SettingsPanel::close() noexcept { open_ = false; }

    void SettingsPanel::cancel() noexcept {
        open_ = false;
        lastError_.reset();
        status_.clear();
    }

    bool SettingsPanel::isOpen() const noexcept { return open_; }

    void SettingsPanel::setWidthText(std::string text) {
        width_.setText(std::move(text));
        refreshValidation();
    }

    void SettingsPanel::setHeightText(std::string text) {
        height_.setText(std::move(text));
        refreshValidation();
    }

    void SettingsPanel::setIntervalText(std::string text) {
        interval_.setText(std::move(text));
        refreshValidation();
    }

    const std::string& SettingsPanel::widthText() const noexcept { return width_.text(); }

    const std::string& SettingsPanel::heightText() const noexcept { return height_.text(); }

    const std::string& SettingsPanel::intervalText() const noexcept { return interval_.text(); }

    const NumericField& SettingsPanel::widthField() const noexcept { return width_; }

    const NumericField& SettingsPanel::heightField() const noexcept { return height_; }

    const NumericField& SettingsPanel::intervalField() const noexcept { return interval_; }

    bool SettingsPanel::canSave() const noexcept { return open_ && draftValid_; }

    auto SettingsPanel::draftSettings()
        -> foundation::Result<application::SimulationSettings, foundation::ErrorCode> {
        refreshValidation();
        if (!draftValid_) {
            return foundation::Result<application::SimulationSettings,
                                      foundation::ErrorCode>::failure(
                foundation::ErrorCode::InvalidArgument);
        }

        return application::SimulationSettings::createFromText(width_.text(), height_.text(),
                                                                interval_.text());
    }

    const application::SimulationSettings& SettingsPanel::activeSettings() const noexcept {
        return settingsService_.activeSettings();
    }

    const std::optional<foundation::ErrorCode>& SettingsPanel::lastError() const noexcept {
        return lastError_;
    }

    auto SettingsPanel::save() -> foundation::Result<void, foundation::ErrorCode> {
        if (!open_) {
            lastError_ = foundation::ErrorCode::InvalidState;
            return foundation::Result<void, foundation::ErrorCode>::failure(*lastError_);
        }

        const auto candidate = draftSettings();
        if (!candidate) {
            lastError_ = candidate.error();
            return foundation::Result<void, foundation::ErrorCode>::failure(candidate.error());
        }

        const auto result = settingsService_.save(candidate.value());
        if (!result) {
            lastError_ = result.error();
            status_.showError("Save failed. Retry or cancel.");
            return result;
        }

        lastError_.reset();
        status_.showSuccess("Settings saved.");
        open_ = false;
        return result;
    }

    SettingsPanelAction SettingsPanel::handleInput(int viewportWidth, int viewportHeight,
                                                    PointerSample pointer,
                                                    const TextInput& textInput) {
        if (!open_) {
            return SettingsPanelAction::None;
        }

        const auto layout = calculateLayout(viewportWidth, viewportHeight);
        static_cast<void>(width_.handleInput(layout.values[0], pointer.position, pointer.pressed,
                                              textInput));
        static_cast<void>(height_.handleInput(layout.values[1], pointer.position,
                                               pointer.pressed, textInput));
        static_cast<void>(interval_.handleInput(layout.values[2], pointer.position,
                                                pointer.pressed, textInput));
        // GuiTextBox may update its buffer during the preceding render pass.
        // Revalidate before processing this frame's buttons.
        refreshValidation();

        if (!pointer.pressed || !pointer.down) {
            return SettingsPanelAction::None;
        }
        if (contains(layout.cancel, pointer.position)) {
            cancel();
            return SettingsPanelAction::Cancelled;
        }
        if (contains(layout.save, pointer.position) && canSave()) {
            const auto result = save();
            return result ? SettingsPanelAction::Saved
                          : (isSaveFailure() ? SettingsPanelAction::SaveFailed
                                             : SettingsPanelAction::None);
        }

        return SettingsPanelAction::None;
    }

    void SettingsPanel::render(int viewportWidth, int viewportHeight) const {
        if (!open_) {
            return;
        }

        const auto layout = calculateLayout(viewportWidth, viewportHeight);
        GuiPanel(layout.panel, nullptr);
        GuiLabel(Rectangle{layout.panel.x + 20.0F, layout.panel.y + 10.0F, 320.0F, 22.0F},
                 "Global Settings");

        const auto renderPlan = calculateRenderPlan();
        for (std::size_t row = 0; row < SettingsPanelLayout::ROW_COUNT; ++row) {
            GuiLabel(layout.labels[row], renderPlan.labels[row].data());
        }
        width_.render(layout.values[0]);
        height_.render(layout.values[1]);
        interval_.render(layout.values[2]);
        for (std::size_t row = 0; row < SettingsPanelLayout::ROW_COUNT; ++row) {
            if (!renderPlan.validation[row].empty()) {
                GuiLabel(layout.validation[row], renderPlan.validation[row].data());
            }
        }

        if (canSave()) {
            GuiButton(layout.save, "Save");
        } else {
            GuiDisable();
            GuiButton(layout.save, "Save");
            GuiEnable();
        }
        GuiButton(layout.cancel, "Cancel");
        status_.render(layout.status);
    }

    void SettingsPanel::refreshValidation() {
        const auto width = application::SimulationSettings::parseDimension(width_.text());
        const auto height = application::SimulationSettings::parseDimension(height_.text());
        const auto interval =
            application::SimulationSettings::parseGenerationInterval(interval_.text());

        width_.clearValidationMessage();
        height_.clearValidationMessage();
        interval_.clearValidationMessage();
        draftValid_ = false;

        const auto validWidth =
            width && width.value() >= domain::FieldDimensions::MIN_SIDE &&
            width.value() <= domain::FieldDimensions::MAX_SIDE;
        const auto validHeight =
            height && height.value() >= domain::FieldDimensions::MIN_SIDE &&
            height.value() <= domain::FieldDimensions::MAX_SIDE;

        if (!validWidth) {
            width_.setValidationMessage("Enter a whole number from 1 to 4096.");
        }
        if (!validHeight) {
            height_.setValidationMessage("Enter a whole number from 1 to 4096.");
        }
        if (!interval) {
            interval_.setValidationMessage("Enter positive seconds using '.' as decimal.");
        }
        if (!validWidth || !validHeight || !interval) {
            return;
        }

        const auto dimensions = domain::FieldDimensions::create(width.value(), height.value());
        if (!dimensions) {
            width_.setValidationMessage("Dimensions exceed the supported field limits.");
            height_.setValidationMessage("Dimensions exceed the supported field limits.");
            return;
        }

        draftValid_ = true;
    }

    bool SettingsPanel::isSaveFailure() const noexcept {
        return lastError_.has_value() && draftValid_;
    }

} // namespace lifeGame::presentation
