#include "hacking.hpp"
#include "types.hpp"
#include <cstdlib>
#include <cmath>
#include <algorithm>

void InitHackGame(HackGame& h, HackType type, int difficulty) {
    h.type = type;
    h.difficulty = difficulty;
    h.completed = false;
    h.failed = false;
    h.trace = 0;
    h.score = 0;
    
    switch (type) {
    case HackType::CodeBreak: {
        h.timeLimit = 60 - difficulty * 5;
        h.timer = h.timeLimit;
        int len = 3 + difficulty;
        h.secretCode.clear();
        for (int i = 0; i < len; i++) h.secretCode += '0' + rand() % 10;
        h.playerCode.clear();
        h.maxAttempts = 8 - difficulty;
        h.attempts = 0;
        h.history.clear();
        break;
    }
    case HackType::MemoryGrid: {
        h.timeLimit = 45;
        h.timer = h.timeLimit;
        int len = 4 + difficulty * 2;
        h.sequence.clear();
        for (int i = 0; i < len; i++) h.sequence.push_back(rand() % 9);
        h.playerSeq.clear();
        h.showPhase = 0;
        h.showTimer = 0;
        h.currentShow = 0;
        break;
    }
    case HackType::PacketRoute: {
        h.timeLimit = 40 + difficulty * 10;
        h.timer = h.timeLimit;
        h.gridW = 8 + difficulty * 2;
        h.gridH = 6 + difficulty;
        h.grid.resize(h.gridW * h.gridH, 0);
        // Стены
        int walls = h.gridW * h.gridH / 4;
        for (int i = 0; i < walls; i++) {
            int x = rand() % h.gridW, y = rand() % h.gridH;
            if ((x != 0 || y != 0) && (x != h.gridW-1 || y != h.gridH-1))
                h.grid[y * h.gridW + x] = 1;
        }
        h.grid[0] = 2; // start
        h.grid[(h.gridH-1) * h.gridW + h.gridW-1] = 3; // end
        h.packetX = 0; h.packetY = 0;
        h.targetX = h.gridW - 1; h.targetY = h.gridH - 1;
        h.playerPath.clear();
        break;
    }
    case HackType::Decrypt: {
        h.timeLimit = 30 + difficulty * 5;
        h.timer = h.timeLimit;
        const char* words[] = {"NEXUS","GHOST","GENESIS","PROTOCOL","CIPHER","MATRIX","NEURAL","SYSTEM"};
        h.decrypted = words[rand() % 8];
        h.encrypted.clear();
        h.pattern.clear();
        int shift = 1 + rand() % 5;
        for (char c : h.decrypted) {
            h.encrypted += (char)('A' + (c - 'A' + shift) % 26);
            h.pattern += '0' + shift;
        }
        h.patternPos = 0;
        break;
    }
    case HackType::Firewall: {
        h.timeLimit = 20 + difficulty * 5;
        h.timer = h.timeLimit;
        h.barriers.clear();
        h.required = 10 + difficulty * 5;
        for (int i = 0; i < h.required + 10; i++) {
            h.barriers.push_back(300 + i * 80 + rand() % 40);
        }
        h.playerY = H / 2.f;
        h.scrollSpeed = 150 + difficulty * 30;
        h.passed = 0;
        break;
    }
    }
}

void UpdateHackGame(HackGame& h, float dt) {
    if (h.completed || h.failed) return;
    
    h.timer -= dt;
    if (h.timer <= 0) { h.failed = true; return; }
    
    switch (h.type) {
    case HackType::CodeBreak: {
        // Ввод цифр
        for (int k = KEY_ZERO; k <= KEY_NINE; k++) {
            if (IsKeyPressed(k) && (int)h.playerCode.size() < (int)h.secretCode.size()) {
                h.playerCode += '0' + (k - KEY_ZERO);
            }
        }
        for (int k = KEY_KP_0; k <= KEY_KP_9; k++) {
            if (IsKeyPressed(k) && (int)h.playerCode.size() < (int)h.secretCode.size()) {
                h.playerCode += '0' + (k - KEY_KP_0);
            }
        }
        if (IsKeyPressed(KEY_BACKSPACE) && !h.playerCode.empty()) {
            h.playerCode.pop_back();
        }
        if (IsKeyPressed(KEY_ENTER) && (int)h.playerCode.size() == (int)h.secretCode.size()) {
            h.attempts++;
            int correct = 0, wrongPos = 0;
            std::string tempSecret = h.secretCode, tempPlayer = h.playerCode;
            // Точные совпадения
            for (int i = 0; i < (int)h.playerCode.size(); i++) {
                if (h.playerCode[i] == h.secretCode[i]) {
                    correct++;
                    tempSecret[i] = tempPlayer[i] = 'X';
                }
            }
            // Неточные
            for (int i = 0; i < (int)tempPlayer.size(); i++) {
                if (tempPlayer[i] != 'X') {
                    for (int j = 0; j < (int)tempSecret.size(); j++) {
                        if (tempSecret[j] == tempPlayer[i]) {
                            wrongPos++;
                            tempSecret[j] = 'X';
                            break;
                        }
                    }
                }
            }
            h.history.push_back({correct, wrongPos});
            if (correct == (int)h.secretCode.size()) h.completed = true;
            else if (h.attempts >= h.maxAttempts) h.failed = true;
            h.playerCode.clear();
            h.trace += 5;
        }
        break;
    }
    case HackType::MemoryGrid: {
        if (h.showPhase == 0) {
            h.showTimer += dt;
            if (h.showTimer > 0.8f) {
                h.showTimer = 0;
                h.currentShow++;
                if (h.currentShow >= (int)h.sequence.size()) {
                    h.showPhase = 1;
                }
            }
        } else {
            for (int k = KEY_ONE; k <= KEY_NINE; k++) {
                if (IsKeyPressed(k)) {
                    int num = k - KEY_ONE;
                    h.playerSeq.push_back(num);
                    if (h.playerSeq.back() != h.sequence[h.playerSeq.size()-1]) {
                        h.failed = true;
                    } else if (h.playerSeq.size() == h.sequence.size()) {
                        h.completed = true;
                    }
                    h.trace += 2;
                }
            }
        }
        break;
    }
    case HackType::PacketRoute: {
        int dx = 0, dy = 0;
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) dy = -1;
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) dy = 1;
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) dx = -1;
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) dx = 1;
        
        int nx = h.packetX + dx, ny = h.packetY + dy;
        if (nx >= 0 && nx < h.gridW && ny >= 0 && ny < h.gridH) {
            int idx = ny * h.gridW + nx;
            if (h.grid[idx] != 1) {
                h.packetX = nx; h.packetY = ny;
                h.playerPath.push_back(idx);
                h.trace += 1;
                if (nx == h.targetX && ny == h.targetY) h.completed = true;
            }
        }
        break;
    }
    case HackType::Decrypt: {
        for (int k = KEY_ZERO; k <= KEY_NINE; k++) {
            if (IsKeyPressed(k)) {
                int shift = k - KEY_ZERO;
                char decrypted = (char)('A' + (h.encrypted[h.patternPos] - 'A' - shift + 26) % 26);
                if (decrypted == h.decrypted[h.patternPos]) {
                    h.patternPos++;
                    if (h.patternPos >= (int)h.decrypted.size()) h.completed = true;
                } else {
                    h.trace += 10;
                }
            }
        }
        break;
    }
    case HackType::Firewall: {
        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) h.playerY -= 300 * dt;
        if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) h.playerY += 300 * dt;
        h.playerY = std::max(50.f, std::min((float)H - 50, h.playerY));
        
        for (auto& b : h.barriers) b -= h.scrollSpeed * dt;
        
        // Проверка столкновений и прохождения
        for (int i = 0; i < (int)h.barriers.size(); i++) {
            float b = h.barriers[i];
            if (b < 100 && b > 90) {
                float gapY = 100 + (i * 73) % (H - 200);
                float gapH = 120 - h.difficulty * 15;
                if (h.playerY < gapY || h.playerY > gapY + gapH) {
                    h.failed = true;
                } else {
                    h.passed++;
                    h.barriers[i] = -100; // Пройден
                    if (h.passed >= h.required) h.completed = true;
                }
            }
        }
        h.trace += (int)(dt * 3);
        break;
    }
    }
}

void DrawHackGame(const HackGame& h, Font& font) {
    auto Text = [&](const char* t, int x, int y, int s, Color c) {
        DrawTextEx(font, t, {(float)x, (float)y}, (float)s, 1, c);
    };
    auto Measure = [&](const char* t, int s) {
        return (int)MeasureTextEx(font, t, (float)s, 1).x;
    };
    
    // Заголовок
    const char* titles[] = {u8"ВЗЛОМ КОДА", u8"ПАМЯТЬ", u8"МАРШРУТ", u8"ДЕШИФРОВКА", u8"ФАЙРВОЛ"};
    DrawPanel(50, 50, W - 100, 50, C_CYAN);
    DrawNeonText(titles[(int)h.type], 70, 60, 28, C_CYAN);
    
    // Таймер и след
    char buf[64];
    snprintf(buf, 64, u8"ВРЕМЯ: %.1f", h.timer);
    Text(buf, W - 200, 60, 18, h.timer < 10 ? C_RED : C_YELLOW);
    snprintf(buf, 64, u8"СЛЕД: %d%%", h.trace);
    Text(buf, W - 350, 60, 18, h.trace > 70 ? C_RED : C_GREEN);
    
    // Игровая область
    DrawPanel(50, 120, W - 100, H - 200, C_CYAN);
    
    switch (h.type) {
    case HackType::CodeBreak: {
        Text(u8"Угадай код. Зелёный = верная позиция, Жёлтый = есть в коде", 70, 140, 14, C_GRAY);
        
        // Поле ввода
        int cx = W/2 - (int)h.secretCode.size() * 25;
        for (int i = 0; i < (int)h.secretCode.size(); i++) {
            DrawRectangle(cx + i * 50, 180, 40, 50, {30, 40, 60, 255});
            DrawRectangleLinesEx({(float)(cx + i * 50), 180, 40, 50}, 2, C_CYAN);
            if (i < (int)h.playerCode.size()) {
                char c[2] = {h.playerCode[i], 0};
                Text(c, cx + i * 50 + 12, 190, 28, C_WHITE);
            }
        }
        
        // История
        int hy = 260;
        for (int i = 0; i < (int)h.history.size(); i++) {
            snprintf(buf, 64, u8"Попытка %d:", i + 1);
            Text(buf, 70, hy, 16, C_GRAY);
            snprintf(buf, 64, u8"● %d  ○ %d", h.history[i].first, h.history[i].second);
            Text(buf, 200, hy, 16, C_WHITE);
            // Цветные индикаторы
            for (int j = 0; j < h.history[i].first; j++)
                DrawCircle(350 + j * 20, hy + 8, 6, C_GREEN);
            for (int j = 0; j < h.history[i].second; j++)
                DrawCircle(350 + h.history[i].first * 20 + j * 20, hy + 8, 6, C_YELLOW);
            hy += 30;
        }
        
        snprintf(buf, 64, u8"Попыток: %d/%d", h.attempts, h.maxAttempts);
        Text(buf, 70, H - 100, 16, C_YELLOW);
        Text(u8"[0-9] Ввод  [BACKSPACE] Удалить  [ENTER] Проверить", 300, H - 100, 14, C_GRAY);
        break;
    }
    case HackType::MemoryGrid: {
        Text(u8"Запомни последовательность и повтори", 70, 140, 14, C_GRAY);
        
        // Сетка 3x3
        int gx = W/2 - 120, gy = 200;
        for (int i = 0; i < 9; i++) {
            int x = gx + (i % 3) * 80, y = gy + (i / 3) * 80;
            bool lit = (h.showPhase == 0 && h.currentShow < (int)h.sequence.size() && 
                       h.sequence[h.currentShow] == i);
            bool done = false;
            for (int j = 0; j < (int)h.playerSeq.size(); j++) {
                if (h.sequence[j] == i && j < (int)h.playerSeq.size()) done = true;
            }
            Color c = lit ? C_CYAN : done ? C_GREEN : Color{40, 50, 70, 255};
            DrawRectangle(x, y, 70, 70, c);
            DrawRectangleLinesEx({(float)x, (float)y, 70, 70}, 2, C_CYAN);
            char n[2] = {(char)('1' + i), 0};
            Text(n, x + 25, y + 20, 24, lit || done ? C_WHITE : C_GRAY);
        }
        
        if (h.showPhase == 0) {
            snprintf(buf, 64, u8"Запоминай: %d/%d", h.currentShow + 1, (int)h.sequence.size());
            Text(buf, W/2 - 80, gy + 260, 18, C_YELLOW);
        } else {
            snprintf(buf, 64, u8"Введено: %d/%d", (int)h.playerSeq.size(), (int)h.sequence.size());
            Text(buf, W/2 - 80, gy + 260, 18, C_GREEN);
        }
        break;
    }
    case HackType::PacketRoute: {
        Text(u8"Доведи пакет до цели. Избегай стен.", 70, 140, 14, C_GRAY);
        
        int cellW = (W - 200) / h.gridW;
        int cellH = (H - 300) / h.gridH;
        int ox = 100, oy = 180;
        
        for (int y = 0; y < h.gridH; y++) {
            for (int x = 0; x < h.gridW; x++) {
                int idx = y * h.gridW + x;
                int px = ox + x * cellW, py = oy + y * cellH;
                Color c = {30, 40, 60, 255};
                if (h.grid[idx] == 1) c = {80, 40, 40, 255}; // wall
                if (h.grid[idx] == 2) c = C_GREEN; // start
                if (h.grid[idx] == 3) c = C_MAG; // end
                // Path
                for (int p : h.playerPath) if (p == idx) c = {0, 100, 100, 255};
                DrawRectangle(px, py, cellW - 2, cellH - 2, c);
                if (x == h.packetX && y == h.packetY) {
                    DrawRectangle(px + 5, py + 5, cellW - 12, cellH - 12, C_CYAN);
                }
            }
        }
        Text(u8"[WASD/Стрелки] Движение", 70, H - 100, 14, C_GRAY);
        break;
    }
    case HackType::Decrypt: {
        Text(u8"Найди сдвиг для каждой буквы (0-9)", 70, 140, 14, C_GRAY);
        
        int cx = W/2 - (int)h.encrypted.size() * 30;
        // Зашифрованное
        Text(u8"Шифр:", cx - 80, 200, 18, C_GRAY);
        for (int i = 0; i < (int)h.encrypted.size(); i++) {
            char c[2] = {h.encrypted[i], 0};
            Color col = i < h.patternPos ? C_GREEN : i == h.patternPos ? C_CYAN : C_GRAY;
            DrawRectangle(cx + i * 60, 195, 50, 50, {30, 40, 60, 255});
            DrawRectangleLinesEx({(float)(cx + i * 60), 195, 50, 50}, 2, col);
            Text(c, cx + i * 60 + 15, 205, 28, col);
        }
        
        // Расшифрованное
        Text(u8"Текст:", cx - 80, 280, 18, C_GRAY);
        for (int i = 0; i < (int)h.decrypted.size(); i++) {
            char c[2] = {i < h.patternPos ? h.decrypted[i] : '?', 0};
            Color col = i < h.patternPos ? C_GREEN : C_GRAY;
            DrawRectangle(cx + i * 60, 275, 50, 50, {30, 40, 60, 255});
            Text(c, cx + i * 60 + 15, 285, 28, col);
        }
        
        // Подсказка
        if (h.patternPos < (int)h.encrypted.size()) {
            Text(u8"Текущая буква:", 70, 380, 16, C_GRAY);
            char cur[2] = {h.encrypted[h.patternPos], 0};
            Text(cur, 200, 375, 24, C_CYAN);
            Text(u8"Введи сдвиг (0-9):", 70, 420, 16, C_YELLOW);
        }
        break;
    }
    case HackType::Firewall: {
        Text(u8"Пролети через щели в файрволе!", 70, 140, 14, C_GRAY);
        
        // Игровое поле
        DrawRectangle(80, 160, W - 160, H - 260, {10, 15, 25, 255});
        
        // Барьеры
        for (int i = 0; i < (int)h.barriers.size(); i++) {
            float b = h.barriers[i];
            if (b > 80 && b < W - 80) {
                float gapY = 100 + (i * 73) % (H - 200);
                float gapH = 120 - h.difficulty * 15;
                // Верхняя часть
                DrawRectangle((int)b, 160, 20, (int)(gapY - 160), C_RED);
                // Нижняя часть
                DrawRectangle((int)b, (int)(gapY + gapH), 20, H - 260 - (int)(gapY + gapH - 160), C_RED);
            }
        }
        
        // Игрок
        DrawCircle(120, (int)h.playerY, 15, C_CYAN);
        DrawCircle(120, (int)h.playerY, 10, C_WHITE);
        
        snprintf(buf, 64, u8"Пройдено: %d/%d", h.passed, h.required);
        Text(buf, W/2 - 60, H - 90, 18, C_GREEN);
        Text(u8"[W/S или Стрелки] Движение", 70, H - 90, 14, C_GRAY);
        break;
    }
    }
}

bool IsHackComplete(const HackGame& h) { return h.completed; }
bool IsHackFailed(const HackGame& h) { return h.failed; }
