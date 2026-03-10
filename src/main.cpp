#include <cmath>
#include <SFML/Graphics.hpp>
#include "Canvas.h"
#include "Toolbar.h"
#include "ColorPicker.h"
#include "CanvasSizeDialog.h"

static constexpr unsigned WIN_W     = 1100;
static constexpr unsigned WIN_H     = 750;
static constexpr float    TOOLBAR_W = 140.f;
static constexpr float    COLOR_W   = 160.f;
static constexpr int      INIT_ZOOM = 16;

int main() {
    sf::RenderWindow window(
        sf::VideoMode({WIN_W, WIN_H}),
        "Pixel Art Editor");
    window.setFramerateLimit(60);

    // font
    sf::Font font;
    bool fontLoaded =
        font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf") ||
        font.openFromFile("/usr/share/fonts/TTF/DejaVuSans.ttf")             ||
        font.openFromFile("C:/Windows/Fonts/arial.ttf")                       ||
        font.openFromFile("/System/Library/Fonts/Helvetica.ttc");
    (void)fontLoaded;

    // subelements
    Canvas canvas(32, 32, INIT_ZOOM);
    canvas.clear(sf::Color::White);

    Toolbar toolbar({0.f, 0.f}, TOOLBAR_W, font);

    float cpSize = COLOR_W - 16.f;
    ColorPicker colorPicker({WIN_W - COLOR_W + 8.f, 8.f}, cpSize);

    CanvasSizeDialog sizeDialog(font);

    // canvas offset
    auto centeredOffset = [&]() -> sf::Vector2f {
        float midW = static_cast<float>(WIN_W) - TOOLBAR_W - COLOR_W;
        float cw   = static_cast<float>(canvas.getWidth()  * canvas.getPixelSize());
        float ch   = static_cast<float>(canvas.getHeight() * canvas.getPixelSize());
        return { TOOLBAR_W + (midW - cw) / 2.f, (static_cast<float>(WIN_H) - ch) / 2.f };
    };
    sf::Vector2f canvasOffset = centeredOffset();

    // main loop
    const sf::Vector2f LOGICAL_SIZE(static_cast<float>(WIN_W), static_cast<float>(WIN_H));
    sf::View fixedView(sf::FloatRect({0.f, 0.f}, LOGICAL_SIZE));

    auto applyLetterbox = [&]() {
        sf::Vector2u winSize = window.getSize();
        float winRatio  = static_cast<float>(winSize.x) / static_cast<float>(winSize.y);
        float viewRatio = LOGICAL_SIZE.x / LOGICAL_SIZE.y;
        float sizeX = 1.f, sizeY = 1.f;
        float posX  = 0.f, posY  = 0.f;
        if (winRatio > viewRatio) {
            sizeX = viewRatio / winRatio;
            posX  = (1.f - sizeX) / 2.f;
        } else {
            sizeY = winRatio / viewRatio;
            posY  = (1.f - sizeY) / 2.f;
        }
        fixedView.setViewport(sf::FloatRect({posX, posY}, {sizeX, sizeY}));
        window.setView(fixedView);
    };
    applyLetterbox();

    // maps physical pixel mouse position to logical coordinate space
    auto mapMouse = [&](sf::Vector2i physPx) -> sf::Vector2f {
        return window.mapPixelToCoords(physPx, fixedView);
    };

    bool         painting        = false;
    bool         panning         = false;
    sf::Vector2f panAnchor;
    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {

            if (event->is<sf::Event::Closed>())
                window.close();

            // reapply letterbox when window is resized
            if (event->is<sf::Event::Resized>())
                applyLetterbox();

            // scroll to zoom towards cursor
            if (const auto* e = event->getIf<sf::Event::MouseWheelScrolled>()) {
                sf::Vector2f mouseLogical = mapMouse(e->position);
                int oldPs = canvas.getPixelSize();
                int newPs = (e->delta > 0) ? std::min(oldPs + 1, 64) : std::max(oldPs - 1, 1);
                if (newPs != oldPs) {
                    sf::Vector2f canvasPixel = (mouseLogical - canvasOffset) / static_cast<float>(oldPs);
                    canvas.setPixelSize(newPs);
                    canvasOffset = mouseLogical - canvasPixel * static_cast<float>(newPs);
                }
            }

            // Middle-mouse pan
            if (const auto* e = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (e->button == sf::Mouse::Button::Middle) {
                    panning   = true;
                    panAnchor = mapMouse(e->position);
                }
            }
            if (const auto* e = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (e->button == sf::Mouse::Button::Middle)
                    panning = false;
            }
            if (const auto* e = event->getIf<sf::Event::MouseMoved>()) {
                if (panning) {
                    sf::Vector2f cur = mapMouse(e->position);
                    canvasOffset += cur - panAnchor;
                    panAnchor = cur;
                }
            }

            // handle dialog first
            if (sizeDialog.isOpen()) {
                sizeDialog.handleEvent(*event, window, fixedView);
                int nw, nh;
                if (sizeDialog.consumeConfirm(nw, nh))
                    canvas.resize(nw, nh);
                continue;
            }

            toolbar.handleEvent(*event, window, fixedView);
            colorPicker.handleEvent(*event, window, fixedView);

            if (toolbar.consumeResizeRequest())
                sizeDialog.open(canvas.getWidth(), canvas.getHeight());
            if (toolbar.consumeClearRequest())
                canvas.clear(sf::Color::White);

            // home key re-centers the canvas
            if (const auto* e = event->getIf<sf::Event::KeyPressed>()) {
                if (e->code == sf::Keyboard::Key::Home)
                    canvasOffset = centeredOffset();
            }

            if (const auto* e = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (e->button == sf::Mouse::Button::Left) {
                    sf::Vector2f mp = mapMouse(e->position);
                    if (!colorPicker.contains(mp) && mp.x > TOOLBAR_W)
                        painting = true;
                }
            }
            if (event->is<sf::Event::MouseButtonReleased>())
                painting = false;
        }

        // continuous painting while LMB is held
        if (painting && !sizeDialog.isOpen()) {
            sf::Vector2f mp = mapMouse(sf::Mouse::getPosition(window));
            if (!colorPicker.contains(mp) && mp.x > TOOLBAR_W) {
                canvas.paint(mp, canvasOffset,
                             colorPicker.getColor(),
                             toolbar.getBrushSize(),
                             toolbar.getBrushShape());
            }
        }

        // render
        window.clear(sf::Color(30, 30, 30));

        // Canvas area background
        sf::RectangleShape mid(sf::Vector2f(WIN_W - TOOLBAR_W - COLOR_W,
                                            static_cast<float>(WIN_H)));
        mid.setPosition({TOOLBAR_W, 0.f});
        mid.setFillColor(sf::Color(45, 45, 48));
        window.draw(mid);

        canvas.draw(window, canvasOffset);
        toolbar.draw(window);

        // Right panel background
        sf::RectangleShape rPanel({COLOR_W, static_cast<float>(WIN_H)});
        rPanel.setPosition({WIN_W - COLOR_W, 0.f});
        rPanel.setFillColor(sf::Color(40, 42, 44));
        window.draw(rPanel);

        colorPicker.draw(window);

        // Status bar
        {
            sf::RectangleShape bar({static_cast<float>(WIN_W), 20.f});
            bar.setPosition({0.f, static_cast<float>(WIN_H) - 20.f});
            bar.setFillColor(sf::Color(25, 25, 25));
            window.draw(bar);

            auto mp  = mapMouse(sf::Mouse::getPosition(window));
            auto off = canvasOffset;
            int cx   = static_cast<int>(std::floor((mp.x - off.x) / canvas.getPixelSize()));
            int cy   = static_cast<int>(std::floor((mp.y - off.y) / canvas.getPixelSize()));

            std::string status =
                "Canvas: " + std::to_string(canvas.getWidth()) +
                "x"        + std::to_string(canvas.getHeight()) +
                "   Zoom: " + std::to_string(canvas.getPixelSize()) + "px" +
                "   Cursor: (" + std::to_string(cx) + ", " + std::to_string(cy) + ")" +
                "   Scroll=zoom  MMB=pan  Home=re-center";

            sf::Text statusTxt(font, status, 11);
            statusTxt.setFillColor(sf::Color(140, 140, 140));
            statusTxt.setPosition({6.f, static_cast<float>(WIN_H) - 17.f});
            window.draw(statusTxt);
        }

        sizeDialog.draw(window);
        window.display();
    }
    return 0;
}