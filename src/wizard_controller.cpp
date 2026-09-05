#include "wizard_controller.h"

WizardController::WizardController(QObject *parent) : QObject(parent) {
    setupSteps();
}

WizardController::~WizardController() = default;

void WizardController::setupSteps() {
    m_steps.clear();

    // Step 1: Black Level
    m_steps.append({
        PatternType::BlackLevel,
        "黒レベル / 輝度 (Brightness) の調整",
        "暗部階調の基準を合わせます",
        "0%（純黒）が背景に溶け込み、1%〜2%のバーがギリギリ判別できるレベルまでモニタの輝度（Brightness）を下げてください。黒浮きを防ぎつつシャドウの階調を保持します。",
        "モニタOSDの「輝度 (Brightness)」を調整。暗い部屋での調整を推奨。"
    });

    // Step 2: White Level
    m_steps.append({
        PatternType::WhiteLevel,
        "白レベル / コントラスト (Contrast) の調整",
        "明部階調の白飛びを防ぎます",
        "100%（純白）と99%〜99.6%（RGB 254）の境界がはっきりと識別できる限界までコントラスト（Contrast）を上げてください。上げすぎると白飛びが発生します。",
        "モニタOSDの「コントラスト (Contrast)」を調整。RGB各色で白飛びがないかも確認。"
    });

    // Step 3: Gamma 2.2
    m_steps.append({
        PatternType::Gamma22,
        "ガンマ 2.2 (Gamma 2.2) の調整",
        "標準sRGBトーンカーブに合わせます",
        "画面から少し離れて目を細めて見たとき、中央の「γ 2.2」パッチが白黒ストライプ背景と同化して見える状態が適正値です。",
        "モニタOSDの「ガンマ (Gamma)」設定で 2.2 を選択、またはカラーモードを変更。"
    });

    // Step 4: Grayscale Ramp
    m_steps.append({
        PatternType::GrayRamp,
        "グレースケール & カラーバランスの確認",
        "中立な無彩色（ニュートラルグレー）を確認します",
        "32段階のステップバーおよび連続グラデーションにおいて、中間に緑や赤、青などの色被り・バンディング（階調段差）がないか確認してください。",
        "特定の色が強い場合はモニタOSDの「色温度 (Color Temp)」やRGBゲインを微調整。"
    });

    // Step 5: Sharpness
    m_steps.append({
        PatternType::Sharpness,
        "シャープネス & フォーカスの調整",
        "文字の視認性とエッジの輪郭補正を適正化します",
        "文字や1px格子の輪郭に不自然な白フチ（ハロー/リンギング）やモアレが発生していないか確認してください。高すぎると輪郭が強調されすぎます。",
        "モニタOSDの「シャープネス (Sharpness)」を50%またはオフ/標準に設定。"
    });

    // Step 6: Uniformity
    m_steps.append({
        PatternType::ColorUniformity,
        "色均一性 & ドット抜け検査",
        "パネル全体の色ムラや輝度ムラ、ドット抜けを検査します",
        "全画面表示の白・灰・赤・緑・青で四隅の暗がり（周辺減光）や色ムラ、常時点灯・消灯ドットをチェックします。[C] キーで背景色を切り替えられます。",
        "画面全体の明るさや色温度の均一性を目視でチェック。"
    });

    // Step 7: Geometry & 1:1 Pixel Mapping
    m_steps.append({
        PatternType::GeometryFocus,
        "画面比率 & オーバースキャン検査",
        "1:1 ドット・バイ・ドット表示を確認します",
        "最外周の白枠（1ピクセル）が隠れずに四辺すべて見えているか、円が真円で描かれているかを確認してください。",
        "モニタOSDの「アスペクト比」「オーバースキャン」「画面サイズ」を「Dot by Dot」や「フル」に設定。"
    });
}

const WizardStepInfo& WizardController::currentStepInfo() const {
    if (m_currentStepIndex >= 0 && m_currentStepIndex < m_steps.size()) {
        return m_steps[m_currentStepIndex];
    }
    static WizardStepInfo empty;
    return empty;
}

void WizardController::nextStep() {
    if (m_currentStepIndex < m_steps.size() - 1) {
        m_currentStepIndex++;
        emit stepChanged(m_currentStepIndex, currentStepInfo());
    } else {
        emit wizardFinished();
    }
}

void WizardController::prevStep() {
    if (m_currentStepIndex > 0) {
        m_currentStepIndex--;
        emit stepChanged(m_currentStepIndex, currentStepInfo());
    }
}

void WizardController::goToStep(int index) {
    if (index >= 0 && index < m_steps.size() && index != m_currentStepIndex) {
        m_currentStepIndex = index;
        emit stepChanged(m_currentStepIndex, currentStepInfo());
    }
}

void WizardController::reset() {
    m_currentStepIndex = 0;
    emit stepChanged(m_currentStepIndex, currentStepInfo());
}
