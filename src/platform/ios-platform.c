#include <math.h>
#include <limits.h>
#include "Rogue.h"
#include "platform.h"
#include "GlobalsBase.h"

boolean nonInteractivePlayback = false;
struct brogueConsole currentConsole;

boolean tryParseUint64(char *str, uint64_t *num) {
    unsigned long long n;
    char buf[100];
    if (strlen(str)                 // we need some input
        && sscanf(str, "%llu", &n)  // try to convert to number
        && sprintf(buf, "%llu", n)  // convert back to string
        && !strcmp(buf, str)) {     // compare (we need them equal)
        *num = (uint64_t)n;
        return true; // success
    } else {
        return false; // input was too large or not a decimal number
    }
}

