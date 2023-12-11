#include "../cjson/cJSON.h"
#include "Rogue.h"
#include "GlobalsBase.h"

const char* JSON_FILENAME = "brogue_config.json";

typedef struct configParams {
    short playbackDelayPerTurn;
    short gameVariant;
    short graphicsMode;
    boolean displayStealthRangeMode;
    boolean trueColorMode;
    boolean wizard;
    boolean easyMode;
} configParams;

typedef enum {
    INT_TYPE,
    BOOLEAN_TYPE,
    GAME_VARIANT,
    GRAPHICS_MODE,
} fieldType;

typedef struct {
    const char* fieldName;
    void* fieldPointer;
    fieldType fieldType;
    const char** stringMapping;
} configEntry;

const char* variantMappings[] = {
    "brogue",
    "rapid",
    NULL
};

const char* graphicsModeMappings[] = {
    "text",
    "tiles",
    "hybrid",
    NULL
};

static configParams createDefaultConfig() {
    configParams config;

    config.playbackDelayPerTurn = DEFAULT_PLAYBACK_DELAY;
    config.gameVariant = VARIANT_BROGUE;
    config.graphicsMode = TEXT_GRAPHICS;
    config.displayStealthRangeMode = false;
    config.trueColorMode = false;
    config.easyMode = false;
    config.wizard = false;

    return config;
}

static char* loadConfigFile() {
    FILE* jsonFile = fopen(JSON_FILENAME, "r");

    if (!jsonFile) {
        return NULL;
    }

    fseek(jsonFile, 0, SEEK_END);
    long fileSize = ftell(jsonFile);
    fseek(jsonFile, 0, SEEK_SET);

    char* buffer = (char*)malloc(fileSize + 1);

    if (!buffer) {
        fclose(jsonFile);
        return NULL;
    }

    fread(buffer, 1, fileSize, jsonFile);
    fclose(jsonFile);

    buffer[fileSize] = '\0';

    return buffer;
}

static configEntry* getFieldEntries(configParams* config) {
    int numFields = 7;

    configEntry* entries = calloc(numFields + 1, sizeof(configEntry));

    entries[0] = (configEntry){ "gameVariant", &(config->gameVariant), GAME_VARIANT, variantMappings };
    entries[1] = (configEntry){ "graphicsMode", &(config->graphicsMode), GRAPHICS_MODE, graphicsModeMappings };
    entries[2] = (configEntry){ "playbackDelayPerTurn", &(config->playbackDelayPerTurn), INT_TYPE };
    entries[3] = (configEntry){ "displayStealthRangeMode", &(config->displayStealthRangeMode), BOOLEAN_TYPE };
    entries[4] = (configEntry){ "trueColorMode", &(config->trueColorMode), BOOLEAN_TYPE };
    entries[5] = (configEntry){ "wizard", &(config->wizard), BOOLEAN_TYPE };
    entries[6] = (configEntry){ "easyMode", &(config->easyMode), BOOLEAN_TYPE };
    entries[numFields] = (configEntry){ NULL };

    return entries;
}

static short mapStringToEnum(const char* inputString, const char** mappings) {
    for (short i = 0; mappings[i]; i++) {
        if (strcmp(inputString, mappings[i]) == 0) {
            return i;
        }
    }
    return -1;
}

static void parseConfigValues(const char* jsonString, configParams* config) {
    if (!jsonString || !config) {
        return; // Invalid input
    }

    cJSON* root = cJSON_Parse(jsonString);

    if (!root) {
        return; // JSON parsing error
    }

    configEntry* entries = getFieldEntries(config);

    for (int i = 0; entries[i].fieldName; i++) {
        cJSON* jsonField = cJSON_GetObjectItem(root, entries[i].fieldName);

        if (jsonField) {
            switch (entries[i].fieldType) {
                case INT_TYPE:
                    *((short*)entries[i].fieldPointer) = jsonField->valueint;
                    break;

                case BOOLEAN_TYPE:
                    *((boolean*)entries[i].fieldPointer) = jsonField->valueint;
                    break;

                case GAME_VARIANT:
                case GRAPHICS_MODE:
                {
                    const char* modeString = jsonField->valuestring;
                    short mode = mapStringToEnum(modeString, entries[i].stringMapping);

                    if (mode != -1) {
                        *((short*)entries[i].fieldPointer) = mode;
                    }
                    break;
                }

                default:
                    break;
            }
        }
    }

    free(entries);
    cJSON_Delete(root);
}

static char* createJsonString(configParams* config) {
    cJSON* root = cJSON_CreateObject();

    configEntry* entries = getFieldEntries(config);

    for (int i = 0; entries[i].fieldName; i++) {
        switch (entries[i].fieldType) {
            case INT_TYPE: {
                short short_value = *((short*)entries[i].fieldPointer);
                cJSON_AddNumberToObject(root, entries[i].fieldName, short_value);
                break;
            }
            case BOOLEAN_TYPE: {
                boolean bool_value = *((boolean*)entries[i].fieldPointer);
                cJSON_AddBoolToObject(root, entries[i].fieldName, bool_value);
                break;
            }
            case GAME_VARIANT:
            case GRAPHICS_MODE:
            {
                short enum_value = *((short*)entries[i].fieldPointer);
                const char* string_value = entries[i].stringMapping[enum_value];
                cJSON_AddStringToObject(root, entries[i].fieldName, string_value);
                break;
            }

            default:
                break;
        }

    }

    char* jsonString = cJSON_Print(root);

    free(entries);
    cJSON_Delete(root);

    return jsonString;
}

void readFromConfig(playerCharacter* rogue, enum graphicsModes* initialGraphics) {
    char* jsonString = loadConfigFile();

    configParams config = createDefaultConfig();

    parseConfigValues(jsonString, &config);

    rogue->wizard = config.wizard;
    rogue->easyMode = config.easyMode;
    rogue->displayStealthRangeMode = config.displayStealthRangeMode;
    rogue->trueColorMode = config.trueColorMode;
    rogue->playbackDelayPerTurn = config.playbackDelayPerTurn;

    gameVariant = config.gameVariant;
    *initialGraphics = config.graphicsMode;

    free(jsonString);
}

void writeIntoConfig() {
    configParams config;

    FILE* file = fopen(JSON_FILENAME, "w");

    if (!file) {
        return;
    }

    config.wizard = rogue.wizard;
    config.easyMode = rogue.easyMode;
    config.displayStealthRangeMode = rogue.displayStealthRangeMode;
    config.trueColorMode = rogue.trueColorMode;
    config.playbackDelayPerTurn = rogue.playbackDelayPerTurn;

    config.gameVariant = gameVariant;
    config.graphicsMode = graphicsMode;

    char* jsonString = createJsonString(&config);

    fprintf(file, "%s", jsonString);

    fclose(file);
    free(jsonString);
}