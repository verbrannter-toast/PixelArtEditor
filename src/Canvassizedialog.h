#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class CanvasSizeDialog {
public:
    explicit CanvasSizeDialog(sf::Font* font);

    void open(int currentW, int currentH);
    void close();
    bool isOpen() const { return m_open; }

    void handleEvent(const sf::Event& event, sf::RenderWindow& window, const sf::View& view);
    void draw(sf::RenderWindow& window);

    bool consumeConfirm(int& outW, int& outH);

private:
    sf::Font* m_font;
    bool      m_open = false;

    std::string m_wStr, m_hStr;
    int         m_activeField = 0;   // 0 = width, 1 = height

    bool m_confirmed = false;
    int  m_outW = 32, m_outH = 32;

    sf::FloatRect dialogRect(sf::RenderWindow& w) const;
    sf::FloatRect wFieldRect(sf::FloatRect dlg)   const;
    sf::FloatRect hFieldRect(sf::FloatRect dlg)   const;
    sf::FloatRect okBtnRect(sf::FloatRect dlg)     const;
    sf::FloatRect cancelBtnRect(sf::FloatRect dlg) const;

    void drawField(sf::RenderWindow& w, sf::FloatRect r,
                   const std::string& label, const std::string& val, bool active);
    void drawBtn(sf::RenderWindow& w, sf::FloatRect r,
                 const std::string& label, sf::Color col);
};