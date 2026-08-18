#pragma once
#include "third_party/json.hpp"
#include <string>

struct SaveData
{
    int maxLevel = 10;          // Максимальный доступный уровень (sav(0))
    int currentSquad = 1;       // Текущий выбранный сквад
    bool gameCompleted = false; // Флаг полного прохождения игры (sav(1))

    // JSON сериализация
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(SaveData, maxLevel, currentSquad, gameCompleted)
};

class SaveSystem
{
public:
    static void load(SaveData &data);
    static void save(const SaveData &data);

private:
    static constexpr const char *SAVE_FILENAME = "save.json";
};