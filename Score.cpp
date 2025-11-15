#include "Score.h"

Score::Score() : player1Score(0), player2Score(0) {
    // Konstruktor default: font kosong, text kosong
    sf::Font dummyFont;  // Font kosong, text tidak akan tampil tapi tidak crash
    text.setFont(dummyFont);
    text.setCharacterSize(24);
    text.setFillColor(sf::Color::White);
    text.setPosition(10, 10);
    text.setString("");  // Kosongkan text
}

Score::Score(sf::Font& font) {
    player1Score = 0;
    player2Score = 0;
    text.setFont(font);
    text.setCharacterSize(24);
    text.setFillColor(sf::Color::White);
    text.setPosition(10, 10);
}

void Score::updateScore(int player) {
    if (player == 1) player1Score++;
    else player2Score++;
    text.setString("Player 1: " + std::to_string(player1Score) + " | Player 2: " + std::to_string(player2Score));
}

void Score::draw(sf::RenderWindow& window) {
    window.draw(text);
}
