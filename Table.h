#pragma once
#include <SFML/Graphics.hpp>

class Table {
public:
    sf::RectangleShape felt;
    float borderThickness;

    Table(int width, int height, float border);

    void draw(sf::RenderWindow& window);
};
