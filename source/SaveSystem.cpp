#include "SaveSystem.h"
#include "raylib.h"
#include <cstring>
#include <string>

void SaveSystem::load(SaveData &data)
{
    int bytesRead = 0;
    unsigned char *fileData = LoadFileData(SAVE_FILENAME, &bytesRead);

    if (fileData != nullptr && bytesRead > 0)
    {
        std::string json(reinterpret_cast<char *>(fileData), bytesRead);

        // Парсим "maxLevel"
        size_t pos = json.find("\"maxLevel\"");
        if (pos != std::string::npos)
        {
            pos = json.find(':', pos);
            if (pos != std::string::npos)
            {
                pos++;
                while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t'))
                    pos++;
                int value = 0;
                while (pos < json.length() && json[pos] >= '0' && json[pos] <= '9')
                {
                    value = value * 10 + (json[pos] - '0');
                    pos++;
                }
                data.maxLevel = value;
            }
        }

        // Парсим "gameCompleted"
        pos = json.find("\"gameCompleted\"");
        if (pos != std::string::npos)
        {
            pos = json.find(':', pos);
            if (pos != std::string::npos)
            {
                pos++;
                while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t'))
                    pos++;
                if (pos + 4 <= json.length() && json.substr(pos, 4) == "true")
                    data.gameCompleted = true;
                else
                    data.gameCompleted = false;
            }
        }

        UnloadFileData(fileData);
        TraceLog(LOG_INFO, "Save data loaded: maxLevel=%d, gameCompleted=%d",
                 data.maxLevel, data.gameCompleted ? 1 : 0);
    }
    else
    {
        data.maxLevel = 10;
        data.gameCompleted = false;
        save(data);
        TraceLog(LOG_INFO, "No save file found, created new save with maxLevel=10");
    }
}

void SaveSystem::save(const SaveData &data)
{
    char json[256];
    snprintf(json, sizeof(json),
             "{\n  \"maxLevel\": %d,\n  \"gameCompleted\": %s\n}\n",
             data.maxLevel, data.gameCompleted ? "true" : "false");

    unsigned int dataSize = static_cast<unsigned int>(strlen(json));
    bool success = SaveFileData(SAVE_FILENAME, json, dataSize);

    if (success)
    {
        TraceLog(LOG_INFO, "Save data saved: maxLevel=%d, gameCompleted=%d",
                 data.maxLevel, data.gameCompleted ? 1 : 0);
    }
    else
    {
        TraceLog(LOG_ERROR, "Failed to save game data!");
    }
}