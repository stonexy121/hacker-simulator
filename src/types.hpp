#pragma once
#include "raylib.h"
#include <string>
#include <vector>
#include <deque>
#include <map>

constexpr int W = 1280, H = 720;

// Цвета
inline const Color C_BG = {5,8,15,255};
inline const Color C_PANEL = {12,18,30,240};
inline const Color C_CYAN = {0,255,255,255};
inline const Color C_MAG = {255,0,128,255};
inline const Color C_GREEN = {0,255,100,255};
inline const Color C_RED = {255,50,50,255};
inline const Color C_YELLOW = {255,200,0,255};
inline const Color C_WHITE = {240,245,255,255};
inline const Color C_GRAY = {100,120,140,255};

// Структуры
struct Particle { Vector2 p,v; Color c; float life,size; };
struct Rain { float x,y,spd,len; };

struct DLine { std::string who, text; bool player; };
struct DChoice { std::string text; int next, karma; std::string req; int hackType; };
struct Scene { 
    int id; 
    std::string loc; 
    std::vector<DLine> dlg; 
    std::vector<DChoice> ch; 
    bool hack; 
    int diff; 
    int hackType; // 0-4 = разные мини-игры
    int next; 
    std::string unlocks;
    int creditsReward;
};

// Экраны
enum class Scr { Intro, Menu, Game, Hack, Shop, Event, End };

// Улучшения
struct Upgrade {
    std::string id, name, desc;
    int cost, level, maxLevel;
    bool owned;
};

// Достижения
struct Achievement {
    std::string id, name, desc;
    bool unlocked;
};

// Случайные события
struct RandEvent {
    std::string text;
    int credits, karma, rep;
    std::string faction;
};

// Глобальные переменные (extern)
extern Font gFont;
extern float gTime, gPulse, gGlitch, gShake;
extern std::vector<Particle> gParts;
extern std::vector<Rain> gRain;
extern std::vector<float> gMatrix;
extern Scr gScr;
extern float gScrTime;
extern int gMenuSel, gCredits, gKarma, gScene, gLine;
extern float gTypeTimer;
extern int gTypeIdx;
extern std::string gDispText;
extern int gHackProg, gHackTrace;
extern float gHackTime;
extern std::map<std::string,bool> gFlags;
extern std::deque<std::pair<std::string,Color>> gLog;
extern std::vector<Scene> gScenes;
extern int gHackType;

// Новые системы
extern std::vector<Upgrade> gUpgrades;
extern std::vector<Achievement> gAchievements;
extern std::vector<RandEvent> gEvents;
extern std::map<std::string,int> gRep; // репутация фракций
extern int gHackBonus, gTimeBonus, gTraceReduce;
extern RandEvent gCurrentEvent;
extern int gTotalHacks, gTotalChoices;

// Функции
void InitStory();
void InitVisuals();
void UpdateVisuals(float dt);
void SpawnParticles(Vector2 pos, Color c, int n);
void TriggerGlitch(float d);
void TriggerShake(float i);
void AddLog(const std::string& t, Color c);

void DrawBackground();
void DrawCity();
void DrawNeonText(const char* t, int x, int y, int s, Color c);
void DrawPanel(int x, int y, int w, int h, Color c);
void DrawText2(const char* t, int x, int y, int s, Color c);
int MeasureText2(const char* t, int s);

void DrawIntro();
void DrawMenu();
void DrawGame();
void DrawHack();
void DrawShop();
void DrawEvent();
void DrawEnd();
void DrawHUD();
void DrawEffects();

void UpdateGame(float dt);
void InitSystems();
void CheckAchievements();
void TriggerRandomEvent();
