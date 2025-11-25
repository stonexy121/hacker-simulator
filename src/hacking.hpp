#pragma once
#include "raylib.h"
#include <string>
#include <vector>

// Типы мини-игр взлома
enum class HackType { 
    CodeBreak,    // Подбор кода (как Mastermind)
    MemoryGrid,   // Запоминание последовательности
    PacketRoute,  // Маршрутизация пакетов
    Decrypt,      // Расшифровка паттерна
    Firewall      // Обход файрвола (реакция)
};

struct HackGame {
    HackType type;
    int difficulty;  // 1-5
    float timeLimit;
    
    // CodeBreak
    std::string secretCode;
    std::string playerCode;
    int attempts;
    int maxAttempts;
    std::vector<std::pair<int,int>> history; // correct pos, correct digit
    
    // MemoryGrid
    std::vector<int> sequence;
    std::vector<int> playerSeq;
    int showPhase;  // 0=showing, 1=input
    float showTimer;
    int currentShow;
    
    // PacketRoute
    int gridW, gridH;
    std::vector<int> grid;  // 0=empty, 1=wall, 2=start, 3=end, 4=path
    int packetX, packetY;
    int targetX, targetY;
    std::vector<int> playerPath;
    
    // Decrypt
    std::string encrypted;
    std::string decrypted;
    std::string pattern;
    int patternPos;
    
    // Firewall
    std::vector<float> barriers;  // Y positions
    float playerY;
    float scrollSpeed;
    int passed;
    int required;
    
    // Общее
    bool completed;
    bool failed;
    float timer;
    int trace;
    int score;
};

void InitHackGame(HackGame& h, HackType type, int difficulty);
void UpdateHackGame(HackGame& h, float dt);
void DrawHackGame(const HackGame& h, Font& font);
bool IsHackComplete(const HackGame& h);
bool IsHackFailed(const HackGame& h);
