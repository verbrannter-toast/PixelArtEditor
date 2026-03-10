#include "Toolbar.h"
#include <string>
#include <algorithm>
#include <cmath>

static constexpr float PAD            = 16.f;
static constexpr float BTN_H          = 44.f;
static constexpr float SECTION_LABEL_H = 24.f;
static constexpr float RULE_H          = 14.f;
static constexpr float FONT_LABEL      = 22.f;
static constexpr float FONT_BTN        = 22.f;
static constexpr float FONT_VALUE      = 26.f;
static constexpr float SLIDER_H        = 10.f;
static constexpr float WELL_SIZE       = 60.f;

struct ToolbarLayout {
    float bw;
    float roundBtnY;
    float squareBtnY;
    float sliderLabelY;
    float sliderValueY;
    float sliderY;
    float previewY;
    float ruleBelowPreviewY;
    float canvasLabelY;
    float resizeBtnY;
    float clearBtnY;
};

static ToolbarLayout computeLayout(float posY, float width) {
    ToolbarLayout L;
    L.bw = (width - PAD * 3.f) / 2.f;

    float cur = posY + PAD;
    L.roundBtnY  = cur + SECTION_LABEL_H + 4.f;
    L.squareBtnY = L.roundBtnY;
    cur = L.roundBtnY + BTN_H + RULE_H;

    L.sliderLabelY = cur;
    cur += SECTION_LABEL_H + 4.f;
    L.sliderValueY = cur;
    cur += SECTION_LABEL_H + 6.f;
    L.sliderY = cur;
    cur += SLIDER_H + PAD;

    L.previewY = cur;
    cur += WELL_SIZE + RULE_H;

    L.ruleBelowPreviewY = cur;
    cur += RULE_H;

    L.canvasLabelY = cur;
    cur += SECTION_LABEL_H + 4.f;
    L.resizeBtnY = cur;
    cur += BTN_H + PAD / 2.f;
    L.clearBtnY  = cur;

    return L;
}

Toolbar::Toolbar(sf::Vector2f position, float width, sf::Font* font)
    : m_pos(position), m_width(width), m_font(font) {}

// layout rects
sf::FloatRect Toolbar::roundBtnRect() const {
    auto L = computeLayout(m_pos.y, m_width);
    return sf::FloatRect({m_pos.x + PAD, L.roundBtnY}, {L.bw, BTN_H});
}
sf::FloatRect Toolbar::squareBtnRect() const {
    auto L = computeLayout(m_pos.y, m_width);
    return sf::FloatRect({m_pos.x + PAD * 2.f + L.bw, L.squareBtnY}, {L.bw, BTN_H});
}
sf::FloatRect Toolbar::sliderTrack() const {
    auto L = computeLayout(m_pos.y, m_width);
    return sf::FloatRect({m_pos.x + PAD, L.sliderY}, {m_width - PAD * 2.f, 6.f});
}
sf::FloatRect Toolbar::resizeBtnRect() const {
    auto L = computeLayout(m_pos.y, m_width);
    return sf::FloatRect({m_pos.x + PAD, L.resizeBtnY}, {m_width - PAD * 2.f, BTN_H});
}
sf::FloatRect Toolbar::clearBtnRect() const {
    auto L = computeLayout(m_pos.y, m_width);
    return sf::FloatRect({m_pos.x + PAD, L.clearBtnY}, {m_width - PAD * 2.f, BTN_H});
}

float Toolbar::sizeToSliderX(int size) const {
    auto tr = sliderTrack();
    float t = (static_cast<float>(size) - 1.f) / 31.f;
    return tr.position.x + t * tr.size.x;
}
int Toolbar::sliderXToSize(float x) const {
    auto tr = sliderTrack();
    float t = std::clamp((x - tr.position.x) / tr.size.x, 0.f, 1.f);
    return std::clamp(static_cast<int>(std::round(1.f + t * 31.f)), 1, 32);
}

// events
void Toolbar::handleEvent(const sf::Event& event, sf::RenderWindow& window, const sf::View& view) {
    if (const auto* e = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (e->button == sf::Mouse::Button::Left) {
            sf::Vector2f mp = window.mapPixelToCoords(e->position, view);

            if (roundBtnRect().contains(mp))  m_shape = BrushShape::Round;
            if (squareBtnRect().contains(mp)) m_shape = BrushShape::Square;
            if (resizeBtnRect().contains(mp)) m_resizeRequested = true;
            if (clearBtnRect().contains(mp))  m_clearRequested  = true;

            auto tr = sliderTrack();
            sf::FloatRect sliderHit({tr.position.x, tr.position.y - 10.f},
                                    {tr.size.x, tr.size.y + 20.f});
            if (sliderHit.contains(mp)) {
                m_draggingSlider = true;
                m_brushSize = sliderXToSize(mp.x);
            }
        }
    }
    if (event.is<sf::Event::MouseButtonReleased>())
        m_draggingSlider = false;

    if (const auto* e = event.getIf<sf::Event::MouseMoved>()) {
        if (m_draggingSlider) {
            sf::Vector2f mp = window.mapPixelToCoords(e->position, view);
            m_brushSize = sliderXToSize(mp.x);
        }
    }
}

bool Toolbar::consumeResizeRequest() {
    if (m_resizeRequested) { m_resizeRequested = false; return true; }
    return false;
}
bool Toolbar::consumeClearRequest() {
    if (m_clearRequested) { m_clearRequested = false; return true; }
    return false;
}

// color palette
static const sf::Color COL_PANEL      (30,  30,  36);   // sidebar bg
static const sf::Color COL_PANEL_DARK (22,  22,  27);   // deeper inset
static const sf::Color COL_BORDER     (55,  55,  65);   // 1px border
static const sf::Color COL_BORDER_LT  (70,  70,  82);   // lighter border
static const sf::Color COL_BTN        (45,  47,  55);   // idle button
static const sf::Color COL_BTN_HOV    (58,  60,  70);   // (unused – no hover state yet)
static const sf::Color COL_ACCENT     (72, 152, 168);   // teal accent
static const sf::Color COL_ACCENT_DIM (45,  95, 108);   // darker teal
static const sf::Color COL_TEXT       (188, 190, 196);  // main text
static const sf::Color COL_TEXT_DIM   (110, 112, 120);  // secondary text
static const sf::Color COL_PREVIEW_BG (20,  20,  25);   // preview well bg

// draw
void Toolbar::drawButton(sf::RenderWindow& w, sf::FloatRect r,
                         const std::string& label, bool active) {
    sf::RectangleShape btn({r.size.x, r.size.y});
    btn.setPosition({r.position.x, r.position.y});
    btn.setFillColor(active ? COL_ACCENT_DIM : COL_BTN);
    btn.setOutlineColor(active ? COL_ACCENT : COL_BORDER);
    btn.setOutlineThickness(1.f);
    w.draw(btn);

    sf::Text txt(*m_font, label, static_cast<unsigned>(FONT_BTN));
    txt.setFillColor(active ? sf::Color::White : COL_TEXT);
    auto tb = txt.getLocalBounds();
    txt.setPosition({r.position.x + (r.size.x - tb.size.x) / 2.f - tb.position.x,
                     r.position.y + (r.size.y - tb.size.y) / 2.f - tb.position.y});
    w.draw(txt);
}

static void drawRule(sf::RenderWindow& w, float x, float y, float width) {
    sf::RectangleShape rule({width, 1.f});
    rule.setPosition({x, y});
    rule.setFillColor(COL_BORDER);
    w.draw(rule);
}

static void drawSectionLabel(sf::RenderWindow& w, sf::Font& font,
                              const std::string& text, float x, float y) {
    sf::Text lbl(font, text, static_cast<unsigned>(FONT_LABEL));
    lbl.setFillColor(COL_TEXT_DIM);
    lbl.setPosition({x, y});
    w.draw(lbl);
}

void Toolbar::draw(sf::RenderWindow& window) {
    float h = static_cast<float>(window.getSize().y);
    auto  L = computeLayout(m_pos.y, m_width);
    float x = m_pos.x;

    // panel background
    sf::RectangleShape panel({m_width, h});
    panel.setPosition(m_pos);
    panel.setFillColor(COL_PANEL);
    window.draw(panel);

    sf::RectangleShape border({1.f, h});
    border.setPosition({x + m_width - 1.f, m_pos.y});
    border.setFillColor(COL_BORDER);
    window.draw(border);

    // BRUSH SHAPE
    drawSectionLabel(window, *m_font, "BRUSH SHAPE", x + PAD, m_pos.y + PAD);
    drawButton(window, roundBtnRect(),  "Round",  m_shape == BrushShape::Round);
    drawButton(window, squareBtnRect(), "Square", m_shape == BrushShape::Square);

    drawRule(window, x + PAD, L.roundBtnY + BTN_H + RULE_H / 2.f, m_width - PAD * 2.f);

    // BRUSH SIZE
    drawSectionLabel(window, *m_font, "BRUSH SIZE", x + PAD, L.sliderLabelY);

    std::string sizeStr = std::to_string(m_brushSize);
    sf::Text sizeVal(*m_font, sizeStr, static_cast<unsigned>(FONT_VALUE));
    sizeVal.setFillColor(COL_ACCENT);
    auto svb = sizeVal.getLocalBounds();
    sizeVal.setPosition({x + m_width - PAD - svb.size.x - svb.position.x, L.sliderValueY});
    window.draw(sizeVal);

    auto tr = sliderTrack();
    sf::RectangleShape trackBg({tr.size.x, tr.size.y});
    trackBg.setPosition({tr.position.x, tr.position.y});
    trackBg.setFillColor(COL_PANEL_DARK);
    trackBg.setOutlineColor(COL_BORDER);
    trackBg.setOutlineThickness(1.f);
    window.draw(trackBg);

    float knobX = sizeToSliderX(m_brushSize);
    sf::RectangleShape fill({std::max(0.f, knobX - tr.position.x), tr.size.y});
    fill.setPosition({tr.position.x, tr.position.y});
    fill.setFillColor(COL_ACCENT_DIM);
    window.draw(fill);

    sf::RectangleShape knob({8.f, tr.size.y + 10.f});
    knob.setOrigin({4.f, (tr.size.y + 10.f) / 2.f});
    knob.setPosition({knobX, tr.position.y + tr.size.y / 2.f});
    knob.setFillColor(COL_ACCENT);
    window.draw(knob);

    // BRUSH PREVIEW
    float wellX = x + (m_width - WELL_SIZE) / 2.f;
    sf::RectangleShape well({WELL_SIZE, WELL_SIZE});
    well.setPosition({wellX, L.previewY});
    well.setFillColor(COL_PREVIEW_BG);
    well.setOutlineColor(COL_BORDER);
    well.setOutlineThickness(1.f);
    window.draw(well);

    float halfPx = std::min(static_cast<float>(m_brushSize) * 2.5f, WELL_SIZE / 2.f - 4.f);
    float cx2    = wellX + WELL_SIZE / 2.f;
    float cy2    = L.previewY + WELL_SIZE / 2.f;
    if (m_shape == BrushShape::Round) {
        sf::CircleShape preview(halfPx);
        preview.setOrigin({halfPx, halfPx});
        preview.setPosition({cx2, cy2});
        preview.setFillColor(COL_ACCENT);
        window.draw(preview);
    } else {
        sf::RectangleShape preview({halfPx * 2.f, halfPx * 2.f});
        preview.setOrigin({halfPx, halfPx});
        preview.setPosition({cx2, cy2});
        preview.setFillColor(COL_ACCENT);
        window.draw(preview);
    }

    drawRule(window, x + PAD, L.ruleBelowPreviewY, m_width - PAD * 2.f);

    // utility buttons
    drawSectionLabel(window, *m_font, "CANVAS", x + PAD, L.canvasLabelY);
    drawButton(window, resizeBtnRect(), "Resize Canvas", false);
    drawButton(window, clearBtnRect(),  "Clear Canvas",  false);
}