#include "GameManager.h"
#include "Menu.h"

int main() {
    bool appRunning = true;
    
    while(appRunning) {
        // 1. Jalankan Menu
        Menu menu(1260, 760); 
        int menuResult = menu.run(); // 0 = exit, 1 = play

        if (menuResult == 1) {
            // 2. Jalankan Game
            GameManager game;
            int gameResult = game.run(); // 0 = exit total, 1 = kembali ke menu

            if (gameResult == 0) {
                appRunning = false;
            }
        } else {
            appRunning = false;
        }
    }

    return 0;
}