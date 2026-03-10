#include "Toolbar.h"
#include <string>
#include <algorithm>
#include <cmath>

static constexpr float ROW_H = 34.f;
static constexpr float PAD   = 10.f;
static constexpr float BTN_H = 28.f;

Toolbar::Toolbar(sf::Vector2f position, float width, sf::Font& font)
    : m_pos(position), m_width(width), m_font(font) {}

// layout
sf::FloatRect Toolbar::roundBtnRect() const {
    float bw = (m_width - PAD*3) / 2.f;
    return sf::FloatRect({m_pos.x + PAD, m_pos.y + PAD + ROW_H}, {bw, BTN_H});
}
sf::FloatRect Toolbar::squareBtnRect() const {
    float bw = (m_width - PAD*3) / 2.f;
    return sf::FloatRect({m_pos.x + PAD*2 + bw, m_pos.y + PAD + ROW_H}, {bw, BTN_H});
}
sf::FloatRect Toolbar::sliderTrack() const {
    return sf::FloatRect({m_pos.x + PAD, m_pos.y + PAD + ROW_H*3}, {m_width - PAD*2, 6.f});
}
sf::FloatRect Toolbar::resizeBtnRect() const {
    return sf::FloatRect({m_pos.x + PAD, m_pos.y + PAD + ROW_H*5}, {m_width - PAD*2, BTN_H});
}
sf::FloatRect Toolbar::clearBtnRect() const {
    return sf::FloatRect({m_pos.x + PAD, m_pos.y + PAD + ROW_H*6 + PAD}, {m_width - PAD*2, BTN_H});
}

float Toolbar::sizeToSliderX(int size) const {
    auto tr = sliderTrack();
    float t = std::log2f(static_cast<float>(size)) / std::log2f(32.f);
    return tr.position.x + t * tr.size.x;
}
int Toolbar::sliderXToSize(float x) const {
    auto tr = sliderTrack();
    float t = std::clamp((x - tr.position.x) / tr.size.x, 0.f, 1.f);
    float sz = std::pow(2.f, t * std::log2f(32.f));
    return std::clamp(static_cast<int>(std::round(sz)), 1, 32);
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

// drawing
void Toolbar::drawButton(sf::RenderWindow& w, sf::FloatRect r,
                         const std::string& label, bool active) {
    sf::RectangleShape btn({r.size.x, r.size.y});
    btn.setPosition({r.position.x, r.position.y});
    btn.setFillColor(active ? sf::Color(80, 140, 220) : sf::Color(60, 63, 65));
    btn.setOutlineColor(active ? sf::Color(120, 180, 255) : sf::Color(90, 93, 95));
    btn.setOutlineThickness(1.f);
    w.draw(btn);

    sf::Text txt(m_font, label, 12);
    txt.setFillColor(sf::Color::White);
    auto tb = txt.getLocalBounds();
    txt.setPosition({r.position.x + (r.size.x - tb.size.x)/2.f - tb.position.x,
                     r.position.y + (r.size.y - tb.size.y)/2.f - tb.position.y});
    w.draw(txt);
}

void Toolbar::draw(sf::RenderWindow& window) {
    // panel background
    sf::RectangleShape panel({m_width, static_cast<float>(window.getSize().y)});
    panel.setPosition(m_pos);
    panel.setFillColor(sf::Color(40, 42, 44));
    window.draw(panel);

    // brush shape
    sf::Text lbl(m_font, "Brush Shape", 12);
    lbl.setFillColor(sf::Color(180, 180, 180));
    lbl.setPosition({m_pos.x + PAD, m_pos.y + PAD + ROW_H - 16.f});
    window.draw(lbl);

    drawButton(window, roundBtnRect(),  "Round",  m_shape == BrushShape::Round);
    drawButton(window, squareBtnRect(), "Square", m_shape == BrushShape::Square);

    // brush size
    sf::Text szLbl(m_font, "Brush Size: " + std::to_string(m_brushSize), 12);
    szLbl.setFillColor(sf::Color(180, 180, 180));
    szLbl.setPosition({m_pos.x + PAD, m_pos.y + PAD + ROW_H*2 + 6.f});
    window.draw(szLbl);

    auto tr = sliderTrack();
    sf::RectangleShape track({tr.size.x, tr.size.y});
    track.setPosition({tr.position.x, tr.position.y});
    track.setFillColor(sf::Color(80, 83, 85));
    window.draw(track);

    float knobX = sizeToSliderX(m_brushSize);
    sf::RectangleShape fill({knobX - tr.position.x, tr.size.y});
    fill.setPosition({tr.position.x, tr.position.y});
    fill.setFillColor(sf::Color(80, 140, 220));
    window.draw(fill);

    sf::CircleShape knob(7.f);
    knob.setOrigin({7.f, 7.f});
    knob.setPosition({knobX, tr.position.y + tr.size.y / 2.f});
    knob.setFillColor(sf::Color(200, 220, 255));
    knob.setOutlineColor(sf::Color(80, 140, 220));
    knob.setOutlineThickness(1.5f);
    window.draw(knob);

    // brush shape preview
    float previewY = tr.position.y + 24.f;
    float previewX = m_pos.x + m_width / 2.f;
    float halfPx   = std::min(static_cast<float>(m_brushSize) * 3.f, 30.f) / 2.f;

    if (m_shape == BrushShape::Round) {
        sf::CircleShape preview(halfPx);
        preview.setOrigin({halfPx, halfPx});
        preview.setPosition({previewX, previewY + halfPx});
        preview.setFillColor(sf::Color(200, 220, 255));
        window.draw(preview);
    } else {
        sf::RectangleShape preview({halfPx*2, halfPx*2});
        preview.setOrigin({halfPx, halfPx});
        preview.setPosition({previewX, previewY + halfPx});
        preview.setFillColor(sf::Color(200, 220, 255));
        window.draw(preview);
    }

    // utility buttons
    drawButton(window, resizeBtnRect(), "Resize Canvas", false);
    drawButton(window, clearBtnRect(),  "Clear Canvas",  false);
}