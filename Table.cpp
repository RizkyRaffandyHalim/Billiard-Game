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
    felt.setOutlineColor(sf::Color(139, 69, 19)); 
}

    // SetupCushions
void Table::setupCushions(float thickness, sf::Color color) {
    sf::Vector2f pos = felt.getPosition();
    sf::Vector2f size = felt.getSize();

    // Cushion Kiri
    cushions[0].setSize(sf::Vector2f(thickness, size.y));
    cushions[0].setPosition(pos.x, pos.y);

    // Cushion Atas
    cushions[1].setSize(sf::Vector2f(size.x, thickness));
    cushions[1].setPosition(pos.x, pos.y);

    // Cushion Kanan
    cushions[2].setSize(sf::Vector2f(thickness, size.y));
    cushions[2].setPosition(pos.x + size.x - thickness, pos.y);

    // Cushion Bawah
    cushions[3].setSize(sf::Vector2f(size.x, thickness));
    cushions[3].setPosition(pos.x, pos.y + size.y - thickness);

    for(int i = 0; i < 4; i++) {
        cushions[i].setFillColor(color);
    }
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
        
        // Draw cushions if no texture
        for(int i = 0; i < 4; i++) {
            window.draw(cushions[i]);
        }
    }
}