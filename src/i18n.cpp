#include "i18n.h"
#include <QMap>

I18n* I18n::instance() {
    static I18n s_instance;
    return &s_instance;
}

I18n::I18n(QObject *parent) : QObject(parent) {
    loadSettings();
}

void I18n::loadSettings() {
    QSettings settings("DisplayTools", "AdjustDisplay");
    if (settings.contains("language")) {
        QString langStr = settings.value("language").toString();
        if (langStr == "ja") {
            m_language = Language::Japanese;
        } else {
            m_language = Language::English;
        }
    } else {
        // Auto-detect based on system locale
        QLocale sysLocale = QLocale::system();
        if (sysLocale.language() == QLocale::Japanese) {
            m_language = Language::Japanese;
        } else {
            m_language = Language::English;
        }
    }
}

void I18n::saveSettings() {
    QSettings settings("DisplayTools", "AdjustDisplay");
    settings.setValue("language", m_language == Language::Japanese ? "ja" : "en");
}

void I18n::setLanguage(Language lang) {
    if (m_language != lang) {
        m_language = lang;
        saveSettings();
        emit languageChanged(m_language);
    }
}

static const QMap<QString, QString> s_translationsJa = {
    // App
    {"app_title", "AdjustDisplay - Ubuntu ディスプレイ調整ツール"},
    {"app_display_name", "Ubuntu ディスプレイ調整ツール"},

    // Header
    {"screen_primary_tag", "(プライマリ)"},
    {"btn_identify", "🎯 画面識別"},
    {"btn_identify_tip", "選択中の画面に大きな識別番号を表示します"},
    {"btn_identify_all", "✨ 全画面識別"},
    {"btn_identify_all_tip", "すべての画面に識別番号を同時に表示します"},
    {"btn_refresh", "🔄 再検出"},
    {"lang_label", "🌐 言語:"},

    // Tabs
    {"tab_wizard", "🧙‍♂️ ガイド付きウィザード"},
    {"tab_explorer", "🎨 パターン一覧・個別検査"},
    {"tab_hardware", "⚙️ ディスプレイ & DDC/CI 設定"},

    // Wizard Tab
    {"wizard_sidebar_title", "📋 調整ステップ"},
    {"btn_wizard_prev", "◀ 前のステップ"},
    {"btn_wizard_next", "次のステップ ▶"},
    {"btn_wizard_finish", "完了 🎉"},
    {"btn_wizard_fullscreen", "🖥️ 全画面で調整 (推奨)"},

    // Wizard Steps
    {"step1_short", "1. 黒レベル"},
    {"step1_title", "黒レベル / 輝度 (Brightness) の調整"},
    {"step1_subtitle", "暗部階調の基準を合わせます"},
    {"step1_inst", "0%（純黒）が背景に溶け込み、1%〜2%のバーがギリギリ判別できるレベルまでモニタの輝度（Brightness）を下げてください。黒浮きを防ぎつつシャドウの階調を保持します。"},
    {"step1_osd", "モニタOSDの「輝度 (Brightness)」を調整。暗い部屋での調整を推奨。"},

    {"step2_short", "2. 白レベル"},
    {"step2_title", "白レベル / コントラスト (Contrast) の調整"},
    {"step2_subtitle", "明部階調の白飛びを防ぎます"},
    {"step2_inst", "100%（純白）と99%〜99.6%（RGB 254）の境界がはっきりと識別できる限界までコントラスト（Contrast）を上げてください。上げすぎると白飛びが発生します。"},
    {"step2_osd", "モニタOSDの「コントラスト (Contrast)」を調整。RGB各色で白飛びがないかも確認。"},

    {"step3_short", "3. ガンマ 2.2"},
    {"step3_title", "ガンマ 2.2 (Gamma 2.2) の調整"},
    {"step3_subtitle", "標準sRGBトーンカーブに合わせます"},
    {"step3_inst", "画面から少し離れて目を細めて見たとき、中央の「γ 2.2」パッチが白黒ストライプ背景と同化して見える状態が適正値です。"},
    {"step3_osd", "モニタOSDの「ガンマ (Gamma)」設定で 2.2 を選択、またはカラーモードを変更。"},

    {"step4_short", "4. グレースケール"},
    {"step4_title", "グレースケール & カラーバランスの確認"},
    {"step4_subtitle", "中立な無彩色（ニュートラルグレー）を確認します"},
    {"step4_inst", "32段階のステップバーおよび連続グラデーションにおいて、中間に緑や赤、青などの色被り・バンディング（階調段差）がないか確認してください。"},
    {"step4_osd", "特定の色が強い場合はモニタOSDの「色温度 (Color Temp)」やRGBゲインを微調整。"},

    {"step5_short", "5. シャープネス"},
    {"step5_title", "シャープネス & フォーカスの調整"},
    {"step5_subtitle", "文字の視認性とエッジの輪郭補正を適正化します"},
    {"step5_inst", "文字や1px格子の輪郭に不自然な白フチ（ハロー/リンギング）やモアレが発生していないか確認してください。高すぎると輪郭が強調されすぎます。"},
    {"step5_osd", "モニタOSDの「シャープネス (Sharpness)」を50%またはオフ/標準に設定。"},

    {"step6_short", "6. 色均一性"},
    {"step6_title", "色均一性 & ドット抜け検査"},
    {"step6_subtitle", "パネル全体の色ムラや輝度ムラ、ドット抜けを検査します"},
    {"step6_inst", "全画面表示の白・灰・赤・緑・青で四隅の暗がり（周辺減光）や色ムラ、常時点灯・消灯ドットをチェックします。[C] キーで背景色を切り替えられます。"},
    {"step6_osd", "画面全体の明るさや色温度の均一性を目視でチェック。"},

    {"step7_short", "7. 画面比率"},
    {"step7_title", "画面比率 & オーバースキャン検査"},
    {"step7_subtitle", "1:1 ドット・バイ・ドット表示を確認します"},
    {"step7_inst", "最外周の白枠（1ピクセル）が隠れずに四辺すべて見えているか、円が真円で描かれているかを確認してください。"},
    {"step7_osd", "モニタOSDの「アスペクト比」「オーバースキャン」「画面サイズ」を「Dot by Dot」や「フル」に設定。"},

    // Explorer Cards
    {"card_black_title", "1. 黒レベル / 輝度 (Brightness)"},
    {"card_black_desc", "0%〜5%低輝度ステップと点滅ボックスによる暗部階調の基準調整"},
    {"card_white_title", "2. 白レベル / コントラスト (Contrast)"},
    {"card_white_desc", "95%〜100%ハイライト階調とRGB階調による白飛び・クリッピング防止"},
    {"card_gamma_title", "3. ガンマ 2.2 (Gamma 2.2)"},
    {"card_gamma_desc", "1px白黒ラスタラインと基準パッチ比較によるガンマ曲線調整"},
    {"card_gray_title", "4. グレースケール & カラーバランス"},
    {"card_gray_desc", "32階調ステップバー、滑らかな連続グラデーション、RGB原色リニアリティ"},
    {"card_sharp_title", "5. シャープネス & フォーカス"},
    {"card_sharp_desc", "1px白黒格子・市松模様による輪郭補正（ハロー/リンギング）の最適化"},
    {"card_unif_title", "6. 色均一性 & ドット抜け検査"},
    {"card_unif_desc", "単色（白・灰・黒・赤・緑・青）全画面表示によるムラ・常時点灯ドット検査"},
    {"card_geom_title", "7. 画面比率 & オーバースキャン"},
    {"card_geom_desc", "1:1ピクセルマッピング、ドット・バイ・ドット、外枠1px欠けの検査"},
    {"btn_open_fullscreen", "全画面で開く"},

    // Hardware Tab
    {"hw_info_group", "🖥️ ディスプレイ情報"},
    {"hw_lbl_model", "モニタ名 / 型番:"},
    {"hw_lbl_res", "解像度 / 位置:"},
    {"hw_lbl_rate", "リフレッシュレート:"},
    {"hw_lbl_dpi", "DPI / スケール:"},
    {"hw_val_res", "%1 x %2 (位置: X=%3, Y=%4)"},
    {"hw_val_rate", "%1 Hz (色深度: %2 bit)"},
    {"hw_val_dpi", "DPI: %1 (スケール: %2x)"},

    {"hw_ctrl_group", "🎛️ ハードウェア / ソフトウェア制御 (DDC/CI & XRandR)"},
    {"hw_status_checking", "ステータス: 確認中..."},
    {"hw_status_ddc_ok", "✅ DDC/CI 連携可能 (モニタ内部パラメータを直接制御できます)"},
    {"hw_status_xrandr", "⚠️ DDC/CI不可 (XRandR ソフトウェア補正またはモニタOSDボタンを使用してください)"},
    {"hw_status_manual", "ℹ️ モニタOSD手動調整モード (モニタ本体の操作ボタンで調整してください)"},
    {"hw_lbl_brightness", "輝度 (Brightness):"},
    {"hw_lbl_contrast", "コントラスト (Contrast):"},
    {"hw_btn_reset_sw", "ソフトウェア補正 (ガンマ/輝度) をデフォルトにリセット"},
    {"hw_dlg_reset_title", "リセット完了"},
    {"hw_dlg_reset_msg", "XRandR ソフトウェア設定をデフォルト (1.0) に戻しました。"},

    // Patterns Drawing Text
    {"pat_black_title", "黒レベル / 輝度 (Brightness) 調整パターン"},
    {"pat_black_desc", "※ 0%（完全な黒）は背景と同化し、1%〜2%が「かろうじて識別できる」状態にモニタの輝度（Brightness）を調整してください。"},
    {"pat_black_blink", "点滅テストボックス (RGB 2 / RGB 4 - 点滅が視認できれば暗部階調は良好)"},

    {"pat_white_title", "白レベル / コントラスト (Contrast) 調整パターン"},
    {"pat_white_desc", "※ 100%（完全な白）と99%〜99.6%（RGB 254）の境界が識別できる限界までモニタのコントラストを調整してください。"},

    {"pat_gamma_title", "ガンマ 2.2 (Gamma 2.2) 調整パターン"},
    {"pat_gamma_desc", "※ 画面から少し離れて目を細めて見たとき、中央のパッチが背景の白黒ラスタと同化する状態が適正値です。"},
    {"pat_gamma_patch_fmt", "γ %1"},
    {"pat_gamma_target", "(標準 2.2)"},

    {"pat_gray_title", "グレースケール & カラーバランス調整パターン"},
    {"pat_gray_desc", "※ 32段階のステップおよび連続グラデーションに色被り（色付き）やトーンジャンプがないか確認してください。"},
    {"pat_gray_32step", "32段階 グレースケールステップ"},
    {"pat_gray_smooth", "連続階調 リニアグラデーション (8-bit)"},
    {"pat_gray_rgb", "RGB 原色チャンネル階調リニアリティ"},

    {"pat_sharp_title", "シャープネス & フォーカス調整パターン"},
    {"pat_sharp_desc", "※ 文字や線の輪郭に不自然な白フチ（オーバーシュート/リンギング）やモアレがないか確認してください。"},

    {"pat_geom_title", "画面比率 & オーバースキャン (1:1 ピクセルマッピング) 検査"},

    {"color_white", "白"},
    {"color_gray50", "灰 50%"},
    {"color_black", "黒"},
    {"color_red", "赤"},
    {"color_green", "緑"},
    {"color_blue", "青"},
    {"color_cyan", "シアン"},
    {"color_magenta", "マゼンタ"},
    {"color_yellow", "黄"},
    {"color_unif_hint", "[C] キーまたはクリックで色切り替え | [Esc] で終了"},

    // HUD
    {"hud_step_prefix", "ステップ %1 / %2 : "},
    {"hud_osd_tip", "💡 OSD操作: "},
    {"hud_shortcuts", "[Space/Enter] 次へ   [Backspace] 前へ   [H] ガイド表示切替   [F] 全画面   [Esc] 戻る"},
    {"hud_shortcuts_color", "[C] 色切り替え   "},
    {"hud_btn_prev", "◀ 前へ"},
    {"hud_btn_next", "次へ ▶"},
    {"hud_single_title", "テストパターン表示中"},
    {"hud_single_desc", "キーボードの [Esc] で終了、[F] で全画面/ウィンドウ切り替え"},

    // Screen identify
    {"screen_id_banner", "ディスプレイ %1: %2 (%3x%4 @ %5Hz)"},
    {"screen_id_primary", "(プライマリ画面)"}
};

static const QMap<QString, QString> s_translationsEn = {
    // App
    {"app_title", "AdjustDisplay - Visual Display Calibration Tool"},
    {"app_display_name", "AdjustDisplay"},

    // Header
    {"screen_primary_tag", "(Primary)"},
    {"btn_identify", "🎯 Identify"},
    {"btn_identify_tip", "Display large identification banner on the selected screen"},
    {"btn_identify_all", "✨ Identify All"},
    {"btn_identify_all_tip", "Display identification numbers on all screens simultaneously"},
    {"btn_refresh", "🔄 Refresh"},
    {"lang_label", "🌐 Language:"},

    // Tabs
    {"tab_wizard", "🧙‍♂️ Guided Wizard"},
    {"tab_explorer", "🎨 Pattern Explorer"},
    {"tab_hardware", "⚙️ Display & DDC/CI Settings"},

    // Wizard Tab
    {"wizard_sidebar_title", "📋 Adjustment Steps"},
    {"btn_wizard_prev", "◀ Previous Step"},
    {"btn_wizard_next", "Next Step ▶"},
    {"btn_wizard_finish", "Finish 🎉"},
    {"btn_wizard_fullscreen", "🖥️ Adjust in Fullscreen (Recommended)"},

    // Wizard Steps
    {"step1_short", "1. Black Level"},
    {"step1_title", "Black Level / Brightness Adjustment"},
    {"step1_subtitle", "Calibrate shadow detail thresholds"},
    {"step1_inst", "Lower your monitor's Brightness until 0% (pure black) blends seamlessly into the background and the 1%-2% bars are barely distinguishable. This maintains shadow detail while preventing elevated blacks."},
    {"step1_osd", "Adjust 'Brightness' in monitor OSD. Adjusting in a dimly lit room is recommended."},

    {"step2_short", "2. White Level"},
    {"step2_title", "White Level / Contrast Adjustment"},
    {"step2_subtitle", "Prevent highlight clipping"},
    {"step2_inst", "Increase Contrast up to the highest point where the boundaries between 100% (pure white) and 99%-99.6% (RGB 254) remain clearly visible. Setting it too high will cause highlight clipping/crushing."},
    {"step2_osd", "Adjust 'Contrast' in monitor OSD. Ensure no color tint/clipping across RGB highlights."},

    {"step3_short", "3. Gamma 2.2"},
    {"step3_title", "Gamma 2.2 Calibration"},
    {"step3_subtitle", "Align with standard sRGB tone curve"},
    {"step3_inst", "Step back slightly from your screen and squint. The optimal setting is when the center 'γ 2.2' patch visually blends seamlessly with the black-and-white striped background."},
    {"step3_osd", "Select '2.2' in monitor OSD 'Gamma' settings or switch color profile presets."},

    {"step4_short", "4. Grayscale"},
    {"step4_title", "Grayscale & Color Balance Verification"},
    {"step4_subtitle", "Verify neutral achromatic gray"},
    {"step4_inst", "Examine the 32-step grayscale bar and continuous gradient to ensure there is no color cast (green, red, blue tinting) or sudden tone jumping (banding) throughout the midtones."},
    {"step4_osd", "If a specific color cast is noticeable, fine-tune 'Color Temperature' or RGB Gains in monitor OSD."},

    {"step5_short", "5. Sharpness"},
    {"step5_title", "Sharpness & Focus Adjustment"},
    {"step5_subtitle", "Optimize text clarity and edge rendering"},
    {"step5_inst", "Check that text and 1px grids have clean edges without unnatural white halos (ringing/overshoot) or moire artifacts. Excessive sharpness artificially emphasizes outlines."},
    {"step5_osd", "Set monitor OSD 'Sharpness' to 50% or Off/Standard (native unenhanced)."},

    {"step6_short", "6. Uniformity"},
    {"step6_title", "Color Uniformity & Dead Pixel Check"},
    {"step6_subtitle", "Inspect panel luminance uniformity and defects"},
    {"step6_inst", "Inspect fullscreen White, Gray, Red, Green, and Blue fields for corner vignetting, color shading, or stuck/dead sub-pixels. Press [C] to cycle background colors."},
    {"step6_osd", "Visually inspect overall screen brightness and color temperature consistency."},

    {"step7_short", "7. Geometry"},
    {"step7_title", "Screen Ratio & Overscan Test"},
    {"step7_subtitle", "Confirm 1:1 pixel mapping (Dot-by-Dot)"},
    {"step7_inst", "Verify that the outer 1-pixel white border is completely visible along all four edges without clipping, and that geometric circles are perfectly round."},
    {"step7_osd", "Set monitor OSD 'Aspect Ratio' / 'Overscan' / 'Screen Size' to 'Dot by Dot', 'Just Scan', or 'Full'."},

    // Explorer Cards
    {"card_black_title", "1. Black Level / Brightness"},
    {"card_black_desc", "Calibrate shadow detail thresholds using 0%-5% luminance steps and blinking test blocks."},
    {"card_white_title", "2. White Level / Contrast"},
    {"card_white_desc", "Prevent highlight clipping using 95%-100% white steps and RGB peak bars."},
    {"card_gamma_title", "3. Gamma 2.2 (Gamma 2.2)"},
    {"card_gamma_desc", "Calibrate gamma curve using 1px alternating raster lines against reference luminance patches."},
    {"card_gray_title", "4. Grayscale & Color Balance"},
    {"card_gray_desc", "Verify 32-step grayscale linearity, smooth gradation, and RGB primary channel balance."},
    {"card_sharp_title", "5. Sharpness & Focus"},
    {"card_sharp_desc", "Optimize edge rendering and eliminate ringing/halos using 1px grids and checkerboards."},
    {"card_unif_title", "6. Color Uniformity & Dead Pixels"},
    {"card_unif_desc", "Inspect panel uniformity, vignetting, and pixel defects with pure fullscreen colors."},
    {"card_geom_title", "7. Screen Ratio & Overscan"},
    {"card_geom_desc", "Test 1:1 pixel mapping, dot-by-dot integrity, and 1px outer edge clipping."},
    {"btn_open_fullscreen", "Open Fullscreen"},

    // Hardware Tab
    {"hw_info_group", "🖥️ Display Information"},
    {"hw_lbl_model", "Monitor / Model:"},
    {"hw_lbl_res", "Resolution / Position:"},
    {"hw_lbl_rate", "Refresh Rate:"},
    {"hw_lbl_dpi", "DPI / Scale:"},
    {"hw_val_res", "%1 x %2 (Position: X=%3, Y=%4)"},
    {"hw_val_rate", "%1 Hz (Color Depth: %2-bit)"},
    {"hw_val_dpi", "DPI: %1 (Scale: %2x)"},

    {"hw_ctrl_group", "🎛️ Hardware / Software Control (DDC/CI & XRandR)"},
    {"hw_status_checking", "Status: Checking..."},
    {"hw_status_ddc_ok", "✅ DDC/CI Available (Direct monitor hardware control enabled)"},
    {"hw_status_xrandr", "⚠️ DDC/CI Unavailable (Use XRandR software calibration or monitor OSD buttons)"},
    {"hw_status_manual", "ℹ️ Manual Monitor OSD Mode (Please adjust via monitor physical buttons)"},
    {"hw_lbl_brightness", "Brightness:"},
    {"hw_lbl_contrast", "Contrast:"},
    {"hw_btn_reset_sw", "Reset Software Calibration (Gamma/Brightness) to Default"},
    {"hw_dlg_reset_title", "Reset Complete"},
    {"hw_dlg_reset_msg", "XRandR software settings have been reset to default (1.0)."},

    // Patterns Drawing Text
    {"pat_black_title", "Black Level / Brightness Calibration Pattern"},
    {"pat_black_desc", "※ Adjust monitor Brightness until 0% (pure black) blends into background and 1%-2% is barely visible."},
    {"pat_black_blink", "Blinking test boxes (RGB 2 / RGB 4 - Good dark gradation if blinking is visible)"},

    {"pat_white_title", "White Level / Contrast Calibration Pattern"},
    {"pat_white_desc", "※ Adjust monitor Contrast to the highest level where 100% and 99%-99.6% (RGB 254) boundaries remain distinct."},

    {"pat_gamma_title", "Gamma 2.2 Calibration Pattern"},
    {"pat_gamma_desc", "※ Step back slightly and squint: adjust until center patch blends with the 1px raster background."},
    {"pat_gamma_patch_fmt", "γ %1"},
    {"pat_gamma_target", "(Target 2.2)"},

    {"pat_gray_title", "Grayscale & Color Balance Calibration Pattern"},
    {"pat_gray_desc", "※ Check for smooth tonal steps without tinting, color cast, or banding across all 32 steps."},
    {"pat_gray_32step", "32-Step Grayscale Bars"},
    {"pat_gray_smooth", "Smooth Continuous Linear Gradient (8-bit)"},
    {"pat_gray_rgb", "RGB Primary Channels Linearity"},

    {"pat_sharp_title", "Sharpness & Focus Calibration Pattern"},
    {"pat_sharp_desc", "※ Check text and lines for unnatural white halos (ringing/overshoot) or moire patterns."},

    {"pat_geom_title", "Screen Ratio & Overscan (1:1 Pixel Mapping) Test"},

    {"color_white", "White"},
    {"color_gray50", "Gray 50%"},
    {"color_black", "Black"},
    {"color_red", "Red"},
    {"color_green", "Green"},
    {"color_blue", "Blue"},
    {"color_cyan", "Cyan"},
    {"color_magenta", "Magenta"},
    {"color_yellow", "Yellow"},
    {"color_unif_hint", "Press [C] or Click to cycle colors | [Esc] to exit"},

    // HUD
    {"hud_step_prefix", "Step %1 / %2 : "},
    {"hud_osd_tip", "💡 Monitor OSD: "},
    {"hud_shortcuts", "[Space/Enter] Next   [Backspace] Prev   [H] Toggle HUD   [F] Fullscreen   [Esc] Exit"},
    {"hud_shortcuts_color", "[C] Cycle Color   "},
    {"hud_btn_prev", "◀ Prev"},
    {"hud_btn_next", "Next ▶"},
    {"hud_single_title", "Pattern Preview"},
    {"hud_single_desc", "Press [Esc] to exit, [F] to toggle fullscreen/windowed"},

    // Screen identify
    {"screen_id_banner", "Display %1: %2 (%3x%4 @ %5Hz)"},
    {"screen_id_primary", "(Primary Screen)"}
};

QString I18n::t(const char *key) const {
    QString k = QString::fromUtf8(key);
    if (m_language == Language::Japanese) {
        return s_translationsJa.value(k, k);
    } else {
        return s_translationsEn.value(k, k);
    }
}
