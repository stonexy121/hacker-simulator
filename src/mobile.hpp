#pragma once
#include "raylib.h"

// Мобильные константы
namespace Mobile {
    // Динамические размеры экрана
    inline int ScreenW() { return GetScreenWidth(); }
    inline int ScreenH() { return GetScreenHeight(); }
    
    // Масштаб UI относительно базового разрешения 1280x720
    inline float ScaleX() { return GetScreenWidth() / 1280.0f; }
    inline float ScaleY() { return GetScreenHeight() / 720.0f; }
    inline float Scale() { return (ScaleX() + ScaleY()) / 2.0f; }
    
    // Адаптивные размеры
    inline int Scaled(int v) { return (int)(v * Scale()); }
    inline float ScaledF(float v) { return v * Scale(); }
    
    // Безопасные зоны (для вырезов камеры и т.д.)
    inline int SafeLeft() { return Scaled(20); }
    inline int SafeRight() { return ScreenW() - Scaled(20); }
    inline int SafeTop() { return Scaled(20); }
    inline int SafeBottom() { return ScreenH() - Scaled(20); }
    
    // Размеры кнопок для пальцев (минимум 48dp ~ 96px на hdpi)
    inline int ButtonH() { return Scaled(70); }
    inline int ButtonPadding() { return Scaled(15); }
    
    // Размеры шрифтов
    inline int FontLarge() { return Scaled(32); }
    inline int FontMedium() { return Scaled(24); }
    inline int FontSmall() { return Scaled(18); }
    inline int FontTiny() { return Scaled(14); }
}

// Сенсорный ввод
struct TouchInput {
    Vector2 pos;
    Vector2 startPos;
    Vector2 delta;
    bool pressed;
    bool released;
    bool held;
    float holdTime;
    
    void Update() {
        pressed = false;
        released = false;
        delta = {0, 0};
        
        int touchCount = GetTouchPointCount();
        
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || (touchCount > 0 && !held)) {
            pressed = true;
            held = true;
            holdTime = 0;
            pos = touchCount > 0 ? GetTouchPosition(0) : GetMousePosition();
            startPos = pos;
        }
        
        if (held) {
            Vector2 newPos = touchCount > 0 ? GetTouchPosition(0) : GetMousePosition();
            delta = {newPos.x - pos.x, newPos.y - pos.y};
            pos = newPos;
            holdTime += GetFrameTime();
        }
        
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) || (touchCount == 0 && held)) {
            released = true;
            held = false;
        }
    }
    
    bool InRect(int x, int y, int w, int h) const {
        return pos.x >= x && pos.x <= x + w && pos.y >= y && pos.y <= y + h;
    }
    
    bool TappedRect(int x, int y, int w, int h) const {
        return released && holdTime < 0.3f && InRect(x, y, w, h);
    }
};

extern TouchInput gTouch;
