#include <raylib.h>
#include <deque>
#include <stdlib.h>
#include <fstream>

using namespace std;

Color custom = {220, 220, 220, 255};

const int LebarWindow = 860;
const int LebarLayar = 600;
const int TinggiLayar = 600;
const int UkuranGrid = 20;
const int LebarGrid = LebarLayar / UkuranGrid;
const int TinggiGrid = TinggiLayar / UkuranGrid;

enum tombol {UP = 0, DOWN, RIGHT, LEFT};

enum Jenisbuah {BIASA, BONUS, RACUN, MINSPD, PLUSSPD};

struct UkuranUlar {
    int x;
    int y;
};

struct Ular {
    deque<UkuranUlar> tubuh;
    tombol arah;
};

struct Makanan {
    int x;
    int y;
    Jenisbuah jenisbuah;
};

int readHighScore(){
    int highScore = 0;
    ifstream hscore ("HighScore.txt");
    hscore >> highScore;
    hscore.close();
    return highScore;
}

void writeHighScore(int score, int highscore){
    if(score > highscore){
        ofstream hscore ("HighScore.txt");
        hscore << score;
        hscore.close();
    }
}
Makanan BuatMakanan(const Ular& ular) {
    Makanan Buah;
    bool valid = false;
    const int borderWidth = 1;
    int persentase;

    while (!valid) {
        Buah.x = borderWidth + rand() % (LebarGrid - 2 * borderWidth);
        Buah.y = borderWidth + rand() % (TinggiGrid - 2 * borderWidth);
        persentase = rand() % 100;

        if ( persentase < 70 ) Buah.jenisbuah = BIASA;
        else if (persentase < 75) Buah.jenisbuah = MINSPD;
        else if (persentase < 80) Buah.jenisbuah = RACUN;
        else if (persentase < 90) Buah.jenisbuah = BONUS;
        else if (persentase < 100) Buah.jenisbuah = PLUSSPD;

        valid = true;
        for (const UkuranUlar& segmen : ular.tubuh) {
            if (Buah.x == segmen.x && Buah.y == segmen.y) {
                valid = false;
                break;
            }
        }
    }
    return Buah;
}

bool CekTabrakTubuh(const Ular& ular) {
    for (size_t i = 1; i < ular.tubuh.size(); i++) {
        if (ular.tubuh[0].x == ular.tubuh[i].x && ular.tubuh[0].y == ular.tubuh[i].y) {
            return true;
        }
    }
    return false;
}

void ResetGame(Ular& ular, Makanan& buah, bool& gameover, int& score, float& kecepatanUlar) {
    ular.tubuh.clear();
    for (size_t i = 0; i < 3; i++) {
        ular.tubuh.push_back({LebarGrid / 2, TinggiGrid / 2});
    }
    ular.arah = RIGHT;

    buah = BuatMakanan(ular);

    gameover = false;
    score = 0;
    kecepatanUlar = 8.5f;
}

int main() {
    InitWindow(LebarWindow, TinggiLayar, "ULAR VEGAN");
    SetTargetFPS(60);

    Texture2D Grass = LoadTexture("texture/rumput.png");
    Texture2D AppleBonus = LoadTexture("texture/apelBonus.png");
    Texture2D apple = LoadTexture("texture/apel.png");
    Texture2D HeadRight = LoadTexture("texture/kepalakanan.png");
    Texture2D HeadLeft = LoadTexture("texture/kepalakiri.png");
    Texture2D HeadUp = LoadTexture("texture/kepalaatas.png");
    Texture2D HeadDown = LoadTexture("texture/kepalabawah.png");
    Texture2D Body = LoadTexture("texture/badan.png");

    Texture2D ArahKepala[] = {HeadUp, HeadDown, HeadRight, HeadLeft};

    Texture2D TipeApel[] = {apple, AppleBonus};
    Color TipeWarnaApel[] = {RED, PURPLE, WHITE, SKYBLUE, GOLD};    
    int nilaiAryApel;
    int nilaiAryWarnaApel;

    InitAudioDevice();

    Sound eat = LoadSound("sound/eat.mp3");
    Sound end = LoadSound("sound/gameover.mp3");

    Ular ular;
    Makanan buah = BuatMakanan(ular);
    bool gameover = false, haveTempSpd = false;
    int score = 0, highScore;
    float kecepatanNormal = 10.0f;
    float kecepatanSementara;
    float waktuAkumulasi = 0.0f;

    for(size_t i = 0; i < 3; i++) {
        ular.tubuh.push_back({LebarGrid / 2, TinggiGrid / 2});
    }

    ular.arah = RIGHT;

    while (WindowShouldClose() == false) {
        float dt = GetFrameTime();
        waktuAkumulasi += dt;

        highScore = readHighScore();
        writeHighScore(score, highScore);

        tombol inputSementara = ular.arah;
        if ((IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) && ular.arah != DOWN) inputSementara = UP;
        if ((IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) && ular.arah != UP) inputSementara = DOWN;
        if ((IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) && ular.arah != LEFT) inputSementara = RIGHT;
        if ((IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) && ular.arah != RIGHT) inputSementara = LEFT;
        ular.arah = inputSementara;

        float kecepatanUlar = haveTempSpd ? kecepatanSementara : kecepatanNormal;

        float intervalGerakan = 1.0f / kecepatanUlar;
        if (waktuAkumulasi >= intervalGerakan) {
            waktuAkumulasi -= intervalGerakan; 

            if (!gameover) {
                UkuranUlar kepala = ular.tubuh.front();
                UkuranUlar ekor = ular.tubuh.back();

                if (ular.arah == UP){
                    kepala.y--;
                }else if (ular.arah == DOWN){
                    kepala.y++;
                }else if (ular.arah == RIGHT){
                    kepala.x++;
                }else if (ular.arah == LEFT){
                    kepala.x--;
                }

                if (kepala.x == buah.x && kepala.y == buah.y) {
                    PlaySound(eat);
                    if (haveTempSpd) {
                        kecepatanSementara = kecepatanNormal;
                        haveTempSpd = false;
                    }

                    if (buah.jenisbuah == BIASA) {
                        score+=2;
                    } else if (buah.jenisbuah == BONUS) {
                        ular.tubuh.push_back(ekor);
                        score += 5;
                    } else if (buah.jenisbuah == RACUN) {
                        ular.tubuh.pop_back();
                        ular.tubuh.pop_back();
                        score -= 3;
                    } else if (buah.jenisbuah == MINSPD) {
                        kecepatanSementara = 4.0f;
                        haveTempSpd =true;
                        score+=2;
                    } else if (buah.jenisbuah == PLUSSPD) {
                        kecepatanSementara = 15.0f;
                        haveTempSpd = true;
                        score+=2;
                    }
                    buah = BuatMakanan(ular);
                } else {
                    ular.tubuh.pop_back();
                }

                ular.tubuh.push_front(kepala);

                if (kepala.x < 1 || kepala.x >= LebarGrid - 1 || kepala.y < 1 || kepala.y >= TinggiGrid - 1 || CekTabrakTubuh(ular)) {
                    PlaySound(end);
                    gameover = true;
                }
            }
        }

        BeginDrawing();
        ClearBackground(DARKGREEN);

        for (int y = 0; y < TinggiGrid; y++) {
            for (int x = 0; x < LebarGrid; x++) {
                if(y == 0 || y == TinggiGrid - 1 || x == 0 || x == TinggiGrid - 1){
                    ClearBackground(DARKGREEN);
                }else{
                    if(y % 2 == 0){
                        if(x % 2 == 0) DrawTexture(Grass, x * UkuranGrid, y * UkuranGrid, LIGHTGRAY);
                        else DrawTexture(Grass, x * UkuranGrid, y * UkuranGrid, custom);
                    }else{
                        if ( x % 2 == 0) DrawTexture(Grass, x * UkuranGrid, y * UkuranGrid, custom);
                        else DrawTexture(Grass, x * UkuranGrid, y * UkuranGrid, LIGHTGRAY);
                    }
                }
            }
        }

        for (const UkuranUlar& segmen : ular.tubuh) {
            if (segmen.x == ular.tubuh.front().x && segmen.y == ular.tubuh.front().y) {
                continue;
            } else {
                DrawTexture(Body, segmen.x * UkuranGrid, segmen.y * UkuranGrid, WHITE);
            }
        }

        DrawTexture(ArahKepala[ular.arah], ular.tubuh.front().x * UkuranGrid, ular.tubuh.front().y * UkuranGrid, WHITE);

        nilaiAryApel = 0;
        if (buah.jenisbuah == BIASA){
            nilaiAryWarnaApel = 0;
        }else if (buah. jenisbuah == RACUN){
            nilaiAryWarnaApel = 1;
        }else if (buah. jenisbuah == BONUS){
            nilaiAryWarnaApel = 2;
            nilaiAryApel = 1;
        }else if (buah. jenisbuah == MINSPD){
            nilaiAryWarnaApel = 3;
        }else if (buah. jenisbuah == PLUSSPD){
            nilaiAryWarnaApel = 4;
        }

        DrawTexture(TipeApel[nilaiAryApel], buah.x * UkuranGrid, buah.y * UkuranGrid, TipeWarnaApel[nilaiAryWarnaApel]);
        
        DrawText("VEGAN SNAKE", LebarLayar + 2 * UkuranGrid, UkuranGrid, 25, RAYWHITE);

        DrawText(TextFormat("High Score : %i", highScore), LebarLayar + UkuranGrid, 10 * UkuranGrid, 20, RAYWHITE);
        DrawText(TextFormat("Live Score : %i", score), LebarLayar + UkuranGrid, 12 * UkuranGrid, 20, RAYWHITE);

        DrawText("W/^ = Untuk Arah Atas", LebarLayar + UkuranGrid, 16 * UkuranGrid, 16, RAYWHITE);
        DrawText("A/< = Untuk Arah Kiri", LebarLayar + UkuranGrid, 17 * UkuranGrid, 16, RAYWHITE);
        DrawText("S/> = Untuk Arah Bawah", LebarLayar + UkuranGrid, 18 * UkuranGrid, 16, RAYWHITE);
        DrawText("D/v = Untuk Arah Kanan", LebarLayar + UkuranGrid, 19 * UkuranGrid, 16, RAYWHITE);

        DrawTexture(apple, LebarLayar, TinggiLayar - UkuranGrid * 4, GOLD);
        DrawText("  = Biasa + Kecepatan Dipercepat", LebarLayar + 10, TinggiLayar - (UkuranGrid * 4 - 4), 15, RAYWHITE);
        DrawTexture(apple, LebarLayar, TinggiLayar - UkuranGrid * 5, SKYBLUE);
        DrawText("  = Biasa + Kecepatan Diperlambat", LebarLayar + 10, TinggiLayar - (UkuranGrid * 5 - 4), 15, RAYWHITE);
        DrawTexture(AppleBonus, LebarLayar, TinggiLayar - UkuranGrid * 6, WHITE);
        DrawText("  = Panjang + 2, Score + 5", LebarLayar + 10, TinggiLayar - (UkuranGrid * 6 - 4), 15, RAYWHITE);
        DrawTexture(apple, LebarLayar , TinggiLayar - UkuranGrid * 7, PURPLE);
        DrawText("  = Panjang - 1, Score - 3", LebarLayar + 10, TinggiLayar - (UkuranGrid * 7 - 4), 15, RAYWHITE);
        DrawTexture(apple, LebarLayar , TinggiLayar - UkuranGrid * 8, RED);
        DrawText("  = Biasa (Panjang + 1, Score + 2)", LebarLayar + 10, TinggiLayar - (UkuranGrid * 8 - 4), 15, RAYWHITE);

        DrawLine(UkuranGrid, UkuranGrid, LebarLayar - UkuranGrid, UkuranGrid, DARKGRAY);
        DrawLine(UkuranGrid, TinggiLayar - UkuranGrid, LebarLayar - UkuranGrid, TinggiLayar - UkuranGrid, DARKGRAY);
        DrawLine(UkuranGrid, UkuranGrid, UkuranGrid, TinggiLayar - UkuranGrid, DARKGRAY);
        DrawLine(LebarLayar - UkuranGrid, UkuranGrid, LebarLayar - UkuranGrid, TinggiLayar - UkuranGrid, DARKGRAY);

        DrawLine(0, 0, LebarLayar, 0, DARKGRAY);
        DrawLine(0, TinggiLayar - 1, LebarLayar, TinggiLayar - 1, DARKGRAY);
        DrawLine(0, 0, 0, TinggiLayar, DARKGRAY);
        DrawLine(LebarLayar - 1, 0, LebarLayar - 1, TinggiLayar, DARKGRAY);

        if (gameover) {
            DrawText("GAME OVER", LebarLayar / 2 - MeasureText("GAME OVER", 40) / 2, TinggiLayar / 2 - 40, 40, RAYWHITE);
            DrawText(TextFormat("SCORE : %i", score), LebarLayar / 2 - MeasureText("SCORE :  ", 23) / 2, TinggiLayar / 2 + 10, 20, RAYWHITE);

            Rectangle retryButton = {LebarLayar / 2 - 120, TinggiLayar / 2 + 40, 100, 40};
            DrawRectangleRec(retryButton, BLUE);
            DrawText("Retry", retryButton.x + 22, retryButton.y + 10, 20, RAYWHITE);
            if (CheckCollisionPointRec(GetMousePosition(), retryButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                haveTempSpd = false;
                ResetGame(ular, buah, gameover, score, kecepatanUlar);
            }

            Rectangle exitButton = {LebarLayar / 2 + 20, TinggiLayar / 2 + 40, 100, 40};
            DrawRectangleRec(exitButton, RED);
            DrawText("Exit", exitButton.x + 33, exitButton.y + 10, 20, RAYWHITE);
            if (CheckCollisionPointRec(GetMousePosition(), exitButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                break;
            }
        }
        EndDrawing();
    }

    UnloadSound(eat);
    UnloadSound(end);

    UnloadTexture(Grass);
    UnloadTexture(AppleBonus);
    UnloadTexture(apple);
    UnloadTexture(Body);
    UnloadTexture(HeadRight);
    UnloadTexture(HeadLeft);
    UnloadTexture(HeadUp);
    UnloadTexture(HeadDown);

    CloseAudioDevice();
    CloseWindow();

    return 0;
}