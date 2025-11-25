#include "types.hpp"
#include "hacking.hpp"
#include <cmath>
#include <cstdlib>
#include <algorithm>

Scr gScr = Scr::Intro;
float gScrTime = 0;
int gMenuSel = 0, gCredits = 0, gKarma = 0, gScene = 0, gLine = 0;
float gTypeTimer = 0;
int gTypeIdx = 0;
std::string gDispText;
int gHackProg = 0, gHackTrace = 0;
float gHackTime = 0;
int gHackType = 0;
std::map<std::string, bool> gFlags;
std::deque<std::pair<std::string, Color>> gLog;

HackGame gHack;

void AddLog(const std::string& t, Color c) {
    gLog.push_front({t, c});
    if (gLog.size() > 12) gLog.pop_back();
}

void DrawIntro() {
    DrawBackground();
    float a = std::min(1.f, gScrTime);
    float p = 0.5f + 0.5f * sinf(gTime * 2);
    
    // Анимированный логотип
    DrawNeonText("HACKER", W/2 - 130, H/2 - 100, 52, {0, (unsigned char)(255*p), 255, (unsigned char)(a*255)});
    DrawNeonText("SIMULATOR", W/2 - 170, H/2 - 30, 52, {255, 0, 128, (unsigned char)(a*255)});
    DrawNeonText("G E N E S I S", W/2 - 110, H/2 + 50, 22, {200, 200, 200, (unsigned char)(a*200)});
    
    // Декоративные линии
    float lw = 300 * std::min(1.f, gScrTime * 0.5f);
    DrawRectangle((int)(W/2 - lw/2), H/2 + 90, (int)lw, 2, {0, 255, 255, (unsigned char)(a*150)});
    
    if (gScrTime > 2) {
        float b = 0.5f + 0.5f * sinf(gTime * 3);
        DrawText2(u8"[ НАЖМИ ENTER ]", W/2 - 80, H - 100, 18, {200, 200, 200, (unsigned char)(b*255)});
    }
    
    DrawText2(u8"© 2024 NeyroSet Games", W/2 - 90, H - 40, 14, {80, 80, 80, 255});
}

void DrawMenu() {
    DrawBackground();
    DrawCity();
    DrawRectangle(0, 0, W, H, {0, 0, 0, 180});
    
    float p = 0.7f + 0.3f * sinf(gTime * 2);
    DrawNeonText("HACKER SIMULATOR", W/2 - 220, 60, 46, {0, (unsigned char)(255*p), 255, 255});
    DrawNeonText("GENESIS", W/2 - 90, 115, 36, C_MAG);
    
    // Декор
    DrawRectangle(W/2 - 250, 165, 500, 2, {0, 255, 255, 100});
    
    const char* opts[] = {u8"НОВАЯ ИГРА", u8"МАГАЗИН", u8"ДОСТИЖЕНИЯ", u8"ВЫХОД"};
    for (int i = 0; i < 4; i++) {
        int y = 220 + i * 65;
        bool s = i == gMenuSel;
        if (s) {
            DrawPanel(W/2 - 160, y - 12, 320, 55, C_CYAN);
            float ar = sinf(gTime * 5) * 8;
            DrawText2(">", W/2 - 185 + (int)ar, y + 5, 28, C_CYAN);
            DrawText2("<", W/2 + 165 - (int)ar, y + 5, 28, C_CYAN);
        }
        DrawText2(opts[i], W/2 - MeasureText2(opts[i], 26)/2, y + 5, 26, s ? C_WHITE : C_GRAY);
    }
    
    // Статистика
    DrawPanel(50, H - 120, 250, 100, C_CYAN);
    DrawText2(u8"СТАТИСТИКА", 70, H - 110, 14, C_CYAN);
    char buf[64];
    snprintf(buf, 64, u8"Взломов: %d  Кредиты: %d", gTotalHacks, gCredits);
    DrawText2(buf, 70, H - 85, 14, C_GRAY);
    int unlocked = 0;
    for (auto& a : gAchievements) if (a.unlocked) unlocked++;
    snprintf(buf, 64, u8"Достижения: %d/%d", unlocked, (int)gAchievements.size());
    DrawText2(buf, 70, H - 65, 14, C_GRAY);
    
    DrawText2(u8"v2.0 FULL // 10 глав // 3 концовки", 20, H - 25, 12, C_GRAY);
}

void DrawGame() {
    DrawBackground();
    Scene& sc = gScenes[gScene];
    
    // Фон локации
    if (sc.loc == "city" || sc.loc == "tower") DrawCity();
    DrawRectangle(0, 0, W, H, {0, 0, 0, 160});
    
    // Заголовок локации
    const char* locNames[] = {"???", u8"УБЕЖИЩЕ", u8"НЕО-МОСКВА", u8"ПРОМЗОНА", u8"ТРУЩОБЫ", u8"ПОДЗЕМКА", u8"БАШНЯ NEXUS", u8"ФИНАЛ"};
    const char* locs[] = {"dark", "safe", "city", "ind", "slum", "under", "tower", "end"};
    const char* locName = "???";
    for (int i = 0; i < 8; i++) if (sc.loc == locs[i]) locName = locNames[i];
    
    DrawPanel(20, 12, 220, 45, C_CYAN);
    DrawNeonText(locName, 35, 20, 22, C_CYAN);
    
    // Статы
    DrawPanel(W - 320, 12, 300, 45, C_MAG);
    char stats[96];
    snprintf(stats, 96, u8"КРЕДИТЫ: %d   КАРМА: %d", gCredits, gKarma);
    DrawText2(stats, W - 305, 22, 16, C_WHITE);
    Color karmaCol = gKarma > 20 ? C_GREEN : gKarma < -20 ? C_RED : C_YELLOW;
    DrawRectangle(W - 80, 40, 60, 8, {40, 40, 40, 255});
    DrawRectangle(W - 80, 40, (int)(30 + gKarma * 0.3f), 8, karmaCol);
    
    // Диалоговое окно
    DrawPanel(40, 70, W - 80, H - 180, C_CYAN);
    
    if (gLine < (int)sc.dlg.size()) {
        DLine& dl = sc.dlg[gLine];
        if (gDispText.empty()) { gDispText = dl.text; gTypeIdx = 0; }
        
        int ty = 95;
        if (!dl.who.empty()) {
            Color nc = dl.who == "SYS" ? C_GREEN : dl.who == "GHOST" ? C_CYAN :
                       dl.who == "NEON" ? C_MAG : dl.who == "VIPER" ? C_RED :
                       dl.who == "MAYA" ? Color{255,150,0,255} : dl.who == "ORACLE" ? Color{150,0,255,255} :
                       dl.who == u8"ЛИ" ? C_YELLOW : dl.who == u8"СОЗДАТЕЛЬ" ? C_WHITE :
                       dl.player ? C_YELLOW : C_WHITE;
            DrawNeonText(dl.who.c_str(), 60, ty, 24, nc);
            ty += 40;
        }
        
        // Текст с переносом
        std::string vis = gDispText.substr(0, gTypeIdx);
        int lineH = 28, maxW = W - 160;
        std::string line;
        int ly = ty;
        for (char c : vis) {
            line += c;
            if (MeasureText2(line.c_str(), 20) > maxW || c == '\n') {
                if (c != '\n') { size_t sp = line.rfind(' '); if (sp != std::string::npos) line = line.substr(0, sp); }
                DrawText2(line.c_str(), 60, ly, 20, C_WHITE);
                ly += lineH;
                line.clear();
            }
        }
        if (!line.empty()) DrawText2(line.c_str(), 60, ly, 20, C_WHITE);
        
        // Курсор
        if (gTypeIdx < (int)gDispText.size() && (int)(gTime * 3) % 2) {
            DrawRectangle(60 + MeasureText2(line.c_str(), 20), ly, 12, 22, C_CYAN);
        }
        
        if (gTypeIdx >= (int)gDispText.size()) {
            float b = 0.5f + 0.5f * sinf(gTime * 4);
            DrawText2(u8"[ ENTER ]", W - 140, H - 130, 16, {200, 200, 200, (unsigned char)(b*255)});
        }
    } else {
        // Выборы
        DrawNeonText(u8"ВЫБОР:", 60, 95, 22, C_YELLOW);
        for (int i = 0; i < (int)sc.ch.size(); i++) {
            int y = 145 + i * 55;
            bool s = i == gMenuSel;
            if (s) {
                DrawRectangle(55, y - 8, W - 120, 48, {0, 255, 255, 25});
                DrawRectangleLinesEx({55, (float)(y - 8), (float)(W - 120), 48}, 2, C_CYAN);
            }
            char num[8]; snprintf(num, 8, "[%d]", i + 1);
            DrawText2(num, 65, y + 5, 20, s ? C_CYAN : C_GRAY);
            DrawText2(sc.ch[i].text.c_str(), 120, y + 5, 20, s ? C_WHITE : C_GRAY);
            if (sc.ch[i].karma > 0) DrawText2(u8"+Карма", W - 150, y + 5, 14, C_GREEN);
            if (sc.ch[i].karma < 0) DrawText2(u8"-Карма", W - 150, y + 5, 14, C_RED);
        }
    }
    
    // Лог
    DrawPanel(40, H - 100, W - 80, 85, C_CYAN);
    DrawText2(u8"ЛОГ:", 55, H - 92, 12, C_CYAN);
    int ly = H - 75;
    for (auto& l : gLog) {
        DrawText2(l.first.c_str(), 55, ly, 13, l.second);
        ly += 16;
        if (ly > H - 20) break;
    }
}

void DrawHack() {
    DrawBackground();
    DrawRectangle(0, 0, W, H, {0, 0, 0, 200});
    DrawHackGame(gHack, gFont);
}

void DrawEnd() {
    DrawBackground();
    DrawCity();
    DrawRectangle(0, 0, W, H, {0, 0, 0, 200});
    
    DrawNeonText(u8"СПАСИБО ЗА ИГРУ!", W/2 - 180, H/2 - 80, 40, C_CYAN);
    
    char buf[64];
    snprintf(buf, 64, u8"Финальная карма: %d", gKarma);
    DrawText2(buf, W/2 - 80, H/2, 20, gKarma > 0 ? C_GREEN : gKarma < 0 ? C_RED : C_YELLOW);
    
    snprintf(buf, 64, u8"Кредиты: %d", gCredits);
    DrawText2(buf, W/2 - 60, H/2 + 35, 18, C_YELLOW);
    
    const char* ending = gFlags.count("ending_free") ? u8"ОСВОБОДИТЕЛЬ" :
                         gFlags.count("ending_destroy") ? u8"ПАЛАЧ" : u8"НАБЛЮДАТЕЛЬ";
    DrawNeonText(ending, W/2 - MeasureText2(ending, 28)/2, H/2 + 80, 28, C_MAG);
    
    float b = 0.5f + 0.5f * sinf(gTime * 3);
    DrawText2(u8"[ ENTER - В МЕНЮ ]", W/2 - 90, H - 80, 18, {200, 200, 200, (unsigned char)(b*255)});
}

void UpdateGame(float dt) {
    UpdateVisuals(dt);
    gScrTime += dt;
    gTypeTimer += dt;
    
    switch (gScr) {
    case Scr::Intro:
        if (gScrTime > 1.5f && (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))) {
            gScr = Scr::Menu; gScrTime = 0; TriggerGlitch(0.5f);
            SpawnParticles({W/2.f, H/2.f}, C_CYAN, 30);
        }
        break;
        
    case Scr::Menu:
        if (IsKeyPressed(KEY_UP)) { gMenuSel = (gMenuSel - 1 + 4) % 4; SpawnParticles({W/2.f, 220.f + gMenuSel * 65}, C_CYAN, 5); }
        if (IsKeyPressed(KEY_DOWN)) { gMenuSel = (gMenuSel + 1) % 4; SpawnParticles({W/2.f, 220.f + gMenuSel * 65}, C_CYAN, 5); }
        if (IsKeyPressed(KEY_ENTER)) {
            SpawnParticles({W/2.f, 220.f + gMenuSel * 65}, C_CYAN, 25);
            TriggerGlitch(0.3f);
            if (gMenuSel == 0) {
                gScr = Scr::Game; gScene = 0; gLine = 0; gDispText.clear();
                gCredits = 100; gKarma = 0; gMenuSel = 0; gFlags.clear(); gLog.clear();
                InitSystems();
                AddLog(u8">>> Система инициализирована", C_CYAN);
                AddLog(u8">>> Добро пожаловать, оператор", C_GREEN);
            } else if (gMenuSel == 1) {
                gScr = Scr::Shop; gMenuSel = 0;
            } else if (gMenuSel == 3) CloseWindow();
        }
        break;
        
    case Scr::Game: {
        Scene& sc = gScenes[gScene];
        if (gLine < (int)sc.dlg.size()) {
            if (gTypeTimer > 0.018f && gTypeIdx < (int)gDispText.size()) {
                gTypeIdx++; gTypeTimer = 0;
                if (rand() % 8 == 0) SpawnParticles({60.f + gTypeIdx * 8.f, 150}, C_CYAN, 1);
            }
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                if (gTypeIdx < (int)gDispText.size()) gTypeIdx = gDispText.size();
                else { gLine++; gDispText.clear(); gTypeIdx = 0; SpawnParticles({W/2.f, H/2.f}, C_CYAN, 8); }
            }
        } else {
            int n = (int)sc.ch.size();
            if (n == 0) return;
            if (IsKeyPressed(KEY_UP)) { gMenuSel = (gMenuSel - 1 + n) % n; SpawnParticles({100, 145.f + gMenuSel * 55}, C_CYAN, 3); }
            if (IsKeyPressed(KEY_DOWN)) { gMenuSel = (gMenuSel + 1) % n; SpawnParticles({100, 145.f + gMenuSel * 55}, C_CYAN, 3); }
            for (int i = 0; i < std::min(n, 9); i++) if (IsKeyPressed(KEY_ONE + i)) gMenuSel = i;
            
            if (IsKeyPressed(KEY_ENTER)) {
                DChoice& c = sc.ch[gMenuSel];
                gKarma += c.karma;
                gCredits += sc.creditsReward;
                if (!sc.unlocks.empty()) gFlags[sc.unlocks] = true;
                if (c.karma > 0) AddLog(u8">>> +Карма", C_GREEN);
                if (c.karma < 0) AddLog(u8">>> -Карма", C_RED);
                if (sc.creditsReward > 0) AddLog(u8">>> +" + std::to_string(sc.creditsReward) + u8" кредитов", C_YELLOW);
                TriggerGlitch(0.25f);
                SpawnParticles({W/2.f, H/2.f}, C_CYAN, 20);
                
                gTotalChoices++;
                // Репутация фракций
                if (sc.unlocks == "ch2" || sc.unlocks == "ch4") gRep["ghost"] += 10;
                if (sc.unlocks == "shadow" || sc.unlocks == "oracle") gRep["shadow"] += 15;
                
                CheckAchievements();
                
                // Случайное событие между главами
                if (sc.unlocks.find("ch") == 0 && rand() % 3 == 0) {
                    TriggerRandomEvent();
                }
                
                if (c.next < 0) { gScr = Scr::End; gScrTime = 0; }
                else if (sc.hack) {
                    gScr = Scr::Hack;
                    InitHackGame(gHack, (HackType)sc.hackType, sc.diff);
                    AddLog(u8">>> Инициализация взлома...", C_CYAN);
                } else {
                    gScene = c.next; gLine = 0; gDispText.clear(); gMenuSel = 0;
                }
            }
        }
        break;
    }
    
    case Scr::Hack:
        UpdateHackGame(gHack, dt);
        if (IsHackComplete(gHack)) {
            gTotalHacks++;
            AddLog(u8">>> ВЗЛОМ УСПЕШЕН!", C_GREEN);
            int reward = 100 + gHack.difficulty * 50 + gHackBonus * 10;
            gCredits += reward;
            gRep["ghost"] += 5;
            CheckAchievements();
            SpawnParticles({W/2.f, H/2.f}, C_GREEN, 40);
            TriggerGlitch(0.3f);
            gScr = Scr::Game;
            gScene = gScenes[gScene].next;
            gLine = 0; gDispText.clear(); gMenuSel = 0;
        }
        if (IsHackFailed(gHack)) {
            AddLog(u8">>> ВЗЛОМ ПРОВАЛЕН!", C_RED);
            gKarma -= 5;
            TriggerGlitch(0.8f);
            gScr = Scr::Game;
            gLine = 0; gDispText.clear(); gMenuSel = 0;
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            gScr = Scr::Game;
            gLine = (int)gScenes[gScene].dlg.size();
            gMenuSel = 0;
        }
        break;
        
    case Scr::End:
        if (IsKeyPressed(KEY_ENTER)) { gScr = Scr::Menu; gScrTime = 0; gMenuSel = 0; }
        break;
        
    case Scr::Shop: {
        int n = (int)gUpgrades.size();
        if (IsKeyPressed(KEY_UP)) gMenuSel = (gMenuSel - 1 + n) % n;
        if (IsKeyPressed(KEY_DOWN)) gMenuSel = (gMenuSel + 1) % n;
        if (IsKeyPressed(KEY_ENTER)) {
            auto& u = gUpgrades[gMenuSel];
            int cost = u.cost * (u.level + 1);
            if (gCredits >= cost && u.level < u.maxLevel) {
                gCredits -= cost;
                u.level++;
                AddLog(u8">>> Куплено: " + u.name, C_GREEN);
                SpawnParticles({W/2.f, 120.f + gMenuSel * 80}, C_GREEN, 20);
                // Применить бонусы
                if (u.id == "neural") gHackBonus += 15;
                if (u.id == "stealth") gTraceReduce += 20;
                if (u.id == "chrono") gTimeBonus += 10;
            }
        }
        if (IsKeyPressed(KEY_ESCAPE)) { gScr = Scr::Menu; gMenuSel = 0; }
        break;
    }
    
    case Scr::Event:
        if (IsKeyPressed(KEY_ENTER)) {
            gCredits += gCurrentEvent.credits;
            gKarma += gCurrentEvent.karma;
            if (!gCurrentEvent.faction.empty()) gRep[gCurrentEvent.faction] += gCurrentEvent.rep;
            if (gCurrentEvent.credits > 0) AddLog(u8">>> +" + std::to_string(gCurrentEvent.credits) + u8" кредитов", C_GREEN);
            if (gCurrentEvent.credits < 0) AddLog(u8">>> " + std::to_string(gCurrentEvent.credits) + u8" кредитов", C_RED);
            gScr = Scr::Game;
        }
        break;
    }
}
