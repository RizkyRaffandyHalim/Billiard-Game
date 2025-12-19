#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class Menu {
public:
    Menu(float width, float height);
    int run();

private:
    float windowWidth;
    float windowHeight;
    sf::RenderWindow window;
    sf::Font font;

    sf::Texture bgTexture;
    sf::Sprite bgSprite;
    
    sf::Texture logoTexture;
    sf::Sprite logoSprite;
    bool hasLogoImage;

    struct Button {
        sf::RectangleShape shape;
        sf::Text text;
        
        bool isHovered(sf::Vector2i mousePos, const sf::RenderWindow& w) {
            sf::Vector2f pos = w.mapPixelToCoords(mousePos);
            return shape.getGlobalBounds().contains(pos);
        }
    };

    Button btnStart;
    Button btnExit;
    sf::Text titleText;

    void setupButton(Button& btn, std::string label, float yPos);
};