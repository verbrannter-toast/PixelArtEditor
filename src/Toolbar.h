#pragma once
#include <SFML/Graphics.hpp>
#include "Canvas.h"

class Toolbar {
public:
    Toolbar(sf::Vector2f position, float width, sf::Font& font);

    void handleEvent(const sf::Event& event, sf::RenderWindow& window, const sf::View& view);
    void draw(sf::RenderWindow& window);

    BrushShape getBrushShape() const { return m_shape; }
    int        getBrushSize()  const { return m_brushSize; }

    bool consumeResizeRequest();
    bool consumeClearRequest();

    float getWidth() const { return m_width; }

private:
    sf::Vector2f m_pos;
    float        m_width;
    sf::Font&    m_font;

    BrushShape m_shape     = BrushShape::Round;
    int        m_brushSize = 1;

    bool m_resizeRequested = false;
    bool m_clearRequested  = false;
    bool m_draggingSlider  = false;

    sf::FloatRect roundBtnRect()  const;
    sf::FloatRect squareBtnRect() const;
    sf::FloatRect sliderTrack()   const;
    sf::FloatRect resizeBtnRect() const;
    sf::FloatRect clearBtnRect()  const;

    float sizeToSliderX(int size) const;
    int   sliderXToSize(float x)  const;

    void drawButton(sf::RenderWindow& w, sf::FloatRect r,
                    const std::string& label, bool active);
};