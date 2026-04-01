#include <SFML/Graphics.hpp>
#include "Canvas.h"
#include "Toolbar.h"
#include "ColorPicker.h"
#include "CanvasSizeDialog.h"
#include <cmath>

static constexpr unsigned WIN_W     = 1920;
static constexpr unsigned WIN_H     = 1080;
static constexpr float    TOOLBAR_W = 200.f;
static constexpr float    COLOR_W   = 220.f;
static constexpr int      INIT_ZOOM = 16;

int main() {
    sf::RenderWindow window( // no window resizing because of issues with the grid I wasn't able to fix
        sf::VideoMode({WIN_W, WIN_H}),
        "Pixel Art Editor",
        sf::Style::None); // borderless window
    window.setFramerateLimit(360);

    // font
    sf::Font font;
    bool fontLoaded =
        font.openFromFile("fonts/AsepriteFont.ttf");
    (void)fontLoaded;

    // subelements
    Canvas canvas(32, 32, INIT_ZOOM);
    canvas.clear(sf::Color::White);

    Toolbar          toolbar({0.f, 0.f}, TOOLBAR_W, &font);
    ColorPicker      colorPicker({WIN_W - COLOR_W + 10.f, 52.f}, COLOR_W - 20.f);
    CanvasSizeDialog sizeDialog(&font);

    auto centeredOffset = [&]() -> sf::Vector2f {
        float midW = WIN_W - TOOLBAR_W - COLOR_W;
        float cw   = static_cast<float>(canvas.getWidth()  * canvas.getPixelSize());
        float ch   = static_cast<float>(canvas.getHeight() * canvas.getPixelSize());
        return { TOOLBAR_W + (midW - cw) / 2.f,
                 (WIN_H - ch) / 2.f };
    };
    sf::Vector2f canvasOffset = centeredOffset();

    bool         painting  = false;
    bool         erasing   = false;
    bool         panning   = false;
    sf::Vector2f panAnchor;

    auto mousePosF = [&]() -> sf::Vector2f {
        auto p = sf::Mouse::getPosition(window);
        return { static_cast<float>(p.x), static_cast<float>(p.y) };
    };
    auto evPosF = [](sf::Vector2i p) -> sf::Vector2f {
        return { static_cast<float>(p.x), static_cast<float>(p.y) };
    };

    window.setView(window.getDefaultView());

    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {

            if (event->is<sf::Event::Closed>())
                window.close();

            // zoom towards cursor
            if (const auto* e = event->getIf<sf::Event::MouseWheelScrolled>()) {
                sf::Vector2f mp = evPosF(e->position);
                int oldPs = canvas.getPixelSize();
                int newPs = (e->delta > 0) ? std::min(oldPs + 1, 64) : std::max(oldPs - 1, 1);
                if (newPs != oldPs) {
                    sf::Vector2f px = (mp - canvasOffset) / static_cast<float>(oldPs);
                    canvas.setPixelSize(newPs);
                    canvasOffset = mp - px * static_cast<float>(newPs);
                }
            }

            // Middle-mouse pan
            if (const auto* e = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (e->button == sf::Mouse::Button::Middle) {
                    panning   = true;
                    panAnchor = evPosF(e->position);
                }
            }
            if (const auto* e = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (e->button == sf::Mouse::Button::Middle)
                    panning = false;
            }
            if (const auto* e = event->getIf<sf::Event::MouseMoved>()) {
                if (panning) {
                    sf::Vector2f cur = evPosF(e->position);
                    canvasOffset += cur - panAnchor;
                    panAnchor = cur;
                }
            }

            // dialog is modal
            if (sizeDialog.isOpen()) {
                sizeDialog.handleEvent(*event, window, window.getDefaultView());
                int nw, nh;
                if (sizeDialog.consumeConfirm(nw, nh))
                    canvas.resize(nw, nh);
                continue;
            }

            toolbar.handleEvent(*event, window, window.getDefaultView());
            colorPicker.handleEvent(*event, window, window.getDefaultView());

            if (toolbar.consumeResizeRequest())
                sizeDialog.open(canvas.getWidth(), canvas.getHeight());
            if (toolbar.consumeClearRequest())
                canvas.clear(sf::Color::White);

            if (const auto* e = event->getIf<sf::Event::KeyPressed>()) {
                if (e->code == sf::Keyboard::Key::Home)
                    canvasOffset = centeredOffset();
            }

            if (const auto* e = event->getIf<sf::Event::MouseButtonPressed>()) {
                sf::Vector2f mp = evPosF(e->position);
                if (e->button == sf::Mouse::Button::Left) {
                    if (!colorPicker.contains(mp) && mp.x > TOOLBAR_W)
                        painting = true;
                }
                if (e->button == sf::Mouse::Button::Right) {
                    if (mp.x > TOOLBAR_W && mp.x < WIN_W - COLOR_W)
                        erasing = true;
                }
            }
            if (event->is<sf::Event::MouseButtonReleased>()) {
                painting = false;
                erasing  = false;
            }
        }

        if (!sizeDialog.isOpen()) {
            sf::Vector2f mp = mousePosF();
            if (painting && !colorPicker.contains(mp) && mp.x > TOOLBAR_W)
                canvas.paint(mp, canvasOffset,
                             colorPicker.getColor(),
                             toolbar.getBrushSize(),
                             toolbar.getBrushShape());
            if (erasing && mp.x > TOOLBAR_W && mp.x < WIN_W - COLOR_W)
                canvas.paint(mp, canvasOffset,
                             sf::Color::White,
                             toolbar.getBrushSize(),
                             toolbar.getBrushShape());
        }

        // render
        window.clear(sf::Color(18, 18, 22));

        sf::RectangleShape mid({WIN_W - TOOLBAR_W - COLOR_W, static_cast<float>(WIN_H)});
        mid.setPosition({TOOLBAR_W, 0.f});
        mid.setFillColor(sf::Color(38, 38, 46));
        window.draw(mid);

        canvas.draw(window, canvasOffset);
        toolbar.draw(window);
        colorPicker.draw(window);

        // "COLOR" label
        {
            sf::Text colorLabel(font, "COLOR", 30);
            colorLabel.setFillColor(sf::Color(110, 112, 120));
            colorLabel.setPosition({WIN_W - COLOR_W + 14.f, 12.f});
            window.draw(colorLabel);
        }

        // status bar
        {
            sf::RectangleShape bar({static_cast<float>(WIN_W), 30.f});
            bar.setPosition({0.f, WIN_H - 30.f});
            bar.setFillColor(sf::Color(22, 22, 27));
            window.draw(bar);
            sf::RectangleShape barBorder({static_cast<float>(WIN_W), 1.f});
            barBorder.setPosition({0.f, static_cast<float>(WIN_H - 30)});
            barBorder.setFillColor(sf::Color(55, 55, 65));
            window.draw(barBorder);

            sf::Vector2f mp = mousePosF();
            int cx = static_cast<int>(std::floor((mp.x - canvasOffset.x) / canvas.getPixelSize()));
            int cy = static_cast<int>(std::floor((mp.y - canvasOffset.y) / canvas.getPixelSize()));

            std::string status =
                "Canvas: " + std::to_string(canvas.getWidth()) + "x" +
                std::to_string(canvas.getHeight()) +
                "   Zoom: " + std::to_string(canvas.getPixelSize()) + "px" +
                "   Cursor: (" + std::to_string(cx) + ", " + std::to_string(cy) + ")" +
                "   Scroll = Zoom   MMB = Pan   RMB = Erase";

            sf::Text statusTxt(font, status, 30);
            statusTxt.setFillColor(sf::Color(110, 112, 120));
            auto tb = statusTxt.getLocalBounds();
            statusTxt.setPosition({10.f, WIN_H - 30.f + (30.f - tb.size.y) / 2.f - tb.position.y});
            window.draw(statusTxt);
        }

        sizeDialog.draw(window);
        window.display();
    }
    return 0;
}