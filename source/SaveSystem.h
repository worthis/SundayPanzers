#pragma once
#include <string>

struct SaveData
{
    int maxLevel = 10;          // Максимальный доступный уровень (sav(0))
    bool gameCompleted = false; // Флаг полного прохождения игры (sav(1))
};

class SaveSystem
{
public:
    static void load(SaveData &data);
    static void save(const SaveData &data);

private:
    static constexpr const char *SAVE_FILENAME = "save.json";
};