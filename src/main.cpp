#include "types.hpp"
#include <cstdlib>
#include <ctime>

int main() {
    std::srand((unsigned)std::time(nullptr));
    
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(W, H, u8"HACKER SIMULATOR: GENESIS // Cyberpunk RPG");
    SetTargetFPS(60);
    
    // Загрузка шрифта с кириллицей
    int codepoints[512], n = 0;
    for (int i = 32; i < 127; i++) codepoints[n++] = i;           // ASCII
    for (int i = 0x410; i <= 0x44F; i++) codepoints[n++] = i;     // А-я
    codepoints[n++] = 0x401; codepoints[n++] = 0x451;             // Ё ё
    
    gFont = LoadFontEx("resources/JetBrainsMono.ttf", 32, codepoints, n);
    if (!gFont.texture.id) gFont = GetFontDefault();
    SetTextureFilter(gFont.texture, TEXTURE_FILTER_BILINEAR);
    
    InitStory();
    InitVisuals();
    InitSystems();
    
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        UpdateGame(dt);
        
        BeginDrawing();
        
        switch (gScr) {
            case Scr::Intro: DrawIntro(); break;
            case Scr::Menu: DrawMenu(); break;
            case Scr::Game: DrawGame(); DrawHUD(); break;
            case Scr::Hack: DrawHack(); break;
            case Scr::Shop: DrawShop(); break;
            case Scr::Event: DrawEvent(); break;
            case Scr::End: DrawEnd(); break;
        }
        
        DrawEffects();
        EndDrawing();
    }
    
    if (gFont.texture.id) UnloadFont(gFont);
    CloseWindow();
    return 0;
}
