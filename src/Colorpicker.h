#pragma once
#include <SFML/Graphics.hpp>
#include <optional>

class ColorPicker {
public:
    ColorPicker(sf::Vector2f position, float size);

    void handleEvent(const sf::Event& event, sf::RenderWindow& window, const sf::View& view);
    void draw(sf::RenderWindow& window);

    sf::Color getColor() const { return m_currentColor; }
    void      setColor(sf::Color c);

    bool contains(sf::Vector2f point) const;

private:
    sf::Vector2f m_pos;
    float        m_size;

    float m_hue = 0.f;   // 0–360
    float m_sat = 1.f;   // 0–1
    float m_val = 1.f;   // 0–1

    sf::Color m_currentColor;

    sf::Texture              m_svTexture;
    std::optional<sf::Sprite> m_svSprite;
    sf::Texture              m_hueTexture;
    std::optional<sf::Sprite> m_hueSprite;

    bool m_draggingSV  = false;
    bool m_draggingHue = false;

    void rebuildSVTexture();
    void rebuildHueTexture();
    void updateColor();
    sf::Color hsv2rgb(float h, float s, float v) const;

    // layout
    sf::FloatRect svRect()     const;
    sf::FloatRect hueRect()    const;
    sf::FloatRect swatchRect() const;
};