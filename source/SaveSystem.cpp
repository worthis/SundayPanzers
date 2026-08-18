#include "SaveSystem.h"
#include "raylib.h"
#include <fstream>

void SaveSystem::load(SaveData &data)
{
    std::ifstream file(SAVE_FILENAME);

    if (!file.is_open())
    {
        // Файла нет - создаем новый с дефолтными значениями
        TraceLog(LOG_WARNING, "Save file not found, creating new save with defaults");
        data = SaveData{};
        save(data);
        return;
    }

    nlohmann::json j;
    file >> j;

    // Вся десериализация — макросом (from_json)
    data = j.get<SaveData>();

    TraceLog(LOG_INFO, "Save loaded");
}

void SaveSystem::save(const SaveData &data)
{
    // Вся сериализация — макросом (to_json)
    nlohmann::json j = data;

    std::ofstream file(SAVE_FILENAME);
    if (!file.is_open())
    {
        TraceLog(LOG_ERROR, "Cannot open %s for writing", SAVE_FILENAME);
        return;
    }

    file << j.dump(2) << std::endl;

    TraceLog(LOG_INFO, "Save written");
}