#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

#include <application/configuration/settings-service.hpp>
#include <foundation/error-code.hpp>
#include <foundation/result.hpp>
#include <presentation/camera/coordinate-converter.hpp>
#include <presentation/input/input-router.hpp>
#include <presentation/ui/numeric-field.hpp>
#include <presentation/ui/status-message.hpp>
#include <raylib.h>

namespace lifeGame::presentation {

    struct SettingsPanelLayout {
        static constexpr std::size_t ROW_COUNT = 3;

        Rectangle panel;
        std::array<Rectangle, ROW_COUNT> labels;
        std::array<Rectangle, ROW_COUNT> values;
        std::array<Rectangle, ROW_COUNT> validation;
        Rectangle save;
        Rectangle cancel;
        Rectangle status;
    };

    enum class SettingsPanelAction {
        None,
        Saved,
        Cancelled,
        SaveFailed,
    };

    struct SettingsPanelRenderPlan {
        std::array<std::string_view, SettingsPanelLayout::ROW_COUNT> labels;
        std::array<std::string_view, SettingsPanelLayout::ROW_COUNT> values;
        std::array<std::string_view, SettingsPanelLayout::ROW_COUNT> validation;
        bool saveEnabled;
    };

    class SettingsPanel {
      public:
        explicit SettingsPanel(application::SettingsService& settingsService);

        [[nodiscard]] static auto calculateLayout(int viewportWidth, int viewportHeight) noexcept
            -> SettingsPanelLayout;
        [[nodiscard]] SettingsPanelRenderPlan calculateRenderPlan() const noexcept;

        void open();
        void close() noexcept;
        void cancel() noexcept;
        [[nodiscard]] bool isOpen() const noexcept;

        void setWidthText(std::string text);
        void setHeightText(std::string text);
        void setIntervalText(std::string text);
        [[nodiscard]] const std::string& widthText() const noexcept;
        [[nodiscard]] const std::string& heightText() const noexcept;
        [[nodiscard]] const std::string& intervalText() const noexcept;

        [[nodiscard]] const NumericField& widthField() const noexcept;
        [[nodiscard]] const NumericField& heightField() const noexcept;
        [[nodiscard]] const NumericField& intervalField() const noexcept;
        [[nodiscard]] bool canSave() const noexcept;
        [[nodiscard]] auto draftSettings()
            -> foundation::Result<application::SimulationSettings,
                                   foundation::ErrorCode>;
        [[nodiscard]] const application::SimulationSettings& activeSettings() const noexcept;
        [[nodiscard]] const std::optional<foundation::ErrorCode>& lastError() const noexcept;

        [[nodiscard]] auto save()
            -> foundation::Result<void, foundation::ErrorCode>;
        [[nodiscard]] SettingsPanelAction handleInput(int viewportWidth, int viewportHeight,
                                                       PointerSample pointer,
                                                       const TextInput& textInput);

        void render(int viewportWidth, int viewportHeight) const;

      private:
        void refreshValidation();
        [[nodiscard]] bool isSaveFailure() const noexcept;

        application::SettingsService& settingsService_;
        NumericField width_{NumericFieldKind::Dimension};
        NumericField height_{NumericFieldKind::Dimension};
        NumericField interval_{NumericFieldKind::GenerationInterval};
        StatusMessage status_;
        std::optional<foundation::ErrorCode> lastError_;
        bool open_ = false;
        bool draftValid_ = false;
    };

} // namespace lifeGame::presentation
