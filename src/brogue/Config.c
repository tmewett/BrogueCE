#include "../../external/cJSON/cJSON.h"
#include "Rogue.h"
#include "GlobalsBase.h"

typedef struct configParams {
    short playbackDelayPerTurn;
    short gameVariant;      // Brogue, Rapid, Bullet
    short gameMode;         // Normal, Easy, Wizard
    short graphicsMode;     // Ascii, Hybrid, Graphics
    boolean stealthRangeEnabled;
    boolean colorEffectsEnabled;
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

static const char* JSON_FILENAME = "brogue_config.json";
static const char* JSON_FILENAME_BACKUP = "brogue_config_backup.json";

// maps json strings to the gameVariant Enum
static const char* variantMappings[] = {"brogue", "rapid", "bullet", NULL};

// maps json strings to the graphicsModes Enum
static const char* graphicsModeMappings[] = {"text", "tiles", "hybrid", NULL};

// maps json strings to the gameMode Enum
static const char* gameModeMappings[] = {"normal", "wizard", "easy", NULL};

static intRange playbackDelayRange = {MIN_PLAYBACK_DELAY, MAX_PLAYBACK_DELAY};

static configParams createDefaultConfig(void) {
    configParams config;

    config.playbackDelayPerTurn = DEFAULT_PLAYBACK_DELAY;
    config.gameVariant = VARIANT_BROGUE;
    config.graphicsMode = TEXT_GRAPHICS;
    config.gameMode = GAME_MODE_NORMAL;
    config.colorEffectsEnabled = false;
    config.stealthRangeEnabled = false;

    return config;
}

static char* loadConfigFile(void) {
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

    size_t len = fread(buffer, 1, fileSize, jsonFile);
    fclose(jsonFile);

    if (len < fileSize) {
        fprintf(stderr, "ERROR: Failed to read entire file.  Attempting to continue.");
    }

    buffer[len] = '\0';

    return buffer;
}

static configField* getFieldEntries(configParams* config) {
    configField fieldDescriptors[] = {
        {"gameVariant", &(config->gameVariant), ENUM_STRING, {.enumMappings = variantMappings}},
        {"graphicsMode", &(config->graphicsMode), ENUM_STRING, {.enumMappings = graphicsModeMappings}},
        {"gameMode", &(config->gameMode), ENUM_STRING, {.enumMappings = gameModeMappings}},
        {"playbackDelayPerTurn", &(config->playbackDelayPerTurn), INT_TYPE, {.intRange = playbackDelayRange}},
        {"stealthRangeEnabled", &(config->stealthRangeEnabled), BOOLEAN_TYPE},
        {"colorEffectsEnabled", &(config->colorEffectsEnabled), BOOLEAN_TYPE},
        {NULL}};

    configField* entries = malloc(sizeof(fieldDescriptors));
    memcpy(entries, fieldDescriptors, sizeof(fieldDescriptors));
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
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "JSON Format Error! Defect detected starting here: \n%s\n", error_ptr);
        } else {
            fprintf(stderr, "Failed to parse JSON (possibly due to an out-of-memory error).\n");
        }

        // Save old config file so the user can fix it
        if (rename(JSON_FILENAME, JSON_FILENAME_BACKUP) == 0) {
            printf("Config file renamed to '%s'.\n", JSON_FILENAME_BACKUP);
        } else {
            // perror prints a system error message explaining why it failed
            perror("Error renaming config file!");
        }

        return;
    }

    configField* entries = getFieldEntries(config);

    for (int i = 0; entries[i].name; i++) {
        intRange valueRange;
        cJSON* jsonField = cJSON_GetObjectItem(root, entries[i].name);
        if (!jsonField) continue;
        
        switch (entries[i].type) {
            case INT_TYPE:
                valueRange = entries[i].validator.intRange;

                // Clamp to range of short
                valueRange.min = max(INT16_MIN, valueRange.min); // We're assuming short means 16 bits...
                valueRange.max = min(INT16_MAX, valueRange.max);

                if (cJSON_IsNumber(jsonField)) {
                    int value = jsonField->valueint;
                    if (value >= valueRange.min && value <= valueRange.max) {
                        *((short*)entries[i].paramPointer) = value;
                    } else {
                        fprintf(stderr, "ERROR: Value '%d' not valid for field '%s'.  Using default '%d' instead.\n", value, entries[i].name, valueRange.min);
                        *((short*)entries[i].paramPointer) = valueRange.min;
                    }
                } else {
                    fprintf(stderr, "ERROR: Invalid type for field '%s'.  Using default '%d' instead.\n", entries[i].name, valueRange.min);
                    *((short*)entries[i].paramPointer) = valueRange.min;
                }
                break;

            case BOOLEAN_TYPE:
                if (cJSON_IsBool(jsonField)) {
                    *((boolean*)entries[i].paramPointer) = cJSON_IsTrue(jsonField) ? true : false;
                } else {
                    fprintf(stderr, "ERROR: Invalid type for field '%s'.  Using default 'false'.\n", entries[i].name);
                    *((boolean*)entries[i].paramPointer) = false;
                }
                break;

            case ENUM_STRING:
                if (cJSON_IsString(jsonField)) {
                    const char* modeString = jsonField->valuestring;
                    short index = mapStringToEnum(modeString, entries[i].validator.enumMappings);

                    if (index != -1) {
                        *((short*)entries[i].paramPointer) = index;
                    } else {
                        fprintf(stderr, "ERROR: String '%s' not valid for field '%s'.  Using default '%s' instead.\n", modeString, entries[i].name, entries[i].validator.enumMappings[0]);
                        *((short*)entries[i].paramPointer) = 0;
                    }
                } else {
                    fprintf(stderr, "ERROR: Invalid type for field '%s'.  Using default '%s' instead.\n", entries[i].name, entries[i].validator.enumMappings[0]);
                    *((short*)entries[i].paramPointer) = 0;
                }
                break;

            default:
                fprintf(stderr, "ERROR: Unknown field type for field '%s', ignoring.\n", entries[i].name);
                break;
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

    rogue.mode = config.gameMode;
    rogue.displayStealthRangeMode = config.stealthRangeEnabled;
    rogue.trueColorMode = config.colorEffectsEnabled;
    rogue.playbackDelayPerTurn = config.playbackDelayPerTurn;

    gameVariant = config.gameVariant;
    *initialGraphics = config.graphicsMode;

    free(jsonString);
}

void writeIntoConfig(void) {
    configParams config;

    FILE* file = fopen(JSON_FILENAME, "w");

    if (!file) {
        return;
    }

    config.gameMode = rogue.mode;
    config.stealthRangeEnabled = rogue.displayStealthRangeMode;
    config.colorEffectsEnabled = rogue.trueColorMode;
    config.playbackDelayPerTurn = rogue.playbackDelayPerTurn;

    config.gameVariant = gameVariant;
    config.graphicsMode = graphicsMode;

    char* jsonString = createJsonString(&config);

    fprintf(file, "%s", jsonString);

    fclose(file);
    free(jsonString);
}
