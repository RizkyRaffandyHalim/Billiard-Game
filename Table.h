#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>

class Table {
public:
    sf::RectangleShape felt;
    sf::RectangleShape cushions[4]; 

    float borderThickness;
    sf::Texture tableTexture;
    sf::Sprite tableSprite;
    bool useTexture;

    Table(int width, int height, float border);
    
    // Fungsi untuk setup visual cushion
    void setupCushions(float thickness, sf::Color color);
    
    void draw(sf::RenderWindow& window);
};