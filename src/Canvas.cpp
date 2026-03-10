#include "Canvas.h"
#include <cmath>
#include <algorithm>

Canvas::Canvas(int width, int height, int pixelSize)
    : m_width(width), m_height(height), m_pixelSize(pixelSize),
      m_pixels(width * height, sf::Color::White),
      m_vertices(sf::PrimitiveType::Triangles, width * height * 6)
{
    rebuildVertices();
}

void Canvas::resize(int newWidth, int newHeight) {
    std::vector<sf::Color> newPixels(newWidth * newHeight, sf::Color::White);
    int copyW = std::min(m_width, newWidth);
    int copyH = std::min(m_height, newHeight);
    for (int y = 0; y < copyH; ++y)
        for (int x = 0; x < copyW; ++x)
            newPixels[y * newWidth + x] = m_pixels[y * m_width + x];

    m_width  = newWidth;
    m_height = newHeight;
    m_pixels = std::move(newPixels);
    m_vertices = sf::VertexArray(sf::PrimitiveType::Triangles, m_width * m_height * 6);
    rebuildVertices();
}

void Canvas::clear(sf::Color color) {
    std::fill(m_pixels.begin(), m_pixels.end(), color);
    rebuildVertices();
}

sf::Color Canvas::getPixel(int x, int y) const {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) return sf::Color::Transparent;
    return m_pixels[y * m_width + x];
}

void Canvas::setPixel(int x, int y, sf::Color color) {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) return;
    m_pixels[y * m_width + x] = color;
    updatePixelVertices(x, y);
}

void Canvas::rebuildVertices() {
    for (int y = 0; y < m_height; ++y)
        for (int x = 0; x < m_width; ++x)
            updatePixelVertices(x, y);
}

// SFML 3 dropped quads so I am using two triangles for one pixel
void Canvas::updatePixelVertices(int x, int y) {
    int idx = (y * m_width + x) * 6;
    float px = static_cast<float>(x * m_pixelSize);
    float py = static_cast<float>(y * m_pixelSize);
    float ps = static_cast<float>(m_pixelSize);
    sf::Color c = m_pixels[y * m_width + x];

    // triangle 1: top-left, top-right, bottom-left
    m_vertices[idx + 0].position = {px,      py};
    m_vertices[idx + 1].position = {px + ps, py};
    m_vertices[idx + 2].position = {px,      py + ps};
    // triangle 2: top-right, bottom-right, bottom-left
    m_vertices[idx + 3].position = {px + ps, py};
    m_vertices[idx + 4].position = {px + ps, py + ps};
    m_vertices[idx + 5].position = {px,      py + ps};

    for (int i = 0; i < 6; ++i) m_vertices[idx + i].color = c;
}

void Canvas::paint(sf::Vector2f mousePos, sf::Vector2f offset,
                   sf::Color color, int brushSize, BrushShape shape)
{
    // use floorf so negative coords also round toward canvas origin correctly
    int cx = static_cast<int>(std::floor((mousePos.x - offset.x) / static_cast<float>(m_pixelSize)));
    int cy = static_cast<int>(std::floor((mousePos.y - offset.y) / static_cast<float>(m_pixelSize)));

    int half = brushSize / 2;
    for (int dy = -half; dy <= half; ++dy) {
        for (int dx = -half; dx <= half; ++dx) {
            if (shape == BrushShape::Round) {
                float dist = std::sqrt(static_cast<float>(dx*dx + dy*dy));
                if (dist > half + 0.5f) continue;
            }
            setPixel(cx + dx, cy + dy, color);
        }
    }
}

void Canvas::draw(sf::RenderWindow& window, sf::Vector2f offset) {
    float cw = static_cast<float>(m_width  * m_pixelSize);
    float ch = static_cast<float>(m_height * m_pixelSize);

    // canvas border and little drop shadow
    sf::RectangleShape shadow({cw + 2.f, ch + 2.f});
    shadow.setPosition({offset.x + 4.f, offset.y + 4.f});
    shadow.setFillColor(sf::Color(0, 0, 0, 80));
    window.draw(shadow);

    sf::RectangleShape border({cw, ch});
    border.setPosition(offset);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color(55, 55, 65));
    border.setOutlineThickness(1.f);
    window.draw(border);

    // pixels
    sf::Transform t;
    t.translate(offset);
    window.draw(m_vertices, t);

    // grid (only appears when zoomed in enough)
    if (m_pixelSize >= 6) {
        sf::VertexArray grid(sf::PrimitiveType::Lines);
        sf::Color gridColor(0, 0, 0, 40);
        for (int x = 0; x <= m_width; ++x) {
            float fx = offset.x + x * m_pixelSize;
            grid.append({{fx, offset.y},  gridColor});
            grid.append({{fx, offset.y + m_height * m_pixelSize}, gridColor});
        }
        for (int y = 0; y <= m_height; ++y) {
            float fy = offset.y + y * m_pixelSize;
            grid.append({{offset.x, fy}, gridColor});
            grid.append({{offset.x + m_width * m_pixelSize, fy}, gridColor});
        }
        window.draw(grid);
    }
}