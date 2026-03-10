#include "ColorPicker.h"
#include <algorithm>
#include <cmath>
#include <vector>

static constexpr float HUE_BAR_WIDTH = 18.f;
static constexpr float SWATCH_HEIGHT = 24.f;
static constexpr float GAP           = 6.f;

// layout
sf::FloatRect ColorPicker::svRect() const {
    float w = m_size - HUE_BAR_WIDTH - GAP;
    float h = m_size - SWATCH_HEIGHT - GAP;
    return sf::FloatRect({m_pos.x, m_pos.y}, {w, h});
}
sf::FloatRect ColorPicker::hueRect() const {
    float svW = m_size - HUE_BAR_WIDTH - GAP;
    float h   = m_size - SWATCH_HEIGHT - GAP;
    return sf::FloatRect({m_pos.x + svW + GAP, m_pos.y}, {HUE_BAR_WIDTH, h});
}
sf::FloatRect ColorPicker::swatchRect() const {
    return sf::FloatRect({m_pos.x, m_pos.y + m_size - SWATCH_HEIGHT}, {m_size, SWATCH_HEIGHT});
}

// HSV helper
sf::Color ColorPicker::hsv2rgb(float h, float s, float v) const {
    h = std::fmod(h, 360.f);
    if (h < 0) h += 360.f;
    float c = v * s;
    float x = c * (1.f - std::fabs(std::fmod(h / 60.f, 2.f) - 1.f));
    float m = v - c;
    float r, g, b;
    if      (h < 60)  { r=c; g=x; b=0; }
    else if (h < 120) { r=x; g=c; b=0; }
    else if (h < 180) { r=0; g=c; b=x; }
    else if (h < 240) { r=0; g=x; b=c; }
    else if (h < 300) { r=x; g=0; b=c; }
    else              { r=c; g=0; b=x; }
    return sf::Color(
        static_cast<uint8_t>((r+m)*255),
        static_cast<uint8_t>((g+m)*255),
        static_cast<uint8_t>((b+m)*255));
}

// constructor
ColorPicker::ColorPicker(sf::Vector2f position, float size)
    : m_pos(position), m_size(size)
{
    auto sv = svRect();
    auto hu = hueRect();

    m_svTexture  = sf::Texture(sf::Vector2u(
        static_cast<unsigned>(sv.size.x),
        static_cast<unsigned>(sv.size.y)));
    m_hueTexture = sf::Texture(sf::Vector2u(
        static_cast<unsigned>(hu.size.x),
        static_cast<unsigned>(hu.size.y)));

    rebuildSVTexture();
    rebuildHueTexture();

    m_svSprite.emplace(m_svTexture);
    m_hueSprite.emplace(m_hueTexture);

    m_svSprite->setPosition( {sv.position.x, sv.position.y});
    m_hueSprite->setPosition({hu.position.x, hu.position.y});

    updateColor();
}

// texture builders
void ColorPicker::rebuildSVTexture() {
    auto sv = svRect();
    unsigned W = static_cast<unsigned>(sv.size.x);
    unsigned H = static_cast<unsigned>(sv.size.y);
    std::vector<uint8_t> pixels(W * H * 4);
    for (unsigned y = 0; y < H; ++y) {
        for (unsigned x = 0; x < W; ++x) {
            float s = static_cast<float>(x) / (W - 1);
            float v = 1.f - static_cast<float>(y) / (H - 1);
            sf::Color c = hsv2rgb(m_hue, s, v);
            int i = (y * W + x) * 4;
            pixels[i]   = c.r;
            pixels[i+1] = c.g;
            pixels[i+2] = c.b;
            pixels[i+3] = 255;
        }
    }
    m_svTexture.update(pixels.data());
}

void ColorPicker::rebuildHueTexture() {
    auto hu = hueRect();
    unsigned W = static_cast<unsigned>(hu.size.x);
    unsigned H = static_cast<unsigned>(hu.size.y);
    std::vector<uint8_t> pixels(W * H * 4);
    for (unsigned y = 0; y < H; ++y) {
        float h = (static_cast<float>(y) / (H - 1)) * 360.f;
        sf::Color c = hsv2rgb(h, 1.f, 1.f);
        for (unsigned x = 0; x < W; ++x) {
            int i = (y * W + x) * 4;
            pixels[i]   = c.r;
            pixels[i+1] = c.g;
            pixels[i+2] = c.b;
            pixels[i+3] = 255;
        }
    }
    m_hueTexture.update(pixels.data());
}

void ColorPicker::updateColor() {
    m_currentColor = hsv2rgb(m_hue, m_sat, m_val);
}

// public
bool ColorPicker::contains(sf::Vector2f p) const {
    return sf::FloatRect({m_pos.x, m_pos.y}, {m_size, m_size}).contains(p);
}

void ColorPicker::setColor(sf::Color c) {
    float r = c.r / 255.f, g = c.g / 255.f, b = c.b / 255.f;
    float mx = std::max({r,g,b}), mn = std::min({r,g,b}), d = mx - mn;
    m_val = mx;
    m_sat = (mx > 0) ? d / mx : 0;
    if (d == 0) m_hue = 0;
    else if (mx == r) m_hue = 60.f * std::fmod((g-b)/d, 6.f);
    else if (mx == g) m_hue = 60.f * ((b-r)/d + 2);
    else              m_hue = 60.f * ((r-g)/d + 4);
    if (m_hue < 0) m_hue += 360.f;
    rebuildSVTexture();
    updateColor();
}

// events
void ColorPicker::handleEvent(const sf::Event& event, sf::RenderWindow& window, const sf::View& view) {
    auto sv = svRect();
    auto hu = hueRect();

    if (const auto* e = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (e->button == sf::Mouse::Button::Left) {
            sf::Vector2f mp = window.mapPixelToCoords(e->position, view);
            if (sv.contains(mp)) m_draggingSV  = true;
            if (hu.contains(mp)) m_draggingHue = true;
        }
    }
    if (event.is<sf::Event::MouseButtonReleased>()) {
        m_draggingSV  = false;
        m_draggingHue = false;
    }

    sf::Vector2f mp;
    bool hasMouse = false;
    if (const auto* e = event.getIf<sf::Event::MouseMoved>()) {
        mp = window.mapPixelToCoords(e->position, view);
        hasMouse = true;
    } else if (const auto* e = event.getIf<sf::Event::MouseButtonPressed>()) {
        mp = window.mapPixelToCoords(e->position, view);
        hasMouse = true;
    }

    if (hasMouse) {
        if (m_draggingSV) {
            m_sat = std::clamp((mp.x - sv.position.x) / sv.size.x, 0.f, 1.f);
            m_val = std::clamp(1.f - (mp.y - sv.position.y) / sv.size.y, 0.f, 1.f);
            updateColor();
        }
        if (m_draggingHue) {
            m_hue = std::clamp((mp.y - hu.position.y) / hu.size.y, 0.f, 1.f) * 360.f;
            rebuildSVTexture();
            updateColor();
        }
    }
}

// draw
void ColorPicker::draw(sf::RenderWindow& window) {
    window.draw(*m_svSprite);
    window.draw(*m_hueSprite);

    // SV crosshair cursor
    auto sv = svRect();
    float cx = sv.position.x + m_sat * sv.size.x;
    float cy = sv.position.y + (1.f - m_val) * sv.size.y;
    sf::CircleShape cursor(5.f);
    cursor.setOrigin({5.f, 5.f});
    cursor.setPosition({cx, cy});
    cursor.setFillColor(sf::Color::Transparent);
    cursor.setOutlineColor(sf::Color::White);
    cursor.setOutlineThickness(2.f);
    window.draw(cursor);
    cursor.setOutlineColor(sf::Color::Black);
    cursor.setOutlineThickness(1.f);
    cursor.setRadius(6.f);
    cursor.setOrigin({6.f, 6.f});
    window.draw(cursor);

    // hue cursor
    auto hu = hueRect();
    float hy = hu.position.y + (m_hue / 360.f) * hu.size.y;
    sf::RectangleShape hcursor({hu.size.x + 4.f, 3.f});
    hcursor.setOrigin({2.f, 1.5f});
    hcursor.setPosition({hu.position.x, hy});
    hcursor.setFillColor(sf::Color::White);
    hcursor.setOutlineColor(sf::Color::Black);
    hcursor.setOutlineThickness(1.f);
    window.draw(hcursor);

    // color swatch
    auto sw = swatchRect();
    sf::RectangleShape swatch({sw.size.x, sw.size.y});
    swatch.setPosition({sw.position.x, sw.position.y});
    swatch.setFillColor(m_currentColor);
    swatch.setOutlineColor(sf::Color(100,100,100));
    swatch.setOutlineThickness(1.f);
    window.draw(swatch);
}