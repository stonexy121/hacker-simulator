#include "types.hpp"
#include <cstdlib>

// Глобальные переменные новых систем
std::vector<Upgrade> gUpgrades;
std::vector<Achievement> gAchievements;
std::vector<RandEvent> gEvents;
std::map<std::string,int> gRep;
int gHackBonus = 0, gTimeBonus = 0, gTraceReduce = 0;
RandEvent gCurrentEvent;
int gTotalHacks = 0, gTotalChoices = 0;

void InitSystems() {
    // Улучшения
    gUpgrades = {
        {"neural", u8"Нейроускоритель", u8"Взлом +15% к прогрессу", 500, 0, 3, false},
        {"stealth", u8"Стелс-модуль", u8"След -20% при взломе", 400, 0, 3, false},
        {"chrono", u8"Хроно-имплант", u8"Время взлома +10 сек", 600, 0, 3, false},
        {"decrypt", u8"Дешифратор", u8"Упрощает мини-игры", 800, 0, 2, false},
        {"firewall", u8"Личный файрвол", u8"Защита от обнаружения", 1000, 0, 1, false},
        {"ghost", u8"Протокол GHOST", u8"Невидимость в сети", 2000, 0, 1, false}
    };
    
    // Достижения
    gAchievements = {
        {"first_hack", u8"Первый взлом", u8"Успешно взломай систему", false},
        {"speed_demon", u8"Скоростной демон", u8"Взломай за 15 секунд", false},
        {"ghost_mode", u8"Призрак", u8"Взломай без следа", false},
        {"rich", u8"Богач", u8"Накопи 5000 кредитов", false},
        {"hero", u8"Герой", u8"Карма выше +50", false},
        {"villain", u8"Злодей", u8"Карма ниже -50", false},
        {"ally_ghost", u8"Друг GHOST", u8"Максимум репутации с Ghost Protocol", false},
        {"ally_shadow", u8"Друг теней", u8"Максимум репутации с Shadow Brokers", false},
        {"completist", u8"Завершитель", u8"Пройди игру", false},
        {"hacker_10", u8"Хакер", u8"Соверши 10 взломов", false}
    };
    
    // Случайные события
    gEvents = {
        {u8"Анонимный донат! Кто-то оценил твою работу.", 200, 5, 0, ""},
        {u8"Nexus усилил патрули. Будь осторожнее.", -50, 0, 0, ""},
        {u8"Ghost Protocol прислал припасы.", 150, 0, 10, "ghost"},
        {u8"Shadow Brokers поделились информацией.", 100, 0, 10, "shadow"},
        {u8"Твой старый контакт вышел на связь.", 75, 5, 0, ""},
        {u8"Обнаружена уязвимость в системе Nexus!", 0, 0, 15, "ghost"},
        {u8"Наёмники потеряли твой след.", 0, 10, 0, ""},
        {u8"Нашёл заначку в старом убежище.", 300, 0, 0, ""},
        {u8"Информатор слил данные о тебе.", -100, -5, -10, ""},
        {u8"Удачный день на чёрном рынке.", 250, 0, 5, "shadow"},
        {u8"NEON прислала полезные скрипты.", 0, 5, 10, "ghost"},
        {u8"Перехвачен зашифрованный пакет.", 100, 0, 0, ""}
    };
    
    // Репутация фракций
    gRep["ghost"] = 0;   // Ghost Protocol
    gRep["shadow"] = 0;  // Shadow Brokers
    gRep["nexus"] = -50; // Nexus Corp (враги)
    
    gHackBonus = 0;
    gTimeBonus = 0;
    gTraceReduce = 0;
    gTotalHacks = 0;
    gTotalChoices = 0;
}

void CheckAchievements() {
    for (auto& a : gAchievements) {
        if (a.unlocked) continue;
        
        bool unlock = false;
        if (a.id == "first_hack" && gTotalHacks >= 1) unlock = true;
        if (a.id == "hacker_10" && gTotalHacks >= 10) unlock = true;
        if (a.id == "rich" && gCredits >= 5000) unlock = true;
        if (a.id == "hero" && gKarma >= 50) unlock = true;
        if (a.id == "villain" && gKarma <= -50) unlock = true;
        if (a.id == "ally_ghost" && gRep["ghost"] >= 50) unlock = true;
        if (a.id == "ally_shadow" && gRep["shadow"] >= 50) unlock = true;
        if (a.id == "completist" && gFlags.count("ending_free")) unlock = true;
        
        if (unlock) {
            a.unlocked = true;
            AddLog(u8">>> ДОСТИЖЕНИЕ: " + a.name, C_YELLOW);
            gCredits += 100; // Бонус за достижение
            TriggerGlitch(0.3f);
            SpawnParticles({W/2.f, 50}, C_YELLOW, 30);
        }
    }
}

void TriggerRandomEvent() {
    if (gEvents.empty()) return;
    gCurrentEvent = gEvents[rand() % gEvents.size()];
    gScr = Scr::Event;
    gScrTime = 0;
}

void DrawShop() {
    DrawBackground();
    DrawRectangle(0, 0, W, H, {0, 0, 0, 200});
    
    DrawPanel(50, 40, W - 100, 60, C_CYAN);
    DrawNeonText(u8"[ ЧЁРНЫЙ РЫНОК ]", 70, 55, 32, C_CYAN);
    
    char buf[64];
    snprintf(buf, 64, u8"Кредиты: %d", gCredits);
    DrawText2(buf, W - 250, 60, 20, C_YELLOW);
    
    int y = 120;
    for (int i = 0; i < (int)gUpgrades.size(); i++) {
        auto& u = gUpgrades[i];
        bool sel = i == gMenuSel;
        bool canBuy = gCredits >= u.cost && u.level < u.maxLevel;
        
        Color borderCol = sel ? C_CYAN : C_GRAY;
        if (u.level >= u.maxLevel) borderCol = C_GREEN;
        
        DrawPanel(60, y, W - 120, 70, borderCol);
        
        // Название и описание
        DrawText2(u.name.c_str(), 80, y + 10, 20, sel ? C_WHITE : C_GRAY);
        DrawText2(u.desc.c_str(), 80, y + 35, 14, C_GRAY);
        
        // Уровень
        for (int l = 0; l < u.maxLevel; l++) {
            Color lc = l < u.level ? C_GREEN : Color{40, 50, 60, 255};
            DrawRectangle(W - 280 + l * 25, y + 15, 20, 20, lc);
            DrawRectangleLinesEx({(float)(W - 280 + l * 25), (float)(y + 15), 20, 20}, 1, C_CYAN);
        }
        
        // Цена
        if (u.level < u.maxLevel) {
            snprintf(buf, 64, "%d", u.cost * (u.level + 1));
            DrawText2(buf, W - 150, y + 25, 18, canBuy ? C_YELLOW : C_RED);
            DrawText2(u8"кр", W - 100, y + 28, 14, C_GRAY);
        } else {
            DrawText2(u8"МАКС", W - 140, y + 25, 16, C_GREEN);
        }
        
        y += 80;
    }
    
    DrawPanel(50, H - 70, W - 100, 50, C_CYAN);
    DrawText2(u8"[↑↓] Выбор   [ENTER] Купить   [ESC] Назад", 70, H - 55, 16, C_GRAY);
}

void DrawEvent() {
    DrawBackground();
    DrawCity();
    DrawRectangle(0, 0, W, H, {0, 0, 0, 220});
    
    DrawPanel(100, H/2 - 120, W - 200, 240, C_CYAN);
    
    DrawNeonText(u8"[ СОБЫТИЕ ]", W/2 - 80, H/2 - 100, 24, C_CYAN);
    
    // Текст события с переносом
    std::string text = gCurrentEvent.text;
    int maxW = W - 280;
    std::string line;
    int ly = H/2 - 50;
    for (size_t i = 0; i < text.size(); i++) {
        line += text[i];
        if (MeasureText2(line.c_str(), 18) > maxW || text[i] == '\n') {
            DrawText2(line.c_str(), 130, ly, 18, C_WHITE);
            ly += 25;
            line.clear();
        }
    }
    if (!line.empty()) DrawText2(line.c_str(), 130, ly, 18, C_WHITE);
    
    // Эффекты
    ly = H/2 + 30;
    char buf[64];
    if (gCurrentEvent.credits != 0) {
        snprintf(buf, 64, "%s%d кредитов", gCurrentEvent.credits > 0 ? "+" : "", gCurrentEvent.credits);
        DrawText2(buf, 130, ly, 16, gCurrentEvent.credits > 0 ? C_GREEN : C_RED);
        ly += 22;
    }
    if (gCurrentEvent.karma != 0) {
        snprintf(buf, 64, "%s%d карма", gCurrentEvent.karma > 0 ? "+" : "", gCurrentEvent.karma);
        DrawText2(buf, 130, ly, 16, gCurrentEvent.karma > 0 ? C_GREEN : C_RED);
        ly += 22;
    }
    if (gCurrentEvent.rep != 0 && !gCurrentEvent.faction.empty()) {
        const char* fname = gCurrentEvent.faction == "ghost" ? "Ghost Protocol" : "Shadow Brokers";
        snprintf(buf, 64, "%s%d репутация (%s)", gCurrentEvent.rep > 0 ? "+" : "", gCurrentEvent.rep, fname);
        DrawText2(buf, 130, ly, 16, gCurrentEvent.rep > 0 ? C_CYAN : C_RED);
    }
    
    float b = 0.5f + 0.5f * sinf(gTime * 3);
    DrawText2(u8"[ ENTER - Продолжить ]", W/2 - 100, H/2 + 90, 16, {200, 200, 200, (unsigned char)(b * 255)});
}

void DrawHUD() {
    // Мини-панель статуса (всегда видна в игре)
    // Репутация фракций
    int rx = W - 200;
    DrawText2(u8"Репутация:", rx, 70, 12, C_GRAY);
    
    char buf[32];
    snprintf(buf, 32, "GP: %d", gRep["ghost"]);
    DrawText2(buf, rx, 85, 11, gRep["ghost"] > 0 ? C_CYAN : C_GRAY);
    
    snprintf(buf, 32, "SB: %d", gRep["shadow"]);
    DrawText2(buf, rx + 60, 85, 11, gRep["shadow"] > 0 ? C_MAG : C_GRAY);
    
    // Бонусы от улучшений
    if (gHackBonus > 0 || gTimeBonus > 0 || gTraceReduce > 0) {
        int bx = 240;
        if (gHackBonus > 0) {
            snprintf(buf, 32, "+%d%%", gHackBonus);
            DrawText2(buf, bx, 25, 11, C_GREEN);
            bx += 40;
        }
        if (gTraceReduce > 0) {
            snprintf(buf, 32, "-%d%%T", gTraceReduce);
            DrawText2(buf, bx, 25, 11, C_CYAN);
        }
    }
}
