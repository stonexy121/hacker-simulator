#include "types.hpp"
#include <cmath>
#include <cstdlib>
#include <algorithm>

// Глобальные переменные визуализации
Font gFont;
float gTime = 0, gPulse = 0, gGlitch = 0, gShake = 0;
std::vector<Particle> gParts;
std::vector<Rain> gRain;
std::vector<float> gMatrix;

void InitVisuals() {
    gRain.clear();
    for (int i = 0; i < 300; i++) {
        gRain.push_back({(float)(rand() % W), (float)(rand() % H), 
                         300.f + rand() % 400, 15.f + rand() % 20});
    }
    gMatrix.resize(W / 12);
    for (auto& m : gMatrix) m = (float)(rand() % H);
}

void UpdateVisuals(float dt) {
    gTime += dt;
    gPulse = 0.5f + 0.5f * sinf(gTime * 2);
    if (gGlitch > 0) gGlitch -= dt;
    if (gShake > 0) gShake *= 0.9f;
    
    for (auto& r : gRain) {
        r.y += r.spd * 0.5f * dt;
        r.x += 20 * dt;
        if (r.y > H) { r.y = -r.len; r.x = (float)(rand() % W); }
    }
    
    for (auto& m : gMatrix) {
        m += 120 * dt;
        if (m > H + 50) m = -50;
    }
    
    for (auto& p : gParts) {
        p.life -= dt;
        p.p.x += p.v.x * dt;
        p.p.y += p.v.y * dt;
        p.v.y += 150 * dt;
    }
    gParts.erase(std::remove_if(gParts.begin(), gParts.end(),
        [](const Particle& p) { return p.life <= 0; }), gParts.end());
}

void SpawnParticles(Vector2 pos, Color c, int n) {
    for (int i = 0; i < n; i++) {
        gParts.push_back({pos, {(float)(rand() % 300 - 150), (float)(rand() % 300 - 200)},
                          c, 0.5f + (rand() % 50) / 100.f, 2.f + rand() % 4});
    }
}

void TriggerGlitch(float d) { gGlitch = d; }
void TriggerShake(float i) { gShake = i; }

void DrawBackground() {
    ClearBackground(C_BG);
    // Матрица
    for (int i = 0; i < (int)gMatrix.size(); i++) {
        float y = gMatrix[i];
        for (int j = 0; j < 12; j++) {
            float cy = y - j * 18;
            if (cy > 0 && cy < H) {
                char s[2] = {(char)('0' + rand() % 10), 0};
                DrawText(s, i * 12, (int)cy, 12, 
                    {0, (unsigned char)(180 + rand() % 75), 0, (unsigned char)(255 - j * 20)});
            }
        }
    }
    // Сетка
    for (int x = 0; x < W; x += 60) DrawLine(x, 0, x, H, {20, 40, 60, 40});
    for (int y = 0; y < H; y += 60) DrawLine(0, y, W, y, {20, 40, 60, 40});
}

void DrawCity() {
    // Здания
    for (int i = 0; i < 15; i++) {
        int bx = i * 90, bh = 200 + rand() % 350, by = H - bh;
        DrawRectangle(bx, by, 80, bh, {(unsigned char)(15 + rand() % 10), 
            (unsigned char)(20 + rand() % 10), (unsigned char)(35 + rand() % 10), 255});
        // Окна
        for (int wy = by + 20; wy < H - 20; wy += 25) {
            for (int wx = bx + 10; wx < bx + 70; wx += 20) {
                if (rand() % 3) {
                    Color wc = sinf(gTime + wx * 0.1f) > 0.3f ? 
                        Color{255, 240, 180, 200} : Color{20, 30, 40, 255};
                    DrawRectangle(wx, wy, 12, 15, wc);
                }
            }
        }
    }
    // Дождь
    for (auto& r : gRain) {
        DrawLine((int)r.x, (int)r.y, (int)(r.x - 3), (int)(r.y + r.len), {150, 180, 220, 80});
    }
}

void DrawNeonText(const char* t, int x, int y, int s, Color c) {
    // Свечение
    DrawTextEx(gFont, t, {(float)x - 1, (float)y - 1}, (float)s, 1, {c.r, c.g, c.b, 60});
    DrawTextEx(gFont, t, {(float)x + 1, (float)y + 1}, (float)s, 1, {c.r, c.g, c.b, 60});
    DrawTextEx(gFont, t, {(float)x, (float)y}, (float)s, 1, c);
}

void DrawPanel(int x, int y, int w, int h, Color c) {
    DrawRectangle(x, y, w, h, C_PANEL);
    // Свечение рамки
    for (int i = 1; i <= 3; i++) {
        DrawRectangleLinesEx({(float)(x - i), (float)(y - i), 
            (float)(w + i * 2), (float)(h + i * 2)}, 1, {c.r, c.g, c.b, (unsigned char)(60 / i)});
    }
    DrawRectangleLinesEx({(float)x, (float)y, (float)w, (float)h}, 2, c);
}

void DrawText2(const char* t, int x, int y, int s, Color c) {
    DrawTextEx(gFont, t, {(float)x, (float)y}, (float)s, 1, c);
}

int MeasureText2(const char* t, int s) {
    return (int)MeasureTextEx(gFont, t, (float)s, 1).x;
}

void DrawEffects() {
    // Частицы
    for (auto& p : gParts) {
        float a = p.life / 0.5f;
        Color c = p.c; c.a = (unsigned char)(255 * a);
        DrawCircleV(p.p, p.size * a, c);
    }
    // Глитч
    if (gGlitch > 0) {
        for (int i = 0; i < (int)(gGlitch * 15); i++) {
            int gy = rand() % H;
            DrawRectangle(rand() % 40 - 20, gy, W, 2 + rand() % 6,
                {(unsigned char)(rand() % 2 ? 0 : 255), 
                 (unsigned char)(rand() % 2 ? 255 : 0), 255, 100});
        }
    }
    // Скан-линии
    for (int y = 0; y < H; y += 3) DrawLine(0, y, W, y, {0, 0, 0, 25});
    // Виньетка
    DrawRectangleGradientH(0, 0, 150, H, {0, 0, 0, 200}, {0, 0, 0, 0});
    DrawRectangleGradientH(W - 150, 0, 150, H, {0, 0, 0, 0}, {0, 0, 0, 200});
    DrawRectangleGradientV(0, 0, W, 100, {0, 0, 0, 200}, {0, 0, 0, 0});
    DrawRectangleGradientV(0, H - 100, W, 100, {0, 0, 0, 0}, {0, 0, 0, 200});
}
