#include "Table.h"

Table::Table(int width, int height, float border) {
    borderThickness = border;
    useTexture = false;

    // SETUP VISUAL (GAMBAR)
    if (tableTexture.loadFromFile("assets/Table.png")) {
        useTexture = true;
        tableTexture.setSmooth(true);
        tableSprite.setTexture(tableTexture);
    } else {
        std::cerr << "Warning: assets/Table.png not found. Using default colors." << std::endl;
    }

    // SETUP LOGIKA (HITBOX)
    felt.setSize({float(width - 2 * border), float(height - 2 * border)});
    felt.setPosition(border, border);
    
    felt.setFillColor(sf::Color(0, 50, 80)); 
    felt.setOutlineThickness(border);
    felt.setOutlineColor(sf::Color(70, 35, 10)); 
}

void Table::draw(sf::RenderWindow& window){
    if (useTexture) {
        sf::Vector2f feltPos = felt.getPosition();
        sf::Vector2f feltSize = felt.getSize();

        float totalWidth = feltSize.x + (borderThickness * 2.f);
        float totalHeight = feltSize.y + (borderThickness * 2.f);

        tableSprite.setPosition(feltPos.x - borderThickness, feltPos.y - borderThickness);

        sf::Vector2u texSize = tableTexture.getSize();
        float scaleX = totalWidth / (float)texSize.x;
        float scaleY = totalHeight / (float)texSize.y;

        tableSprite.setScale(scaleX, scaleY);

        window.draw(tableSprite);
    } else {
        window.draw(felt);
    }
}