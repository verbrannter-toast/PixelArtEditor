#include "CanvasSizeDialog.h"
#include <algorithm>

CanvasSizeDialog::CanvasSizeDialog(sf::Font& font) : m_font(font) {}

void CanvasSizeDialog::open(int w, int h) {
    m_wStr = std::to_string(w);
    m_hStr = std::to_string(h);
    m_activeField = 0;
    m_confirmed   = false;
    m_open        = true;
}
void CanvasSizeDialog::close() { m_open = false; }

bool CanvasSizeDialog::consumeConfirm(int& outW, int& outH) {
    if (!m_confirmed) return false;
    m_confirmed = false;
    outW = m_outW;
    outH = m_outH;
    return true;
}

// layout
sf::FloatRect CanvasSizeDialog::dialogRect(sf::RenderWindow& /*w*/) const {
    // Use fixed logical size, not physical window size
    constexpr float LOGICAL_W = 1100.f, LOGICAL_H = 750.f;
    float dw = 280.f, dh = 190.f;
    return sf::FloatRect(
        {(LOGICAL_W - dw)/2.f, (LOGICAL_H - dh)/2.f},
        {dw, dh});
}
sf::FloatRect CanvasSizeDialog::wFieldRect(sf::FloatRect d) const {
    return sf::FloatRect({d.position.x+16, d.position.y+48},
                         {d.size.x/2.f - 24, 30});
}
sf::FloatRect CanvasSizeDialog::hFieldRect(sf::FloatRect d) const {
    return sf::FloatRect({d.position.x + d.size.x/2.f + 8, d.position.y+48},
                         {d.size.x/2.f - 24, 30});
}
sf::FloatRect CanvasSizeDialog::okBtnRect(sf::FloatRect d) const {
    return sf::FloatRect({d.position.x + d.size.x/2.f + 8, d.position.y + d.size.y - 46},
                         {80, 30});
}
sf::FloatRect CanvasSizeDialog::cancelBtnRect(sf::FloatRect d) const {
    return sf::FloatRect({d.position.x + d.size.x/2.f - 88, d.position.y + d.size.y - 46},
                         {80, 30});
}

// events
void CanvasSizeDialog::handleEvent(const sf::Event& event, sf::RenderWindow& window, const sf::View& view) {
    if (!m_open) return;

    auto dlg = dialogRect(window);

    if (const auto* e = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (e->button == sf::Mouse::Button::Left) {
            sf::Vector2f mp = window.mapPixelToCoords(e->position, view);

            if (wFieldRect(dlg).contains(mp))      m_activeField = 0;
            else if (hFieldRect(dlg).contains(mp)) m_activeField = 1;

            auto confirm = [&]() {
                try { m_outW = std::clamp(std::stoi(m_wStr), 1, 512); } catch(...) { m_outW = 32; }
                try { m_outH = std::clamp(std::stoi(m_hStr), 1, 512); } catch(...) { m_outH = 32; }
                m_confirmed = true;
                m_open      = false;
            };

            if (okBtnRect(dlg).contains(mp))     confirm();
            if (cancelBtnRect(dlg).contains(mp)) m_open = false;
        }
    }

    if (const auto* e = event.getIf<sf::Event::KeyPressed>()) {
        switch (e->code) {
            case sf::Keyboard::Key::Tab:
                m_activeField ^= 1;
                break;
            case sf::Keyboard::Key::Enter:
                try { m_outW = std::clamp(std::stoi(m_wStr), 1, 512); } catch(...) { m_outW = 32; }
                try { m_outH = std::clamp(std::stoi(m_hStr), 1, 512); } catch(...) { m_outH = 32; }
                m_confirmed = true;
                m_open      = false;
                break;
            case sf::Keyboard::Key::Escape:
                m_open = false;
                break;
            case sf::Keyboard::Key::Backspace: {
                auto& s = (m_activeField == 0) ? m_wStr : m_hStr;
                if (!s.empty()) s.pop_back();
                break;
            }
            default: break;
        }
    }

    if (const auto* e = event.getIf<sf::Event::TextEntered>()) {
        char c = static_cast<char>(e->unicode);
        if (c >= '0' && c <= '9') {
            auto& s = (m_activeField == 0) ? m_wStr : m_hStr;
            if (s.size() < 3) s += c;
        }
    }
}

// draw
void CanvasSizeDialog::drawField(sf::RenderWindow& w, sf::FloatRect r,
                                  const std::string& label, const std::string& val, bool active) {
    sf::Text lbl(m_font, label, 11);
    lbl.setFillColor(sf::Color(160,160,160));
    lbl.setPosition({r.position.x, r.position.y - 16.f});
    w.draw(lbl);

    sf::RectangleShape box({r.size.x, r.size.y});
    box.setPosition({r.position.x, r.position.y});
    box.setFillColor(sf::Color(30, 32, 34));
    box.setOutlineColor(active ? sf::Color(80, 140, 220) : sf::Color(80, 83, 85));
    box.setOutlineThickness(active ? 2.f : 1.f);
    w.draw(box);

    sf::Text txt(m_font, val + (active ? "|" : ""), 14);
    txt.setFillColor(sf::Color::White);
    auto tb = txt.getLocalBounds();
    txt.setPosition({r.position.x + 6.f,
                     r.position.y + (r.size.y - tb.size.y)/2.f - tb.position.y});
    w.draw(txt);
}

void CanvasSizeDialog::drawBtn(sf::RenderWindow& w, sf::FloatRect r,
                                const std::string& label, sf::Color col) {
    sf::RectangleShape btn({r.size.x, r.size.y});
    btn.setPosition({r.position.x, r.position.y});
    btn.setFillColor(col);
    btn.setOutlineColor(sf::Color(0,0,0,60));
    btn.setOutlineThickness(1.f);
    w.draw(btn);

    sf::Text txt(m_font, label, 13);
    txt.setFillColor(sf::Color::White);
    auto tb = txt.getLocalBounds();
    txt.setPosition({r.position.x + (r.size.x - tb.size.x)/2.f - tb.position.x,
                     r.position.y + (r.size.y - tb.size.y)/2.f - tb.position.y});
    w.draw(txt);
}

void CanvasSizeDialog::draw(sf::RenderWindow& window) {
    if (!m_open) return;

    sf::RectangleShape overlay(sf::Vector2f(1100.f, 750.f));
    overlay.setFillColor(sf::Color(0, 0, 0, 140));
    window.draw(overlay);

    auto dlg = dialogRect(window);
    sf::RectangleShape box({dlg.size.x, dlg.size.y});
    box.setPosition({dlg.position.x, dlg.position.y});
    box.setFillColor(sf::Color(50, 52, 55));
    box.setOutlineColor(sf::Color(70, 73, 76));
    box.setOutlineThickness(1.f);
    window.draw(box);

    sf::Text title(m_font, "Canvas Size", 15);
    title.setFillColor(sf::Color(220, 220, 220));
    title.setPosition({dlg.position.x + 16.f, dlg.position.y + 14.f});
    window.draw(title);

    drawField(window, wFieldRect(dlg), "Width",  m_wStr, m_activeField == 0);
    drawField(window, hFieldRect(dlg), "Height", m_hStr, m_activeField == 1);

    sf::Text hint(m_font, "1-512 px  |  Tab = switch field  |  Enter = confirm", 10);
    hint.setFillColor(sf::Color(120,120,120));
    hint.setPosition({dlg.position.x + 16.f, dlg.position.y + dlg.size.y - 68.f});
    window.draw(hint);

    drawBtn(window, cancelBtnRect(dlg), "Cancel", sf::Color(70, 73, 76));
    drawBtn(window, okBtnRect(dlg),     "OK",     sf::Color(60, 120, 200));
}