
#ifndef _term_h_
#define _term_h_

#define TERM_NONE 0
#define TERM_MOUSE 1

typedef struct {float r, g, b;} fcolor;
struct term_t {
    int (*start)();
    void (*end)();
    void (*put)(int x, int y, int ch, fcolor *fg, fcolor *bg);
    void (*refresh)();
    int (*getkey)();
    void (*wait)(int ms);
    int (*hasKey)();
    void (*title)(const char *);
    void (*resize)(int w, int h);
    int (*keycodeByName)(const char *);

    struct {
        int up, down, left, right, backspace, del, quit;
    } keys;

    int width, height;
    struct {
        int x, y;
        int isPressed, justPressed, justReleased, justMoved;
        int shift, control;
    } mouse;
};

extern struct term_t Term;

#endif

