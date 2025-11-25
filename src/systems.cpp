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
    
    // Заголовок
    int headerH = Sc(60);
    DrawPanel(Sc(50), Sc(30), W - Sc(100), headerH, C_CYAN);
    DrawNeonText(u8"[ ЧЁРНЫЙ РЫНОК ]", Sc(70), Sc(45), Sc(28), C_CYAN);
    
    char buf[64];
    snprintf(buf, 64, u8"💰 %d", gCredits);
    DrawText2(buf, W - Sc(200), Sc(50), Sc(20), C_YELLOW);
    
    // Улучшения
    int itemH = Sc(70);
    int itemPad = Sc(10);
    int startY = Sc(120);
    int itemX = Sc(50);
    int itemW = W - Sc(100);
    
    for (int i = 0; i < (int)gUpgrades.size(); i++) {
        auto& u = gUpgrades[i];
        int y = startY + i * (itemH + itemPad);
        bool sel = i == gMenuSel;
        bool canBuy = gCredits >= u.cost * (u.level + 1) && u.level < u.maxLevel;
        
        Color borderCol = sel ? C_CYAN : C_GRAY;
        if (u.level >= u.maxLevel) borderCol = C_GREEN;
        
        // Фон кнопки
        Color bg = sel ? Color{0, 40, 60, 220} : Color{15, 25, 40, 200};
        DrawRectangleRounded({(float)itemX, (float)y, (float)itemW, (float)itemH}, 0.1f, 8, bg);
        DrawRectangleRoundedLinesEx({(float)itemX, (float)y, (float)itemW, (float)itemH}, 0.1f, 8, 2, borderCol);
        
        // Название и описание
        DrawText2(u.name.c_str(), itemX + Sc(20), y + Sc(12), Sc(18), sel ? C_WHITE : C_GRAY);
        DrawText2(u.desc.c_str(), itemX + Sc(20), y + Sc(38), Sc(14), C_GRAY);
        
        // Уровень (точки)
        int dotX = W - Sc(200);
        for (int l = 0; l < u.maxLevel; l++) {
            Color lc = l < u.level ? C_GREEN : Color{40, 50, 60, 255};
            DrawCircle(dotX + l * Sc(25), y + Sc(35), Sc(8), lc);
            DrawCircleLines(dotX + l * Sc(25), y + Sc(35), Sc(8), C_CYAN);
        }
        
        // Цена или статус
        if (u.level < u.maxLevel) {
            snprintf(buf, 64, "%d", u.cost * (u.level + 1));
            DrawText2(buf, W - Sc(120), y + Sc(25), Sc(18), canBuy ? C_YELLOW : C_RED);
        } else {
            DrawText2(u8"✓", W - Sc(100), y + Sc(20), Sc(24), C_GREEN);
        }
    }
    
    // Кнопка "Назад" - большая для пальцев
    int backY = H - Sc(80);
    int backH = Sc(60);
    DrawRectangleRounded({(float)itemX, (float)backY, (float)itemW, (float)backH}, 0.15f, 8, {60, 20, 20, 220});
    DrawRectangleRoundedLinesEx({(float)itemX, (float)backY, (float)itemW, (float)backH}, 0.15f, 8, 2, C_RED);
    
    const char* backText = u8"← НАЗАД";
    int backTextW = MeasureText2(backText, Sc(22));
    DrawText2(backText, itemX + (itemW - backTextW)/2, backY + Sc(18), Sc(22), C_WHITE);
}

void DrawEvent() {
    DrawBackground();
    DrawCity();
    DrawRectangle(0, 0, W, H, {0, 0, 0, 220});
    
    int panelX = Sc(80);
    int panelW = W - Sc(160);
    int panelH = Sc(280);
    int panelY = (H - panelH) / 2;
    
    DrawPanel(panelX, panelY, panelW, panelH, C_CYAN);
    
    DrawNeonText(u8"[ СОБЫТИЕ ]", W/2 - Sc(80), panelY + Sc(20), Sc(24), C_CYAN);
    
    // Текст события с переносом
    std::string text = gCurrentEvent.text;
    int maxW = panelW - Sc(60);
    int fontSize = Sc(18);
    std::string line;
    int ly = panelY + Sc(70);
    for (size_t i = 0; i < text.size(); i++) {
        line += text[i];
        if (MeasureText2(line.c_str(), fontSize) > maxW || text[i] == '\n') {
            DrawText2(line.c_str(), panelX + Sc(30), ly, fontSize, C_WHITE);
            ly += Sc(28);
            line.clear();
        }
    }
    if (!line.empty()) DrawText2(line.c_str(), panelX + Sc(30), ly, fontSize, C_WHITE);
    
    // Эффекты
    ly = panelY + Sc(160);
    char buf[64];
    if (gCurrentEvent.credits != 0) {
        snprintf(buf, 64, "%s%d кредитов", gCurrentEvent.credits > 0 ? "+" : "", gCurrentEvent.credits);
        DrawText2(buf, panelX + Sc(30), ly, Sc(16), gCurrentEvent.credits > 0 ? C_GREEN : C_RED);
        ly += Sc(24);
    }
    if (gCurrentEvent.karma != 0) {
        snprintf(buf, 64, "%s%d карма", gCurrentEvent.karma > 0 ? "+" : "", gCurrentEvent.karma);
        DrawText2(buf, panelX + Sc(30), ly, Sc(16), gCurrentEvent.karma > 0 ? C_GREEN : C_RED);
        ly += Sc(24);
    }
    if (gCurrentEvent.rep != 0 && !gCurrentEvent.faction.empty()) {
        const char* fname = gCurrentEvent.faction == "ghost" ? "Ghost Protocol" : "Shadow Brokers";
        snprintf(buf, 64, "%s%d репутация (%s)", gCurrentEvent.rep > 0 ? "+" : "", gCurrentEvent.rep, fname);
        DrawText2(buf, panelX + Sc(30), ly, Sc(16), gCurrentEvent.rep > 0 ? C_CYAN : C_RED);
    }
    
    // Кнопка продолжить
    float b = 0.5f + 0.5f * sinf(gTime * 3);
    const char* contText = u8"[ КОСНИТЕСЬ ДЛЯ ПРОДОЛЖЕНИЯ ]";
    int contW = MeasureText2(contText, Sc(18));
    DrawText2(contText, (W - contW)/2, panelY + panelH - Sc(40), Sc(18), {200, 200, 200, (unsigned char)(b * 255)});
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
