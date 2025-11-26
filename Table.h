#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>

class Table {
public:
    sf::RectangleShape felt;
    float borderThickness;
    sf::Texture tableTexture;
    sf::Sprite tableSprite;
    bool useTexture;

    Table(int width, int height, float border);
    void draw(sf::RenderWindow& window);
};