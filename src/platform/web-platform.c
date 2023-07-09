#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/select.h>
#include "platform.h"

#define SERVER_SOCKET "server-socket"
#define CLIENT_SOCKET "client-socket"

#define OUTPUT_SIZE 10

#define EVENT_MESSAGE1_START 19
#define EVENT_MESSAGE2_START 70
#define EVENT_MESSAGE1_SIZE 51
#define EVENT_MESSAGE2_SIZE 30
#define EVENT_SIZE 100
#define MAX_INPUT_SIZE 5
#define MOUSE_INPUT_SIZE 4
#define OUTPUT_BUFFER_SIZE 1000

//Custom events
#define REFRESH_SCREEN 50

enum StatusTypes
{
    DEEPEST_LEVEL_STATUS,
    GOLD_STATUS,
    SEED_STATUS,
    EASY_MODE_STATUS,
    STATUS_TYPES_NUMBER
};

extern playerCharacter rogue;
static struct sockaddr_un addr_write;
static int wfd, rfd;

static FILE *logfile;
static unsigned char outputBuffer[OUTPUT_BUFFER_SIZE];
static int outputBufferPos = 0;
static int refreshScreenOnly = 0;

static void gameLoop();
static void openLogfile();
static void closeLogfile();
static void writeToLog(const char *msg);
static void setupSockets();
static int readFromSocket(unsigned char *buf, int size);
static void writeToSocket(unsigned char *buf, int size);
static void flushOutputBuffer();

static void gameLoop() {
    openLogfile();
    writeToLog("Logfile started\n");

    setupSockets();

    int statusCode = rogueMain();

    closeLogfile();

    exit(statusCode);
}

static void openLogfile() {
    logfile = fopen("brogue-web.txt", "a");
    if (logfile == NULL)
    {
        fprintf(stderr, "Logfile not created, errno = %d\n", errno);
    }
}

static void closeLogfile() {
    fclose(logfile);
}

static void writeToLog(const char *msg) {
    fprintf(logfile, "%s", msg);
    fflush(logfile);
}

static void setupSockets() {
    struct sockaddr_un addr_read;

    // Open read socket (from external)
    rfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    remove(SERVER_SOCKET);

    memset(&addr_read, 0, sizeof(struct sockaddr_un));
    addr_read.sun_family = AF_UNIX;
    strncpy(addr_read.sun_path, SERVER_SOCKET, sizeof(addr_read.sun_path) - 1);

    bind(rfd, (struct sockaddr *)&addr_read, sizeof(struct sockaddr_un));

    // Open write socket (to external)
    wfd = socket(AF_UNIX, SOCK_DGRAM, 0);

    memset(&addr_write, 0, sizeof(struct sockaddr_un));
    addr_write.sun_family = AF_UNIX;
    strncpy(addr_write.sun_path, CLIENT_SOCKET, sizeof(addr_write.sun_path) - 1);
}

int readFromSocket(unsigned char *buf, int size) {
    return recvfrom(rfd, buf, size, 0, NULL, NULL);
}

static void flushOutputBuffer() {
    char msg[80];
    int no_bytes_sent;

    no_bytes_sent = sendto(wfd, outputBuffer, outputBufferPos, 0, (struct sockaddr *)&addr_write, sizeof(struct sockaddr_un));
    if (no_bytes_sent == -1) {
        snprintf(msg, 80, "Error: %s\n", strerror(errno));
        writeToLog(msg);
    } else if (no_bytes_sent != outputBufferPos) {
        snprintf(msg, 80, "Sent %d bytes only - %s\n", no_bytes_sent, strerror(errno));
        writeToLog(msg);
    }

    outputBufferPos = 0;
}

static void writeToSocket(unsigned char *buf, int size)
{
    if (outputBufferPos + size > OUTPUT_BUFFER_SIZE) {
        flushOutputBuffer();
    }

    memcpy(outputBuffer + outputBufferPos, buf, size);
    outputBufferPos += size;
}

// Map characters which are missing or rendered as emoji on some platforms
static unsigned int fixUnicode(unsigned int code) {
    switch (code) {
        case U_ARIES: return 0x03C8;
        case U_CIRCLE: return 'o';
        case U_CIRCLE_BARS: return 0x25C6;
        case U_FILLED_CIRCLE_BARS: return 0x25C7;
        default: return code;
    }
}

static void web_plotChar(enum displayGlyph inputChar,
                         short xLoc, short yLoc,
                         short foreRed, short foreGreen, short foreBlue,
                         short backRed, short backGreen, short backBlue) {
    unsigned char outputBuffer[OUTPUT_SIZE];
    unsigned char firstCharByte, secondCharByte;
    enum displayGlyph translatedChar;

    translatedChar = glyphToUnicode(inputChar);
    translatedChar = fixUnicode(inputChar);

    firstCharByte = translatedChar >> 8 & 0xff;
    secondCharByte = translatedChar;

    outputBuffer[0] = (unsigned char)xLoc;
    outputBuffer[1] = (unsigned char)yLoc;
    outputBuffer[2] = firstCharByte;
    outputBuffer[3] = secondCharByte;
    outputBuffer[4] = (unsigned char)foreRed * 255 / 100;
    outputBuffer[5] = (unsigned char)foreGreen * 255 / 100;
    outputBuffer[6] = (unsigned char)foreBlue * 255 / 100;
    outputBuffer[7] = (unsigned char)backRed * 255 / 100;
    outputBuffer[8] = (unsigned char)backGreen * 255 / 100;
    outputBuffer[9] = (unsigned char)backBlue * 255 / 100;

    writeToSocket(outputBuffer, OUTPUT_SIZE);
}

static void sendStatusUpdate() {
    unsigned char statusOutputBuffer[OUTPUT_SIZE];
    unsigned long statusValues[STATUS_TYPES_NUMBER];
    int i, j;

    statusValues[DEEPEST_LEVEL_STATUS] = rogue.deepestLevel;
    statusValues[GOLD_STATUS] = rogue.gold;
    statusValues[SEED_STATUS] = rogue.seed;
    statusValues[EASY_MODE_STATUS] = rogue.easyMode;

    memset(statusOutputBuffer, 0, OUTPUT_SIZE);

    for (i = 0; i < STATUS_TYPES_NUMBER; i++) {
        // Coordinates of (255, 255) will let the server and client know that this is a status update rather than a cell update
        statusOutputBuffer[0] = 255;
        statusOutputBuffer[1] = 255;

        // Status type
        statusOutputBuffer[2] = i;

        // Status values
        statusOutputBuffer[3] = statusValues[i] >> 24 & 0xff;
        statusOutputBuffer[4] = statusValues[i] >> 16 & 0xff;
        statusOutputBuffer[5] = statusValues[i] >> 8 & 0xff;
        statusOutputBuffer[6] = statusValues[i];

        // Fill
        for (j = 7; j < OUTPUT_SIZE; j++) {
            statusOutputBuffer[j] = 0;
        }

        writeToSocket(statusOutputBuffer, OUTPUT_SIZE);
    }
}

// Pause by doing a blocking poll on the socket
static boolean web_pauseForMilliseconds(short milliseconds) {
    fd_set input;
    struct timeval timeout;

    FD_ZERO(&input);
    FD_SET(rfd, &input);

    timeout.tv_sec = milliseconds / 1000;
    timeout.tv_usec = (milliseconds % 1000) * 1000;

    return select(rfd + 1, &input, NULL, NULL, &timeout);
}

static void web_nextKeyOrMouseEvent(rogueEvent *returnEvent, boolean textInput, boolean colorsDance) {

    unsigned char inputBuffer[MAX_INPUT_SIZE];
    unsigned short keyCharacter;

    // Because we will halt execution until we get more input, we definitely cannot have any dancing colors from the server side.
    colorsDance = false;

    // Send a status update of game variables we want on the client
    if (!refreshScreenOnly) {
        sendStatusUpdate();
    }
    refreshScreenOnly = 0;

    // Flush output buffer
    flushOutputBuffer();

    // Block for next command
    readFromSocket(inputBuffer, MAX_INPUT_SIZE);

    returnEvent->eventType = inputBuffer[0];

    if (returnEvent->eventType == REFRESH_SCREEN) {
        // Custom event type - not a command for the brogue game
        refreshScreen();
        // Don't send a status update if this was only a screen refresh (may be sent by observer)
        refreshScreenOnly = 1;
        return;
    }

    if (returnEvent->eventType == KEYSTROKE) {
        keyCharacter = inputBuffer[1] << 8 | inputBuffer[2];

        // 13 is sent for RETURN on web, map to RETURN_KEY
        if (keyCharacter == 13) {
            keyCharacter = RETURN_KEY;
        }

        returnEvent->param1 = keyCharacter;
        returnEvent->controlKey = inputBuffer[3];
        returnEvent->shiftKey = inputBuffer[4];
    } else { // Mouse event
        fread(inputBuffer, sizeof(char), MOUSE_INPUT_SIZE, stdin);
        returnEvent->param1 = inputBuffer[1]; //x coord
        returnEvent->param2 = inputBuffer[2]; //y coord
        returnEvent->controlKey = inputBuffer[3];
        returnEvent->shiftKey = inputBuffer[4];
    }
}

static void web_remap(const char *input_name, const char *output_name) {
    // Not needed
}

static boolean web_modifierHeld(int modifier) {
    // Not needed, modifiers past directly with the event data
    return 0;
}

static void web_notifyEvent(short eventId, int data1, int data2, const char *str1, const char *str2) {
    unsigned char statusOutputBuffer[EVENT_SIZE];

    // Coordinates of (254, 254) will let the server and client know that this is a event notification update rather than a cell update
    statusOutputBuffer[0] = 254;
    statusOutputBuffer[1] = 254;

    statusOutputBuffer[2] = eventId;

    statusOutputBuffer[3] = data1 >> 24 & 0xff;
    statusOutputBuffer[4] = data1 >> 16 & 0xff;
    statusOutputBuffer[5] = data1 >> 8 & 0xff;
    statusOutputBuffer[6] = data1;
    statusOutputBuffer[7] = rogue.depthLevel >> 8 & 0xff;
    statusOutputBuffer[8] = rogue.depthLevel;
    statusOutputBuffer[9] = rogue.easyMode >> 8 & 0xff;
    statusOutputBuffer[10] = rogue.easyMode;
    statusOutputBuffer[11] = rogue.gold >> 24 & 0xff;
    statusOutputBuffer[12] = rogue.gold >> 16 & 0xff;
    statusOutputBuffer[13] = rogue.gold >> 8 & 0xff;
    statusOutputBuffer[14] = rogue.gold;
    statusOutputBuffer[15] = rogue.seed >> 24 & 0xff;
    statusOutputBuffer[16] = rogue.seed >> 16 & 0xff;
    statusOutputBuffer[17] = rogue.seed >> 8 & 0xff;
    statusOutputBuffer[18] = rogue.seed;

    // str1 is the death / victory message
    memcpy(statusOutputBuffer + EVENT_MESSAGE1_START, str1, EVENT_MESSAGE1_SIZE);
    statusOutputBuffer[EVENT_MESSAGE2_START - 1] = 0;
    // str2 is unused
    memcpy(statusOutputBuffer + EVENT_MESSAGE1_START + EVENT_MESSAGE1_SIZE, str2, EVENT_MESSAGE2_SIZE);
    statusOutputBuffer[EVENT_SIZE - 1] = 0;

    writeToSocket(statusOutputBuffer, EVENT_SIZE);
    flushOutputBuffer();
}

struct brogueConsole webConsole = {
    gameLoop,
    web_pauseForMilliseconds,
    web_nextKeyOrMouseEvent,
    web_plotChar,
    web_remap,
    web_modifierHeld,
    web_notifyEvent,
    NULL,
    NULL
};
