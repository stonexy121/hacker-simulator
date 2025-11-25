#include "mobile.hpp"
#include "types.hpp"
#include <cmath>
#include <algorithm>

TouchInput gTouch;

using namespace Mobile;

// Рисование адаптивной кнопки
bool DrawMobileButton(const char* text, int x, int y, int w, int h, Color bg, Color fg, bool selected = false) {
    Rectangle rect = {(float)x, (float)y, (float)w, (float)h};
    
    // Подсветка при касании
    bool touching = gTouch.held && gTouch.InRect(x, y, w, h);
    bool tapped = gTouch.TappedRect(x, y, w, h);
    
    Color drawBg = touching ? Color{bg.r, bg.g, bg.b, 255} : bg;
    if (selected) {
        drawBg = C_CYAN;
        DrawRectangle(x - 4, y - 4, w + 8, h + 8, {0, 255, 255, 50});
    }
    
    DrawRectangleRounded(rect, 0.2f, 8, drawBg);
    DrawRectangleRoundedLinesEx(rect, 0.2f, 8, 2, selected ? C_WHITE : C_CYAN);
    
    // Текст по центру
    int fontSize = FontMedium();
    int textW = MeasureText2(text, fontSize);
    DrawText2(text, x + (w - textW) / 2, y + (h - fontSize) / 2, fontSize, fg);
    
    return tapped;
}

// Рисование панели диалога
void DrawMobileDialogPanel(int x, int y, int w, int h) {
    DrawRectangle(x, y, w, h, {10, 15, 25, 230});
    DrawRectangleLinesEx({(float)x, (float)y, (float)w, (float)h}, 2, C_CYAN);
    
    // Декоративные уголки
    int corner = Scaled(15);
    DrawLine(x, y + corner, x + corner, y, C_CYAN);
    DrawLine(x + w - corner, y, x + w, y + corner, C_CYAN);
    DrawLine(x, y + h - corner, x + corner, y + h, C_CYAN);
    DrawLine(x + w - corner, y + h, x + w, y + h - corner, C_CYAN);
}

// Рисование прогресс-бара
void DrawMobileProgressBar(int x, int y, int w, int h, float progress, Color fg, Color bg) {
    DrawRectangle(x, y, w, h, bg);
    DrawRectangle(x, y, (int)(w * progress), h, fg);
    DrawRectangleLinesEx({(float)x, (float)y, (float)w, (float)h}, 2, C_CYAN);
}

// Рисование HUD для мобильных
void DrawMobileHUD(int credits, int karma, int hackBonus) {
    int pad = SafeLeft();
    int y = SafeTop();
    int fontSize = FontSmall();
    
    // Полупрозрачная панель сверху
    DrawRectangle(0, 0, ScreenW(), y + Scaled(50), {0, 0, 0, 150});
    
    // Кредиты
    char buf[64];
    snprintf(buf, 64, u8"💰 %d", credits);
    DrawText2(buf, pad, y + Scaled(10), fontSize, C_YELLOW);
    
    // Карма
    snprintf(buf, 64, u8"⚖ %d", karma);
    Color karmaColor = karma > 0 ? C_GREEN : karma < 0 ? C_RED : C_GRAY;
    DrawText2(buf, pad + Scaled(150), y + Scaled(10), fontSize, karmaColor);
    
    // Бонус взлома
    if (hackBonus > 0) {
        snprintf(buf, 64, u8"🔧 +%d%%", hackBonus);
        DrawText2(buf, pad + Scaled(300), y + Scaled(10), fontSize, C_CYAN);
    }
}

// Рисование подсказки "коснитесь для продолжения"
void DrawTapToContinue(float time) {
    float alpha = 0.5f + 0.5f * sinf(time * 3);
    int fontSize = FontSmall();
    const char* text = u8"Коснитесь для продолжения";
    int textW = MeasureText2(text, fontSize);
    DrawText2(text, (ScreenW() - textW) / 2, SafeBottom() - Scaled(40), fontSize, 
              {200, 200, 200, (unsigned char)(alpha * 255)});
}

// Рисование вариантов выбора для мобильных
int DrawMobileChoices(const std::vector<DChoice>& choices, int selected, float time) {
    int result = -1;
    int n = (int)choices.size();
    if (n == 0) return result;
    
    int btnH = ButtonH();
    int btnPad = ButtonPadding();
    int totalH = n * btnH + (n - 1) * btnPad;
    int startY = (ScreenH() - totalH) / 2;
    int btnW = std::min(ScreenW() - SafeLeft() * 2, Scaled(800));
    int startX = (ScreenW() - btnW) / 2;
    
    for (int i = 0; i < n; i++) {
        int y = startY + i * (btnH + btnPad);
        bool sel = (i == selected);
        
        // Номер варианта
        char num[8];
        snprintf(num, 8, "%d.", i + 1);
        
        std::string text = std::string(num) + " " + choices[i].text;
        
        // Индикатор кармы
        if (choices[i].karma > 0) text += u8" [+карма]";
        else if (choices[i].karma < 0) text += u8" [-карма]";
        
        Color bg = sel ? Color{0, 80, 80, 200} : Color{20, 30, 50, 200};
        if (DrawMobileButton(text.c_str(), startX, y, btnW, btnH, bg, C_WHITE, sel)) {
            result = i;
        }
        
        // Выбор касанием
        if (gTouch.TappedRect(startX, y, btnW, btnH)) {
            result = i;
        }
    }
    
    return result;
}
