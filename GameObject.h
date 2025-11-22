#pragma once
#include <SFML/Graphics.hpp>

class GameObject {
public:
    virtual ~GameObject() = default;

    // Fungsi virtual
    virtual void draw(sf::RenderWindow& window, sf::Font& font) = 0;
    virtual void update(float dt, float tableLeft, float tableTop, float tableRight, float tableBottom) = 0;
};