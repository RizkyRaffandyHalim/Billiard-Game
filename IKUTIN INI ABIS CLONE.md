Notes: Pastikan Compiler C++ sudah terpasang, lalu instalasi Cmake versi 4.2.0 & Library SFML versi 2.6.1
Link Download Cmake: https://cmake.org/download/ (sesuaikan dengan compiler C++ & spesifikasi laptop)
Link Download SFML: https://www.sfml-dev.org/download/sfml/2.6.1/ (sesuaikan dengan compiler C++ & spesifikasi laptop)

Repository GitHub: https://github.com/RizkyRaffandyHalim/Billiard-Game.git

Langkah - langkah setelah clone repo:
1. Crtl + Shift + P
2. Tasks: Configure default build task
3. build SFML (billiard game)
4. Crtl + Shift + P
5. Task: Run Task
6. Build SFML
7. Compile : g++ main.cpp GameManager.cpp Menu.cpp Ball.cpp CueBall.cpp CueStick.cpp Table.cpp Hole.cpp Score.cpp -I C:/SFML/SFML-2.6.1/include -L C:/SFML/SFML-2.6.1/lib -lsfml-graphics -lsfml-window -lsfml-system -o billiard.exe
8. .\billiard.exe
