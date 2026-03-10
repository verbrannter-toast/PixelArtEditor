#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

enum class BrushShape { Round, Square };

class Canvas {
public:
    Canvas(int width, int height, int pixelSize);

    void resize(int newWidth, int newHeight);
    void draw(sf::RenderWindow& window, sf::Vector2f offset);
    void paint(sf::Vector2f mousePos, sf::Vector2f offset, sf::Color color,
               int brushSize, BrushShape shape);
    void clear(sf::Color color = sf::Color::White);

    int  getWidth()     const { return m_width; }
    int  getHeight()    const { return m_height; }
    int  getPixelSize() const { return m_pixelSize; }
    void setPixelSize(int ps) { m_pixelSize = ps; rebuildVertices(); }

    sf::Color getPixel(int x, int y) const;
    void      setPixel(int x, int y, sf::Color color);

private:
    int m_width, m_height, m_pixelSize;
    std::vector<sf::Color> m_pixels;

    sf::VertexArray m_vertices;
    void rebuildVertices();
    void updatePixelVertices(int x, int y);
};