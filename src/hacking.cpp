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

// Глобальные переменные для тача в хакинге
static bool hTouchReleased = false;
static Vector2 hTouchPos = {0, 0};

void UpdateHackGame(HackGame& h, float dt) {
    if (h.completed || h.failed) return;
    
    h.timer -= dt;
    if (h.timer <= 0) { h.failed = true; return; }
    
    // Обновление тача
    hTouchReleased = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
    hTouchPos = GetTouchPointCount() > 0 ? GetTouchPosition(0) : GetMousePosition();
    
    // Хелпер для проверки тапа
    auto Tapped = [](int x, int y, int w, int h) {
        return hTouchReleased && 
               hTouchPos.x >= x && hTouchPos.x <= x + w &&
               hTouchPos.y >= y && hTouchPos.y <= y + h;
    };
    
    switch (h.type) {
    case HackType::CodeBreak: {
        // Экранная клавиатура - позиция
        int keyW = Sc(60), keyH = Sc(55), keyPad = Sc(8);
        int keysStartX = W/2 - (5 * keyW + 4 * keyPad) / 2;
        int keysStartY = H - Sc(180);
        
        // Тап по цифрам 0-9
        for (int i = 0; i < 10; i++) {
            int row = i / 5, col = i % 5;
            int kx = keysStartX + col * (keyW + keyPad);
            int ky = keysStartY + row * (keyH + keyPad);
            if (Tapped(kx, ky, keyW, keyH) && (int)h.playerCode.size() < (int)h.secretCode.size()) {
                h.playerCode += '0' + i;
            }
        }
        
        // Кнопка удаления
        int delX = keysStartX + 5 * (keyW + keyPad);
        if (Tapped(delX, keysStartY, keyW, keyH) && !h.playerCode.empty()) {
            h.playerCode.pop_back();
        }
        
        // Кнопка подтверждения
        if (Tapped(delX, keysStartY + keyH + keyPad, keyW, keyH) && 
            (int)h.playerCode.size() == (int)h.secretCode.size()) {
            // Проверка кода - ниже
        }
        
        // Клавиатура
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
        
        bool confirmCode = IsKeyPressed(KEY_ENTER) || 
                          (Tapped(delX, keysStartY + keyH + keyPad, keyW, keyH));
        if (confirmCode && (int)h.playerCode.size() == (int)h.secretCode.size()) {
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
            // Сетка 3x3 - тап по ячейкам
            int cellSize = Sc(90);
            int cellPad = Sc(10);
            int gx = W/2 - (3 * cellSize + 2 * cellPad) / 2;
            int gy = Sc(180);
            
            for (int i = 0; i < 9; i++) {
                int col = i % 3, row = i / 3;
                int cx = gx + col * (cellSize + cellPad);
                int cy = gy + row * (cellSize + cellPad);
                
                if (Tapped(cx, cy, cellSize, cellSize)) {
                    h.playerSeq.push_back(i);
                    if (h.playerSeq.back() != h.sequence[h.playerSeq.size()-1]) {
                        h.failed = true;
                    } else if (h.playerSeq.size() == h.sequence.size()) {
                        h.completed = true;
                    }
                    h.trace += 2;
                }
            }
            
            // Клавиатура
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
        
        // Кнопки направления на экране
        int btnSize = Sc(70);
        int btnPad = Sc(5);
        int ctrlX = W - Sc(200);
        int ctrlY = H - Sc(200);
        
        // Вверх
        if (Tapped(ctrlX + btnSize + btnPad, ctrlY, btnSize, btnSize)) dy = -1;
        // Вниз
        if (Tapped(ctrlX + btnSize + btnPad, ctrlY + 2*(btnSize + btnPad), btnSize, btnSize)) dy = 1;
        // Влево
        if (Tapped(ctrlX, ctrlY + btnSize + btnPad, btnSize, btnSize)) dx = -1;
        // Вправо
        if (Tapped(ctrlX + 2*(btnSize + btnPad), ctrlY + btnSize + btnPad, btnSize, btnSize)) dx = 1;
        
        // Клавиатура
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
        // Экранные кнопки 0-9
        int keyW = Sc(55), keyH = Sc(50), keyPad = Sc(6);
        int keysStartX = W/2 - (5 * keyW + 4 * keyPad) / 2;
        int keysStartY = H - Sc(140);
        
        for (int i = 0; i < 10; i++) {
            int row = i / 5, col = i % 5;
            int kx = keysStartX + col * (keyW + keyPad);
            int ky = keysStartY + row * (keyH + keyPad);
            
            if (Tapped(kx, ky, keyW, keyH)) {
                int shift = i;
                char decrypted = (char)('A' + (h.encrypted[h.patternPos] - 'A' - shift + 26) % 26);
                if (decrypted == h.decrypted[h.patternPos]) {
                    h.patternPos++;
                    if (h.patternPos >= (int)h.decrypted.size()) h.completed = true;
                } else {
                    h.trace += 10;
                }
            }
        }
        
        // Клавиатура
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
        // Сенсорное управление - игрок следует за пальцем
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) || GetTouchPointCount() > 0) {
            Vector2 touch = GetTouchPointCount() > 0 ? GetTouchPosition(0) : GetMousePosition();
            // Плавное движение к точке касания
            float targetY = touch.y;
            float diff = targetY - h.playerY;
            h.playerY += diff * 8 * dt; // Плавность
        }
        
        // Клавиатура
        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) h.playerY -= 300 * dt;
        if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) h.playerY += 300 * dt;
        h.playerY = std::max(160.f, std::min((float)H - 100, h.playerY));
        
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
        Text(buf, 70, Sc(130), Sc(16), C_YELLOW);
        
        // Экранная клавиатура
        int keyW = Sc(60), keyH = Sc(55), keyPad = Sc(8);
        int keysStartX = W/2 - (5 * keyW + 4 * keyPad) / 2;
        int keysStartY = H - Sc(180);
        
        for (int i = 0; i < 10; i++) {
            int row = i / 5, col = i % 5;
            int kx = keysStartX + col * (keyW + keyPad);
            int ky = keysStartY + row * (keyH + keyPad);
            DrawRectangleRounded({(float)kx, (float)ky, (float)keyW, (float)keyH}, 0.2f, 8, {40, 60, 80, 255});
            DrawRectangleRoundedLinesEx({(float)kx, (float)ky, (float)keyW, (float)keyH}, 0.2f, 8, 2, C_CYAN);
            char digit[2] = {(char)('0' + i), 0};
            Text(digit, kx + keyW/2 - Sc(8), ky + Sc(12), Sc(28), C_WHITE);
        }
        
        // Кнопка удаления
        int delX = keysStartX + 5 * (keyW + keyPad);
        DrawRectangleRounded({(float)delX, (float)keysStartY, (float)keyW, (float)keyH}, 0.2f, 8, {80, 40, 40, 255});
        DrawRectangleRoundedLinesEx({(float)delX, (float)keysStartY, (float)keyW, (float)keyH}, 0.2f, 8, 2, C_RED);
        Text(u8"←", delX + Sc(15), keysStartY + Sc(12), Sc(28), C_WHITE);
        
        // Кнопка подтверждения
        int okY = keysStartY + keyH + keyPad;
        Color okCol = (int)h.playerCode.size() == (int)h.secretCode.size() ? C_GREEN : C_GRAY;
        DrawRectangleRounded({(float)delX, (float)okY, (float)keyW, (float)keyH}, 0.2f, 8, {40, 80, 40, 255});
        DrawRectangleRoundedLinesEx({(float)delX, (float)okY, (float)keyW, (float)keyH}, 0.2f, 8, 2, okCol);
        Text(u8"OK", delX + Sc(10), okY + Sc(12), Sc(24), C_WHITE);
        break;
    }
    case HackType::MemoryGrid: {
        Text(u8"Запомни и повтори касанием", Sc(70), Sc(140), Sc(18), C_GRAY);
        
        // Сетка 3x3 - большие кнопки для пальцев
        int cellSize = Sc(90);
        int cellPad = Sc(10);
        int gx = W/2 - (3 * cellSize + 2 * cellPad) / 2;
        int gy = Sc(180);
        
        for (int i = 0; i < 9; i++) {
            int col = i % 3, row = i / 3;
            int x = gx + col * (cellSize + cellPad);
            int y = gy + row * (cellSize + cellPad);
            
            bool lit = (h.showPhase == 0 && h.currentShow < (int)h.sequence.size() && 
                       h.sequence[h.currentShow] == i);
            bool done = false;
            for (int j = 0; j < (int)h.playerSeq.size(); j++) {
                if (h.sequence[j] == i && j < (int)h.playerSeq.size()) done = true;
            }
            Color c = lit ? C_CYAN : done ? C_GREEN : Color{40, 50, 70, 255};
            DrawRectangleRounded({(float)x, (float)y, (float)cellSize, (float)cellSize}, 0.15f, 8, c);
            DrawRectangleRoundedLinesEx({(float)x, (float)y, (float)cellSize, (float)cellSize}, 0.15f, 8, 2, C_CYAN);
            char n[2] = {(char)('1' + i), 0};
            Text(n, x + cellSize/2 - Sc(10), y + cellSize/2 - Sc(15), Sc(32), lit || done ? C_WHITE : C_GRAY);
        }
        
        int statusY = gy + 3 * (cellSize + cellPad) + Sc(20);
        if (h.showPhase == 0) {
            snprintf(buf, 64, u8"Запоминай: %d/%d", h.currentShow + 1, (int)h.sequence.size());
            Text(buf, W/2 - Sc(80), statusY, Sc(22), C_YELLOW);
        } else {
            snprintf(buf, 64, u8"Коснись: %d/%d", (int)h.playerSeq.size(), (int)h.sequence.size());
            Text(buf, W/2 - Sc(80), statusY, Sc(22), C_GREEN);
        }
        break;
    }
    case HackType::PacketRoute: {
        Text(u8"Веди пакет к цели", Sc(70), Sc(140), Sc(18), C_GRAY);
        
        // Сетка поменьше чтобы влезли кнопки
        int cellW = (W - Sc(280)) / h.gridW;
        int cellH = (H - Sc(250)) / h.gridH;
        int ox = Sc(60), oy = Sc(170);
        
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
                    DrawRectangle(px + 4, py + 4, cellW - 10, cellH - 10, C_CYAN);
                }
            }
        }
        
        // D-pad кнопки
        int btnSize = Sc(70);
        int btnPad = Sc(5);
        int ctrlX = W - Sc(200);
        int ctrlY = H - Sc(200);
        
        // Вверх
        DrawRectangleRounded({(float)(ctrlX + btnSize + btnPad), (float)ctrlY, (float)btnSize, (float)btnSize}, 0.2f, 8, {40, 60, 80, 255});
        DrawRectangleRoundedLinesEx({(float)(ctrlX + btnSize + btnPad), (float)ctrlY, (float)btnSize, (float)btnSize}, 0.2f, 8, 2, C_CYAN);
        Text(u8"↑", ctrlX + btnSize + btnPad + Sc(22), ctrlY + Sc(15), Sc(32), C_WHITE);
        
        // Вниз
        int downY = ctrlY + 2*(btnSize + btnPad);
        DrawRectangleRounded({(float)(ctrlX + btnSize + btnPad), (float)downY, (float)btnSize, (float)btnSize}, 0.2f, 8, {40, 60, 80, 255});
        DrawRectangleRoundedLinesEx({(float)(ctrlX + btnSize + btnPad), (float)downY, (float)btnSize, (float)btnSize}, 0.2f, 8, 2, C_CYAN);
        Text(u8"↓", ctrlX + btnSize + btnPad + Sc(22), downY + Sc(15), Sc(32), C_WHITE);
        
        // Влево
        int midY = ctrlY + btnSize + btnPad;
        DrawRectangleRounded({(float)ctrlX, (float)midY, (float)btnSize, (float)btnSize}, 0.2f, 8, {40, 60, 80, 255});
        DrawRectangleRoundedLinesEx({(float)ctrlX, (float)midY, (float)btnSize, (float)btnSize}, 0.2f, 8, 2, C_CYAN);
        Text(u8"←", ctrlX + Sc(22), midY + Sc(15), Sc(32), C_WHITE);
        
        // Вправо
        int rightX = ctrlX + 2*(btnSize + btnPad);
        DrawRectangleRounded({(float)rightX, (float)midY, (float)btnSize, (float)btnSize}, 0.2f, 8, {40, 60, 80, 255});
        DrawRectangleRoundedLinesEx({(float)rightX, (float)midY, (float)btnSize, (float)btnSize}, 0.2f, 8, 2, C_CYAN);
        Text(u8"→", rightX + Sc(22), midY + Sc(15), Sc(32), C_WHITE);
        break;
    }
    case HackType::Decrypt: {
        Text(u8"Введи сдвиг для буквы", Sc(70), Sc(140), Sc(18), C_GRAY);
        
        int letterSize = Sc(50);
        int letterPad = Sc(8);
        int cx = W/2 - (int)h.encrypted.size() * (letterSize + letterPad) / 2;
        
        // Зашифрованное
        Text(u8"Шифр:", Sc(70), Sc(180), Sc(18), C_GRAY);
        for (int i = 0; i < (int)h.encrypted.size(); i++) {
            char c[2] = {h.encrypted[i], 0};
            Color col = i < h.patternPos ? C_GREEN : i == h.patternPos ? C_CYAN : C_GRAY;
            int lx = cx + i * (letterSize + letterPad);
            DrawRectangleRounded({(float)lx, (float)Sc(175), (float)letterSize, (float)letterSize}, 0.15f, 8, {30, 40, 60, 255});
            DrawRectangleRoundedLinesEx({(float)lx, (float)Sc(175), (float)letterSize, (float)letterSize}, 0.15f, 8, 2, col);
            Text(c, lx + letterSize/2 - Sc(10), Sc(185), Sc(28), col);
        }
        
        // Расшифрованное
        Text(u8"Текст:", Sc(70), Sc(255), Sc(18), C_GRAY);
        for (int i = 0; i < (int)h.decrypted.size(); i++) {
            char c[2] = {i < h.patternPos ? h.decrypted[i] : '?', 0};
            Color col = i < h.patternPos ? C_GREEN : C_GRAY;
            int lx = cx + i * (letterSize + letterPad);
            DrawRectangleRounded({(float)lx, (float)Sc(250), (float)letterSize, (float)letterSize}, 0.15f, 8, {30, 40, 60, 255});
            Text(c, lx + letterSize/2 - Sc(10), Sc(260), Sc(28), col);
        }
        
        // Экранные кнопки 0-9
        int keyW = Sc(55), keyH = Sc(50), keyPad = Sc(6);
        int keysStartX = W/2 - (5 * keyW + 4 * keyPad) / 2;
        int keysStartY = H - Sc(140);
        
        for (int i = 0; i < 10; i++) {
            int row = i / 5, col = i % 5;
            int kx = keysStartX + col * (keyW + keyPad);
            int ky = keysStartY + row * (keyH + keyPad);
            DrawRectangleRounded({(float)kx, (float)ky, (float)keyW, (float)keyH}, 0.2f, 8, {40, 60, 80, 255});
            DrawRectangleRoundedLinesEx({(float)kx, (float)ky, (float)keyW, (float)keyH}, 0.2f, 8, 2, C_CYAN);
            char digit[2] = {(char)('0' + i), 0};
            Text(digit, kx + keyW/2 - Sc(8), ky + Sc(10), Sc(26), C_WHITE);
        }
        break;
    }
    case HackType::Firewall: {
        Text(u8"Касайся экрана чтобы двигаться!", Sc(70), Sc(140), Sc(18), C_GRAY);
        
        // Игровое поле
        int fieldX = Sc(60), fieldY = Sc(160);
        int fieldW = W - Sc(120), fieldH = H - Sc(220);
        DrawRectangle(fieldX, fieldY, fieldW, fieldH, {10, 15, 25, 255});
        DrawRectangleLinesEx({(float)fieldX, (float)fieldY, (float)fieldW, (float)fieldH}, 2, C_CYAN);
        
        // Барьеры
        for (int i = 0; i < (int)h.barriers.size(); i++) {
            float b = h.barriers[i];
            if (b > fieldX && b < fieldX + fieldW - 20) {
                float gapY = fieldY + 50 + (i * 73) % (fieldH - 150);
                float gapH = 100 - h.difficulty * 12;
                // Верхняя часть
                DrawRectangle((int)b, fieldY, Sc(20), (int)(gapY - fieldY), C_RED);
                // Нижняя часть
                DrawRectangle((int)b, (int)(gapY + gapH), Sc(20), fieldY + fieldH - (int)(gapY + gapH), C_RED);
            }
        }
        
        // Игрок - большой для видимости
        int playerX = fieldX + Sc(60);
        DrawCircle(playerX, (int)h.playerY, Sc(20), C_CYAN);
        DrawCircle(playerX, (int)h.playerY, Sc(14), C_WHITE);
        
        snprintf(buf, 64, u8"Пройдено: %d/%d", h.passed, h.required);
        Text(buf, W/2 - Sc(80), H - Sc(60), Sc(22), C_GREEN);
        break;
    }
    }
}

bool IsHackComplete(const HackGame& h) { return h.completed; }
bool IsHackFailed(const HackGame& h) { return h.failed; }
