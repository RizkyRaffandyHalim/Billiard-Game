#include "Table.h"

Table::Table(int width, int height, float border) {
    borderThickness = border;
    felt.setSize({float(width-2*border), float(height-2*border)});
    felt.setPosition(border, border);
    felt.setFillColor(sf::Color(0, 50, 80)); 
    felt.setOutlineThickness(border);
    felt.setOutlineColor(sf::Color(70, 35, 10)); 
}

void Table::draw(sf::RenderWindow& window){
    window.draw(felt);
}