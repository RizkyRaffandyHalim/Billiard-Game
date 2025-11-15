Langkah - langkah setelah clone repo:

1. Crtl + Shift + P
2. Tasks: Configure default build task
3. build SFML (billiard game)
4. Crtl + Shift + P
5. Task: Run Task
6. Build SFML
7. Compile : g++ main.cpp Ball.cpp Table.cpp Hole.cpp CueStick.cpp -I C:/SFML/SFML-2.6.1/include -L C:/SFML/SFML-2.6.1/lib -lsfml-graphics -lsfml-window -lsfml-system -o billiard.exe
8. .\billiard.exe
