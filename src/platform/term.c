#include <ncurses.h>
#include "term.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "Rogue.h"
#include <ctype.h>


// As a rule, everything in term.c is the result of gradual evolutionary
// change.  It's messy.

#define COLORING(fg,bg) (((fg) & 0x0f) | (((bg) & 0x07) << 4))
#define COLOR_FG(color,fg) (((fg) & 0x0f) + ((color) & 0x70))
#define COLOR_BG(color,bg) (((color) & 0x0f) + (((bg) & 0x07) << 4))
#define COLOR_INDEX(color) (1 + ((color)&0x07) + (((color) >> 1) & 0x38))
#define COLOR_ATTR(color) (COLOR_PAIR(COLOR_INDEX(color)) | (((color)&0x08) ? A_BOLD : 0))


static struct { int curses, color; } videomode = { 0, 0 };

static struct { int width, height; } minsize = { 80, 24 };

static void init_coersion(void);


// 256 color mode stuff
static void initialize_prs(void);

typedef struct {
    int r, g, b, idx;
} intcolor;

struct {
    intcolor fore, back;
    int count, next;
} prs[256];


typedef struct {
    int ch, pair, shuffle;
    intcolor fore, back;
} pairmode_cell;

pairmode_cell *cell_buffer;

enum {
    coerce_16,
    coerce_256,
    truecolor
} colormode;

int is_xterm;


//

static void preparecolor (void) {
    // sixteen color mode colors (we use these in 256-color mode, too)
    static int pairParts[8] = {
        COLOR_BLACK, COLOR_RED, COLOR_GREEN, COLOR_YELLOW,
        COLOR_BLUE, COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE
    };

    int fg, bg;
    for (bg=0; bg<8; bg++) {
        for (fg=0; fg<8; fg++) {
            init_pair(
                COLOR_INDEX(COLORING(fg, bg)),
                pairParts[fg], pairParts[bg]
            );
        }
    }

    char *env = getenv("COLORTERM");
    if (env && ((strncmp(env, "truecolor", 9) == 0) || (strncmp(env, "24bit", 5) == 0))) {
        colormode = truecolor;
    } else if (COLORS >= 256) {
        colormode = coerce_256;
    } else {
        colormode = coerce_16;
    }
}

static void term_title(const char *title) {
    if (is_xterm) {
        printf ("\033]2;%s\007", title); // ESC ]0; title BEL
    }
}

static void term_title_pop(void) {
    if (is_xterm) {
        term_title("Terminal");
        printf ("\033[22;2t");
    }
}
static void term_title_push(void) {
    if (is_xterm) {
        printf ("\033[23;2t");
    }
}

static void term_set_size(int h, int w) {
    // works in gnome-terminal, but not xterm; causes trouble for maximized windows
    if (is_xterm) {
        // first, try resizing the height, in case only that is supported
        printf ("\033[%dt", (h > 24 ? h : 24));

        // then try resizing both, in case we can
        printf ("\033[8;%d;%dt", h, w);

        // then refresh so ncurses knows about it
        refresh( );
    }
}

static void term_show_scrollbar(int show) {
    // works in xterm, but not gnome-terminal
    if (is_xterm) {
        if (show) {
            printf ("\033[?30h");
        } else {
            printf ("\033[?30l");
        }
    }
}

static int curses_init(void) {
    if (videomode.curses) return 0;

    // isterm?
    initscr( );
    if (!has_colors( )) {
        endwin( );
        fprintf (stderr, "Your terminal has no color support.\n");
        return 1;
    }

    start_color( );
    clear( );
    curs_set( 0 );
    refresh( );
    leaveok(stdscr, TRUE);
    preparecolor( );
    raw( );
    noecho( );
    nonl( );

    nodelay(stdscr, TRUE);
    meta(stdscr, TRUE);
    keypad(stdscr, TRUE);

    mousemask(BUTTON1_PRESSED | BUTTON1_RELEASED | REPORT_MOUSE_POSITION | BUTTON_SHIFT | BUTTON_CTRL, NULL);
    mouseinterval(0); //do no click processing, thank you

    videomode.curses = 1;

    getmaxyx(stdscr, Term.height, Term.width);

    return 1;
}


static int term_start(void) {
    char *term = getenv("TERM");
    is_xterm = (strncmp(term, "xterm", 5) == 0) || (strncmp(term, "gnome", 5) == 0) || (strncmp(term, "st", 2) == 0);

    term_title_push();
    term_show_scrollbar(0);

    int ok = curses_init();
    init_coersion();

    return ok;
}

static void term_end(void) {
    term_title_pop();
    clear();
    refresh();
    endwin();
}

typedef struct CIE {
    float X, Y, Z;
    float x, y, z;
} CIE;

typedef struct Lab {
    float L, a, b;
} Lab;

#define DARK 0.0
#define DIM 0.1
#define MID 0.3
#define HALFBRIGHT 0.5
#define BRIGHT 0.9

fcolor palette[16] = {
    {DARK, DARK, DARK},
    {MID, DARK, DARK},
    {DARK, MID, DARK},
    {MID, .8 * MID, DIM},
    {DARK, DARK, MID},
    {MID + DIM, DARK, MID},
    {DARK, MID, MID},
    {HALFBRIGHT, HALFBRIGHT, HALFBRIGHT},

    {MID, MID, MID},
    {BRIGHT, DARK, DARK},
    {DARK, BRIGHT, DARK},
    {BRIGHT, BRIGHT, DARK},
    {HALFBRIGHT, MID, BRIGHT},
    {BRIGHT, HALFBRIGHT, BRIGHT},
    {DARK, BRIGHT, BRIGHT},
    {BRIGHT, BRIGHT, BRIGHT}
};

CIE ciePalette[16];
Lab labPalette[16];
CIE adamsPalette[16];

static CIE white;

static CIE toCIE(fcolor c) {
    double a = 0.055;

    // http://en.wikipedia.org/wiki/SRGB_color_space#The_reverse_transformation

    c.r = c.r <= 0.04045 ? c.r / 12.92 : pow((c.r + a) / (1 + a), 2.4);
    c.g = c.g <= 0.04045 ? c.g / 12.92 : pow((c.g + a) / (1 + a), 2.4);
    c.b = c.b <= 0.04045 ? c.b / 12.92 : pow((c.b + a) / (1 + a), 2.4);

    CIE cie;
    cie.X = 0.4124 * c.r + 0.3576 * c.g + 0.1805 * c.b;
    cie.Y = 0.2126 * c.r + 0.7152 * c.g + 0.0722 * c.b;
    cie.Z = 0.0193 * c.r + 0.1192 * c.g + 0.9505 * c.b;

    float sum = cie.X + cie.Y + cie.Z;
    if (sum == 0.0) sum = 1.0;
    cie.x = cie.X / sum;
    cie.y = cie.Y / sum;
    cie.z = 1.0 - cie.x - cie.y;

    return cie;
}

static float Labf(float t) {
    return t > ((6.0/29.0) * (6.0/29.0) * (6.0/29.0)) ? pow(t, 1.0/3.0) : ((1.0/3.0) * (29.0 / 6.0) * (29.0 / 6.0)) * t + (4.0 / 29.0);
}

static Lab toLab(CIE *c) {
    CIE n = (CIE) {Labf(c->X / white.X), Labf(c->Y / white.Y), Labf(c->Z / white.Z)};
    Lab l;

    // http://en.wikipedia.org/wiki/L*a*b*#RGB_and_CMYK_conversions
    l.L = 116.0 * n.Y - 16;
    l.a = 500.0 * (n.X - n.Y);
    l.b = 200.0 * (n.Y - n.Z);

    return l;
}

static float munsellSloanGodlove(float t) {
    return sqrt(1.4742 * t - 0.004743 * t * t);
}

static CIE adams(CIE *v) {
    CIE c;
    c.Y = munsellSloanGodlove(v->Y);
    c.X = munsellSloanGodlove((white.Y / white.X) * v->X) - c.Y;
    c.Z = munsellSloanGodlove((white.Z / white.X) * v->Z) - c.Y;

    return c;
}

#define SQUARE(x) ((x) * (x))

static float CIE76(Lab *L1, Lab *L2) {
    // http://en.wikipedia.org/wiki/Color_difference#CIE76
    float lbias = 1.0;
    return sqrt(lbias * SQUARE(L2->L - L1->L) + SQUARE(L2->a - L1->a) + SQUARE(L2->b - L1->b));
}

static void init_coersion(void) {
    fcolor sRGB_white = (fcolor) {1, 1, 1};
    white = toCIE(sRGB_white);

    int i;
    for (i = 0; i < 16; i++) {
        ciePalette[i] = toCIE(palette[i]);
        labPalette[i] = toLab(&ciePalette[i]);
        adamsPalette[i] = adams(&ciePalette[i]);
    }

    if (colormode == coerce_256) {
        initialize_prs();
    }

    cell_buffer = 0;
}

static int best (fcolor *fg, fcolor *bg) {
    // analyze fg & bg for their contrast
    CIE cieFg = toCIE(*fg);
    CIE cieBg = toCIE(*bg);
    Lab labFg = toLab(&cieFg);
    Lab labBg = toLab(&cieBg);
    // CIE adamsFg = adams(&cieFg);
    // CIE adamsBg = adams(&cieBg);

    float JND = 2.3; // just-noticeable-difference
    int areTheSame = CIE76(&labFg, &labBg) <= 2.0 * JND; // a little extra fudge

    float big = 100000000;
    int fg1 = 0, fg2 = 0, bg1 = 0, bg2 = 0;
    float fg1_score = big, fg2_score = big;
    float bg1_score = big, bg2_score = big;

    int i;

    for (i = 0; i < 8; i++) {
        float s = CIE76(labPalette + i, &labBg);

        if (s < bg2_score) {
            if (s < bg1_score) {
                bg2 = bg1; bg1 = i;
                bg2_score = bg1_score; bg1_score = s;
            } else {
                bg2 = i; bg2_score = s;
            }
        }
    }

    if (areTheSame) {
        return COLORING(bg1, bg1);
    }

    for (i = 0; i < 16; i++) {
        float s = CIE76(labPalette + i, &labFg);

        if (s < fg2_score) {
            if (s < fg1_score) {
                fg2 = fg1; fg1 = i;
                fg2_score = fg1_score; fg1_score = s;
            } else {
                fg2 = i; fg2_score = s;
            }
        }
    }

    if (fg1 != bg1) {
        return COLORING (fg1, bg1);
    } else {
        if (fg1_score + bg2_score < fg2_score + bg1_score) {
            return COLORING(fg1, bg2);
        } else {
            return COLORING(fg2, bg1);
        }
    }
}




static void initialize_prs(void) {
    int i;
    for (i = 16; i < 255; i++) {
        prs[i].next = i + 1;
    }
    prs[0].next = 16;
    prs[1].next = 0;
    prs[255].next = 0;
}

static void coerce_colorcube (fcolor *f, intcolor *c) {
    // 0-15 are the standard ANSI colors
    // 16-231 are a 6x6x6 RGB color cube given by ((36 * r) + (6 * g) + b + 16) with r,g,b in [0..5]
    // 232-255 are a greyscale ramp without black and white.

    float sat = 0.2, bright = 0.6, contrast = 6.3;

    float rf = bright + f->r * contrast,
        gf = bright + f->g * contrast,
        bf = bright + f->b * contrast;

    if (rf < gf && rf < bf) rf -= sat * ((gf < bf ? bf : gf) - rf);
    else if (gf < bf && gf < rf) gf -= sat * ((rf < bf ? bf : rf) - gf);
    else if (bf < gf && bf < rf) bf -= sat * ((gf < rf ? rf : gf) - bf);

    int r = rf, g = gf, b = bf;
    r = r < 0 ? 0 : r > 5 ? 5 : r;
    g = g < 0 ? 0 : g > 5 ? 5 : g;
    b = b < 0 ? 0 : b > 5 ? 5 : b;

    c->r = r;
    c->g = g;
    c->b = b;
    c->idx = ((36 * r) + (6 * g) + b + 16);
}

static int intcolor_distance (intcolor *a, intcolor *b) {
    return
        (a->r - b->r) * (a->r - b->r)
        + (a->g - b->g) * (a->g - b->g)
        + (a->b - b->b) * (a->b - b->b);
}

static int coerce_prs (intcolor *fg, intcolor *bg) {
    // search for an exact match in the list
    int pair;
    pair = prs[1].next;
    while (pair) {
        if (prs[pair].fore.idx == fg->idx && prs[pair].back.idx == bg->idx) {
            // perfect.
            prs[pair].count++;
            return pair;
        }
        pair = prs[pair].next;
    }

    // no exact match? try to insert it as a new one
    pair = prs[0].next;
    if (pair) {
        // there's room!

        // remove
        prs[0].next = prs[pair].next;

        // insert at the front
        prs[pair].next = prs[1].next;
        prs[1].next = pair;

        // initialize it
        prs[pair].fore = *fg;
        prs[pair].back = *bg;
        prs[pair].count = 1;

        init_pair(pair, fg->idx, bg->idx);

        return pair;
    }

    // search for an approximate match in the list
    int bestpair = 0, bestscore = 2 * 3 * 6 * 6; // naive distance metric for now
    pair = prs[1].next;
    while (pair) {
        int delta = intcolor_distance(&prs[pair].fore, fg) + intcolor_distance(&prs[pair].back, bg);
        if (delta < bestscore) {
            bestscore = delta;
            bestpair = pair;
            if (delta == 1) break; // as good as it gets without being exact!
        }
        pair = prs[pair].next;
    }

    prs[bestpair].count++;
    return bestpair;
}

static void buffer_plot(int ch, int x, int y, fcolor *fg, fcolor *bg) {
    // int pair = 256 + x + y * minsize.width;
    // intcolor cube_fg, cube_bg;
    // coerce_colorcube(fg, &cube_fg),
    // coerce_colorcube(bg, &cube_bg);

    // pair = cube_bg.idx;
    // cube_fg = cube_bg;


    // init_pair(pair, cube_fg.idx, cube_bg.idx);

    // return pair;

    intcolor cube_fg, cube_bg;

    if (colormode == coerce_256) {
        coerce_colorcube(fg, &cube_fg);
        coerce_colorcube(bg, &cube_bg);
        if (cube_fg.idx == cube_bg.idx) {
            // verify that the colors are really the same; otherwise, we'd better force the output apart
            int naive_distance =
                (fg->r - bg->r) * (fg->r - bg->r)
                + (fg->g - bg->g) * (fg->g - bg->g)
                + (fg->b - bg->b) * (fg->b - bg->b);
            if (naive_distance > 3) {
                // very arbitrary cutoff, and an arbitrary fix, very lazy
                if (cube_bg.r > 0) {cube_bg.r -= 1; cube_bg.idx -= 1; }
                if (cube_bg.g > 0) {cube_bg.g -= 1; cube_bg.idx -= 6; }
                if (cube_bg.b > 0) {cube_bg.b -= 1; cube_bg.idx -= 36; }
            }
        }
    } else {
        cube_fg = (intcolor){
            .r = round(fg->r * 255),
            .g = round(fg->g * 255),
            .b = round(fg->b * 255)
        };
        cube_bg = (intcolor){
            .r = round(bg->r * 255),
            .g = round(bg->g * 255),
            .b = round(bg->b * 255)
        };
    }

    int cell = x + y * minsize.width;
    cell_buffer[cell].ch = ch;
    cell_buffer[cell].pair = -1;
    cell_buffer[cell].fore = cube_fg;
    cell_buffer[cell].back = cube_bg;
}

static void buffer_render_256(void) {
    // build a new palette
    initialize_prs();

    int length = minsize.width * minsize.height;
    int i, idx, x, y;

    for (i = 0; i < length; i++) {
        cell_buffer[i].shuffle = i;
    }
    for (i = length - 1; i >= 0; i--) {
        // int roll = i == 0 ? 0 : rand() % i;
        // idx = cell_buffer[roll].shuffle;

        // cell_buffer[roll].shuffle = cell_buffer[i].shuffle;

        idx = i;

        int pair = coerce_prs(&cell_buffer[idx].fore, &cell_buffer[idx].back);
        cell_buffer[idx].pair = pair;
    }

    // render it all!
    i = 0;
    for (y = 0; y < minsize.height; y++) {
        move(y, 0);
        for (x = 0; x < minsize.width; x++) {
            color_set(cell_buffer[i].pair, NULL);
            addch(cell_buffer[i].ch);
            i++;
        }
    }
    refresh();
}

static int fullRefresh = 1; // screen needs a full refresh

static void buffer_render_24bit(void) {
    int cx, cy;      // cursor coordinates
    intcolor fg, bg; // current colors

    cx = cy = fg.r = fg.g = fg.b = bg.r = bg.g = bg.b = -1;

    for (int y = 0; y < minsize.height; y++) {
        for (int x = 0; x < minsize.width; x++) {
            pairmode_cell *c = &cell_buffer[x + y * minsize.width];

            // `pair` is set to -1 when a tile changes, which signals we need to print it
            if (!c->pair && !fullRefresh) continue;
            c->pair = 0;

            // change background color
            if (c->back.r != bg.r || c->back.g != bg.g || c->back.b != bg.b) {
                bg = c->back;
                printf("\033[48;2;%d;%d;%dm", bg.r, bg.g, bg.b);
            }

            // change foreground color (doesn't matter for whitespace)
            if (c->ch != ' ' && (fg.r != c->fore.r || fg.g != c->fore.g || fg.b != c->fore.b)) {
                fg = c->fore;
                printf("\033[38;2;%d;%d;%dm", fg.r, fg.g, fg.b);
            }

            // move cursor if necessary
            if (cx != x || cy != y) {
                cx = x, cy = y;
                printf("\033[%d;%df", cy+1, cx+1);
            }

            // print the character
            printf("%c", c->ch);
            cx++;
        }
    }

    fflush(stdout);
    fullRefresh = 0;
}

static void term_mvaddch(int x, int y, int ch, fcolor *fg, fcolor *bg) {
    if (x < 0 || y < 0 || x >= minsize.width || y >= minsize.height) return;

    if (colormode == coerce_16) {
        int c = best(fg, bg);
        attrset(COLOR_ATTR(c));
        mvaddch(y, x, ch);
    } else {
        buffer_plot(ch, x, y, fg, bg);
    }
}

static void term_refresh(void) {
    // to set up a 256-color terminal, see:
    // http://push.cx/2008/256-color-xterms-in-ubuntu
    if (0 && can_change_color()) {
        int i;
        for (i = 0; i < 16; i++) {
            short r = palette[i].r * 1000;
            short g = palette[i].g * 1000;
            short b = palette[i].b * 1000;
            if (r < 0) r = 0;
            if (g < 0) g = 0;
            if (b < 0) b = 0;
            init_color(i + 1, r, g, b);
        }
    }
    if (0) {
        int i;
        short r, g, b;
        for (i = 0; i < 8; i++) {
            color_content(i, &r, &g, &b);
            palette[i].r = r * .001;
            palette[i].g = g * .001;
            palette[i].b = b * .001;
        }
    }

    if (colormode == truecolor) {
        buffer_render_24bit();
    } else if (colormode == coerce_256) {
        buffer_render_256();
    } else if (colormode == coerce_16) {
        refresh();
    }
}

static void ensure_size(void);

static int term_getkey(void) {
    Term.mouse.justPressed = 0;
    Term.mouse.justReleased = 0;
    Term.mouse.justMoved = 0;

    while (1) {
        int got = getch();
        if (got == KEY_RESIZE) {
            ensure_size( );
            fullRefresh = 1;
        } else if (got == KEY_MOUSE) {
            MEVENT mevent;
            getmouse (&mevent);
            Term.mouse.x = mevent.x;
            Term.mouse.y = mevent.y;
            Term.mouse.shift = (mevent.bstate & BUTTON_SHIFT) != 0;
            Term.mouse.control = (mevent.bstate & BUTTON_CTRL) != 0;
            if (mevent.bstate & BUTTON1_PRESSED) {
                Term.mouse.justPressed = 1;
                Term.mouse.isPressed = 1;
            } else if (mevent.bstate & BUTTON1_RELEASED) {
                if (Term.mouse.isPressed) {
                    Term.mouse.justReleased = 1;
                    Term.mouse.isPressed = 0;
                }
            } else {
                Term.mouse.justMoved = 1;
            }
            return TERM_MOUSE;
        } else {
            if (got == KEY_ENTER) got = 13; // KEY_ENTER -> ^M for systems with odd values for KEY_ENTER
            if (got == ERR) return TERM_NONE;
            else return got;
        }
    }
}

static int term_has_key(void) {
    int ch = getch();
    if (ch != ERR) {
        ungetch(ch);
        return 1;
    } else {
        return 0;
    }
}

static void ensure_size(void) {
    int w = minsize.width, h = minsize.height;

    getmaxyx(stdscr, Term.height, Term.width);
    if (Term.height < h || Term.width < w) {
        getmaxyx(stdscr, Term.height, Term.width);
        nodelay(stdscr, FALSE);
        while (Term.height < h || Term.width < w) {
            erase();
            attrset(COLOR_ATTR(7));

            mvprintw(1,0,"Brogue needs a terminal window that is at least [%d x %d]", w, h);

            attrset(COLOR_ATTR(15));
            mvprintw(2,0,"If your terminal can be resized, resize it now.\n");

            attrset(COLOR_ATTR(7));
            mvprintw(3,0,"Press ctrl-c at any time to quit.\n");

            printw("Width:  %d/%d\n", Term.width, w);
            printw("Height: %d/%d\n", Term.height, h);

            mvprintw(10, 0, "Colors (pairs): %d (%d)\n", COLORS, COLOR_PAIRS);

            getch();
            getmaxyx(stdscr, Term.height, Term.width);
        }
        nodelay(stdscr, TRUE);
        erase();
        refresh();
    }
}

static void term_resize(int w, int h) {
    minsize.width = w;
    minsize.height = h;

    // try to set the terminal size if the terminal will let us:
    term_set_size(h, w);
    // (this works in gnome-terminal, but causes trouble for curses on maximized windows.)

    // now make sure it worked, and ask the user to resize the terminal if it didn't
    ensure_size();


    // make a new cell buffer

    if (cell_buffer) free(cell_buffer);
    cell_buffer = malloc(sizeof(pairmode_cell) * w * h);
    // add error checking
    int i;

    for (i = 0; i < w * h; i++) {
        // I guess we could just zero it all, hmm
        cell_buffer[i].ch = 0;
        cell_buffer[i].pair = 0;
        cell_buffer[i].fore.idx = 0;
        cell_buffer[i].back.idx = 0;
    }
}

static void term_wait(int ms) {
    napms(ms);
}


struct {
    char *name;
    int ch;
} curses_keys[] = {
    {"NONE", TERM_NONE},

    {"TAB", '\t'},
    {"ENTER", '\n'},
    {"RETURN", '\n'},
    {"SPACE", ' '},

    {"ESC", 27},
    {"ESCAPE", 27},

    {"BREAK", KEY_BREAK},
    {"SRESET", KEY_SRESET},
    {"RESET", KEY_RESET},
    {"DOWN", KEY_DOWN},
    {"UP", KEY_UP   },
    {"LEFT", KEY_LEFT},
    {"RIGHT", KEY_RIGHT},
    {"HOME", KEY_HOME},
    {"BACKSPACE", KEY_BACKSPACE},
    {"F1", KEY_F(1)},
    {"F2", KEY_F(2)},
    {"F3", KEY_F(3)},
    {"F4", KEY_F(4)},
    {"F5", KEY_F(5)},
    {"F6", KEY_F(6)},
    {"F7", KEY_F(7)},
    {"F8", KEY_F(8)},
    {"F9", KEY_F(9)},
    {"F10", KEY_F(10)},
    {"F11", KEY_F(11)},
    {"F12", KEY_F(12)},
    {"DL", KEY_DL},
    {"IL", KEY_IL},
    {"DC", KEY_DC},
    {"DEL", KEY_DC},
    {"DELETE", KEY_DC},
    {"IC", KEY_IC},
    {"EIC", KEY_EIC},
    {"CLEAR", KEY_CLEAR},
    {"EOS", KEY_EOS},
    {"EOL", KEY_EOL},
    {"SF", KEY_SF},
    {"SR", KEY_SR},

    {"PGUP", KEY_NPAGE},
    {"PGDN", KEY_PPAGE},
    {"PAGEDOWN", KEY_NPAGE},
    {"PAGEUP", KEY_PPAGE},
    {"NPAGE", KEY_NPAGE},
    {"PPAGE", KEY_PPAGE},

    {"STAB", KEY_STAB},
    {"CTAB", KEY_CTAB},
    {"CATAB", KEY_CATAB},

    {"PRINT", KEY_PRINT},
    {"LL", KEY_LL},
    {"A1", KEY_A1},
    {"A3", KEY_A3},
    {"B2", KEY_B2},
    {"C1", KEY_C1},
    {"C3", KEY_C3},
    {"BTAB", KEY_BTAB},
    {"BEG", KEY_BEG },
    {"CANCEL", KEY_CANCEL},
    {"CLOSE", KEY_CLOSE},
    {"COMMAND", KEY_COMMAND},
    {"COPY", KEY_COPY},
    {"CREATE", KEY_CREATE},
    {"END", KEY_END },
    {"EXIT", KEY_EXIT},
    {"FIND", KEY_FIND},
    {"HELP", KEY_HELP},
    {"MARK", KEY_MARK},
    {"MESSAGE", KEY_MESSAGE},
    {"MOVE", KEY_MOVE},
    {"NEXT", KEY_NEXT},
    {"OPEN", KEY_OPEN},
    {"OPTIONS", KEY_OPTIONS},
    {"PREVIOUS", KEY_PREVIOUS},
    {"REDO", KEY_REDO},
    {"REFERENCE", KEY_REFERENCE},
    {"REFRESH", KEY_REFRESH},
    {"REPLACE", KEY_REPLACE},
    {"RESTART", KEY_RESTART},
    {"RESUME", KEY_RESUME},
    {"SAVE", KEY_SAVE},
    {"SBEG", KEY_SBEG},
    {"SCANCEL", KEY_SCANCEL},
    {"SCOMMAND", KEY_SCOMMAND},
    {"SCOPY", KEY_SCOPY},
    {"SCREATE", KEY_SCREATE},
    {"SDC", KEY_SDC },
    {"SDL", KEY_SDL },
    {"SELECT", KEY_SELECT},
    {"SEND", KEY_SEND},
    {"SEOL", KEY_SEOL},
    {"SEXIT", KEY_SEXIT},
    {"SFIND", KEY_SFIND},
    {"SHELP", KEY_SHELP},
    {"SHOME", KEY_SHOME},
    {"SIC", KEY_SIC },
    {"SLEFT", KEY_SLEFT},
    {"SMESSAGE", KEY_SMESSAGE},
    {"SMOVE", KEY_SMOVE},
    {"SNEXT", KEY_SNEXT},
    {"SOPTIONS", KEY_SOPTIONS},
    {"SPREVIOUS", KEY_SPREVIOUS},
    {"SPRINT", KEY_SPRINT},
    {"SREDO", KEY_SREDO},
    {"SREPLACE", KEY_SREPLACE},
    {"SRIGHT", KEY_SRIGHT},
    {"SRSUME", KEY_SRSUME},
    {"SSAVE", KEY_SSAVE},
    {"SSUSPEND", KEY_SSUSPEND},
    {"SUNDO", KEY_SUNDO},
    {"SUSPEND", KEY_SUSPEND},
    {"UNDO", KEY_UNDO},
    {"MOUSE", KEY_MOUSE},
    {"RESIZE", KEY_RESIZE},
    {NULL, 0},
};

static int term_keycodeByName(const char *name) {
    int i = 0;
    while (curses_keys[i].name != NULL) {
        if (strcmp(name, curses_keys[i].name) == 0) {
            return curses_keys[i].ch;
        }
        i++;
    }

    return name[0];
}

static int term_ctrlPressed(int* key) {
    // The keycode representing the enter key depends on curses initialization settings. With the
    // current settings, it's represented as 13, so return `RETURN_KEY` instead.
    if (*key == 13) {
        *key = RETURN_KEY;
        return 0;
    }
    if (*key == '\t') { // Tab is represented as "^I"
        return 0;
    }
    const char* str = keyname(*key);
    if (str == NULL) {
        return 0;
    }
    if (strlen(str) == 2 && str[0] == '^' && isalpha(str[1])) {
        // Curses doesn't distinguish between `ctrl-A` and `ctrl-shift-A`, so this special case is
        // needed for autopilot to work.
        if (str[1] == 'A') {
            *key = 'A';
        } else {
            *key = tolower(str[1]);
        }
        return 1;
    } else {
        return 0;
    }
}


struct term_t Term = {
    term_start,
    term_end,
    term_mvaddch,
    term_refresh,
    term_getkey,
    term_wait,
    term_has_key,
    term_title,
    term_resize,
    term_keycodeByName,
    term_ctrlPressed,
    {KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_BACKSPACE, KEY_DC, KEY_F(12)}
};
