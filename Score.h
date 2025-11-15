#ifndef SCORE_H
#define SCORE_H

#include <SFML/Graphics.hpp>

class Score {
public:
    int player1Score;
    int player2Score;
    sf::Text text;

    Score();  // Konstruktor default baru
    Score(sf::Font& font);
    void updateScore(int player);
    void draw(sf::RenderWindow& window);
};

#endif
