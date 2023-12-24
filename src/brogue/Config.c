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
    ENUM_STRING // a json string that needs to be mapped to an Enum
} fieldType;

typedef struct {
    int min;
    int max;
} intRange;

typedef union {
    const char** enumMappings;
    intRange intRange;
} fieldValidator;

typedef struct {
    const char* name;
    void* paramPointer; // a pointer to a field of configParams struct
    fieldType type;
    fieldValidator validator;
} configField;

// maps json strings to the gameVariant Enum
const char* variantMappings[] = {"brogue", "rapid", NULL};

// maps json strings to the graphicsModes Enum
const char* graphicsModeMappings[] = {"text", "tiles", "hybrid", NULL};

intRange playbackDelayRange = {MIN_PLAYBACK_DELAY, MAX_PLAYBACK_DELAY};

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

static configField* getFieldEntries(configParams* config) {
    int numFields = 7;

    configField fieldDescriptors[] = {
        {"gameVariant", &(config->gameVariant), ENUM_STRING, {.enumMappings = variantMappings}},
        {"graphicsMode", &(config->graphicsMode), ENUM_STRING, {.enumMappings = graphicsModeMappings}},
        {"playbackDelayPerTurn", &(config->playbackDelayPerTurn), INT_TYPE, {.intRange = playbackDelayRange}},
        {"displayStealthRangeMode", &(config->displayStealthRangeMode), BOOLEAN_TYPE},
        {"trueColorMode", &(config->trueColorMode), BOOLEAN_TYPE},
        {"wizard", &(config->wizard), BOOLEAN_TYPE},
        {"easyMode", &(config->easyMode), BOOLEAN_TYPE},
        {NULL}};

    configField* entries = calloc(numFields + 1, sizeof(configField));

    for (int i = 0; i < numFields; i++) {
        entries[i] = fieldDescriptors[i];
    }

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

    configField* entries = getFieldEntries(config);

    for (int i = 0; entries[i].name; i++) {
        cJSON* jsonField = cJSON_GetObjectItem(root, entries[i].name);

        if (jsonField) {
            switch (entries[i].type) {
            case INT_TYPE:
                if (cJSON_IsNumber(jsonField)) {
                    short value = jsonField->valueint;
                    intRange valueRange = entries[i].validator.intRange;

                    if (value >= valueRange.min && value <= valueRange.max) {
                        *((short*)entries[i].paramPointer) = value;
                    }
                }
                break;

            case BOOLEAN_TYPE:
                if (cJSON_IsBool(jsonField)) {
                    *((boolean*)entries[i].paramPointer) = jsonField->valueint;
                }
                break;

            case ENUM_STRING:
                if (cJSON_IsString(jsonField)) {
                    const char* modeString = jsonField->valuestring;
                    short mode = mapStringToEnum(modeString, entries[i].validator.enumMappings);

                    if (mode != -1) {
                        *((short*)entries[i].paramPointer) = mode;
                    }
                }
                break;

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

    configField* entries = getFieldEntries(config);

    for (int i = 0; entries[i].name; i++) {
        switch (entries[i].type) {
        case INT_TYPE: {
            short short_value = *((short*)entries[i].paramPointer);
            cJSON_AddNumberToObject(root, entries[i].name, short_value);
            break;
        }
        case BOOLEAN_TYPE: {
            boolean bool_value = *((boolean*)entries[i].paramPointer);
            cJSON_AddBoolToObject(root, entries[i].name, bool_value);
            break;
        }
        case ENUM_STRING: {
            short enum_value = *((short*)entries[i].paramPointer);
            const char* string_value = entries[i].validator.enumMappings[enum_value];
            cJSON_AddStringToObject(root, entries[i].name, string_value);
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

void readFromConfig(enum graphicsModes* initialGraphics) {
    char* jsonString = loadConfigFile();

    configParams config = createDefaultConfig();

    parseConfigValues(jsonString, &config);

    rogue.wizard = config.wizard;
    rogue.easyMode = config.easyMode;
    rogue.displayStealthRangeMode = config.displayStealthRangeMode;
    rogue.trueColorMode = config.trueColorMode;
    rogue.playbackDelayPerTurn = config.playbackDelayPerTurn;

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