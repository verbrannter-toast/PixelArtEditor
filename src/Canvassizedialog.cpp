#include "CanvasSizeDialog.h"
#include <algorithm>

CanvasSizeDialog::CanvasSizeDialog(sf::Font* font) : m_font(font) {}

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
sf::FloatRect CanvasSizeDialog::dialogRect(sf::RenderWindow& w) const {
    float dw = 480.f, dh = 320.f;
    float wx = static_cast<float>(w.getSize().x);
    float wy = static_cast<float>(w.getSize().y);
    return sf::FloatRect({ (wx - dw) / 2.f, (wy - dh) / 2.f }, { dw, dh });
}
sf::FloatRect CanvasSizeDialog::wFieldRect(sf::FloatRect d) const {
    return sf::FloatRect({d.position.x + 24.f, d.position.y + 90.f},
                         {d.size.x / 2.f - 36.f, 50.f});
}
sf::FloatRect CanvasSizeDialog::hFieldRect(sf::FloatRect d) const {
    return sf::FloatRect({d.position.x + d.size.x / 2.f + 12.f, d.position.y + 90.f},
                         {d.size.x / 2.f - 36.f, 50.f});
}
sf::FloatRect CanvasSizeDialog::okBtnRect(sf::FloatRect d) const {
    return sf::FloatRect({d.position.x + d.size.x - 130.f, d.position.y + d.size.y - 66.f},
                         {106.f, 44.f});
}
sf::FloatRect CanvasSizeDialog::cancelBtnRect(sf::FloatRect d) const {
    return sf::FloatRect({d.position.x + d.size.x - 250.f, d.position.y + d.size.y - 66.f},
                         {106.f, 44.f});
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
    sf::Text lbl(*m_font, label, 30);
    lbl.setFillColor(sf::Color(110, 112, 120));
    lbl.setPosition({r.position.x, r.position.y - 36.f});
    w.draw(lbl);

    sf::RectangleShape box({r.size.x, r.size.y});
    box.setPosition({r.position.x, r.position.y});
    box.setFillColor(sf::Color(18, 18, 22));
    box.setOutlineColor(active ? sf::Color(72, 152, 168) : sf::Color(55, 55, 65));
    box.setOutlineThickness(1.f);
    w.draw(box);

    sf::Text txt(*m_font, val + (active ? "|" : ""), 30);
    txt.setFillColor(sf::Color(188, 190, 196));
    auto tb = txt.getLocalBounds();
    txt.setPosition({r.position.x + 10.f,
                     r.position.y + (r.size.y - tb.size.y) / 2.f - tb.position.y});
    w.draw(txt);
}

void CanvasSizeDialog::drawBtn(sf::RenderWindow& w, sf::FloatRect r,
                                const std::string& label, sf::Color col) {
    sf::RectangleShape btn({r.size.x, r.size.y});
    btn.setPosition({r.position.x, r.position.y});
    btn.setFillColor(col);
    btn.setOutlineColor(sf::Color(55, 55, 65));
    btn.setOutlineThickness(1.f);
    w.draw(btn);

    sf::Text txt(*m_font, label, 30);
    txt.setFillColor(sf::Color(188, 190, 196));
    auto tb = txt.getLocalBounds();
    txt.setPosition({r.position.x + (r.size.x - tb.size.x) / 2.f - tb.position.x,
                     r.position.y + (r.size.y - tb.size.y) / 2.f - tb.position.y});
    w.draw(txt);
}

void CanvasSizeDialog::draw(sf::RenderWindow& window) {
    if (!m_open) return;

    sf::RectangleShape overlay(sf::Vector2f(window.getSize()));
    overlay.setFillColor(sf::Color(0, 0, 0, 160));
    window.draw(overlay);

    auto dlg = dialogRect(window);

    sf::RectangleShape shadow({dlg.size.x + 10.f, dlg.size.y + 10.f});
    shadow.setPosition({dlg.position.x - 5.f, dlg.position.y - 5.f});
    shadow.setFillColor(sf::Color(0, 0, 0, 100));
    window.draw(shadow);

    sf::RectangleShape box({dlg.size.x, dlg.size.y});
    box.setPosition({dlg.position.x, dlg.position.y});
    box.setFillColor(sf::Color(30, 30, 36));
    box.setOutlineColor(sf::Color(55, 55, 65));
    box.setOutlineThickness(1.f);
    window.draw(box);

    // title bar
    float titleH = 44.f;
    sf::RectangleShape titleBar({dlg.size.x, titleH});
    titleBar.setPosition({dlg.position.x, dlg.position.y});
    titleBar.setFillColor(sf::Color(22, 22, 27));
    window.draw(titleBar);
    sf::RectangleShape titleBorder({dlg.size.x, 1.f});
    titleBorder.setPosition({dlg.position.x, dlg.position.y + titleH});
    titleBorder.setFillColor(sf::Color(55, 55, 65));
    window.draw(titleBorder);

    sf::Text title(*m_font, "Resize Canvas", 36);
    title.setFillColor(sf::Color(188, 190, 196));
    auto tb = title.getLocalBounds();
    title.setPosition({dlg.position.x + 16.f,
                       dlg.position.y + (titleH - tb.size.y) / 2.f - tb.position.y});
    window.draw(title);

    drawField(window, wFieldRect(dlg), "Width",  m_wStr, m_activeField == 0);
    drawField(window, hFieldRect(dlg), "Height", m_hStr, m_activeField == 1);

    sf::Text hint(*m_font, "Tab = switch    Enter = confirm    Esc = cancel", 30);
    hint.setFillColor(sf::Color(80, 82, 90));
    hint.setPosition({dlg.position.x + 24.f, dlg.position.y + dlg.size.y - 120.f});
    window.draw(hint);

    drawBtn(window, cancelBtnRect(dlg), "Cancel", sf::Color(45, 47, 55));
    drawBtn(window, okBtnRect(dlg),     "OK",     sf::Color(45, 95, 108));
}