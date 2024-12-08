#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

#include <json-c/json.h>
#include <X11/Xlib.h>

#include <libg13.h>

#include "keydefs.h"

Display *display;
Window win;

unsigned char x, y;


int handle_x11_error(Display* display, XErrorEvent* error){
    fprintf(stderr, "ERROR: X11 error\n");
    return 1;
}

void reset_screen(bool img) {
    g13_clear_lcd();
    g13_set_color(0xff, 0x66, 0x00);
    if (img) {
        g13_set_img("elite.lpbm");
    }
    g13_render();
}

void key_handler(int k, bool pressed) {
    if (!display) {
        return;
    }

    XEvent ev;
    ev.type = pressed ? KeyPress : KeyRelease;
    ev.xkey.state = None;
    ev.xkey.keycode = KEY_ESC;
    ev.xkey.same_screen = True;

    unsigned int *c = &ev.xkey.keycode;
    if (k == G1 ) *c = KEY_INSERT;
    if (k == G2 ) *c = KEY_DEL;
    if (k == G3 ) *c = KEY_Q;
    if (k == G4 ) *c = KEY_W;
    if (k == G5 ) *c = KEY_E;
    if (k == G7 ) *c = KEY_HOME;
    if (k == G10) *c = KEY_A;
    if (k == G11) *c = KEY_S;
    if (k == G12) *c = KEY_D;
    if (k == G15) *c = KEY_LSHIFT;
    if (k == G20) *c = KEY_LCTRL;
    if (k == G21) *c = KEY_END;
    if (k == G22) *c = KEY_SPACE;

    if (k == M1 ) *c = KEY_1;
    if (k == M2 ) *c = KEY_2;
    if (k == M3 ) *c = KEY_3;
    if (k == MR ) *c = KEY_4;

    if (k == CLICK1) *c = KEY_BACKSPACE;
    if (k == CLICK2) *c = KEY_BACKSPACE;

    static bool silent_running = false;
    if (pressed && k == G21) {
        silent_running = !silent_running;
        if (silent_running) {
            g13_set_color(0, 0, 0);
        } else {
            reset_screen(true);
        }
    }

    if (ev.xkey.keycode != KEY_ESC) {
        XSendEvent(display, win, True, KeyPressMask, &ev);
    }
}

void click1(bool pressed) {
    /*
    if (display) {
        XEvent ev;
        ev.type = pressed ? ButtonPress : ButtonRelease;
        ev.xbutton.button = 1;
        ev.xbutton.same_screen = True;
        XSendEvent(display, win, True, KeyPressMask, &ev);
    }
    */
    if (display) {
        XEvent ev;
        ev.type = pressed ? KeyPress : KeyRelease;
        ev.xkey.state = None;
        ev.xkey.keycode = KEY_BACKSPACE;
        ev.xkey.same_screen = True;
        XSendEvent(display, win, True, KeyPressMask, &ev);
    }
}
void click2(bool pressed) {
    if (display) {
        XEvent ev;
        ev.type = pressed ? KeyPress : KeyRelease;
        ev.xkey.state = None;
        ev.xkey.keycode = KEY_BACKSPACE;
        ev.xkey.same_screen = True;
        XSendEvent(display, win, True, KeyPressMask, &ev);
    }
}

void stick(unsigned char new_x, unsigned char new_y) {
    /*printf("Stick position: %d %d\n", new_x, new_y);*/

    /*
    if (new_x != x || new_y != y) {
        x = new_x;
        y = new_y;
        g13_set_color(x, 255, 255 - x);
        g13_clear_lcd();
        g13_draw_circle(2 + (int)(155.f * (float)x / 255.f),
                    2 + (int)(36.f * (float)y / 255.f), 2.);
        g13_render();
    }
    */

    XEvent ev;
    ev.xkey.state = None;
    ev.xkey.same_screen = True;

    static bool x_l_pressed = false;
    static bool x_r_pressed = false;
    static bool y_u_pressed = false;
    static bool y_d_pressed = false;

    bool x_l_state = new_x < 3;
    bool x_r_state = new_x > 252;
    bool y_u_state = new_y < 3;
    bool y_d_state = new_y > 252;

    if (x_l_state != x_l_pressed) {
        bool pressed = x_l_state;
        if (display) {
            ev.type = pressed ? KeyPress : KeyRelease;
            ev.xkey.keycode = KEY_A;
            XSendEvent(display, win, True, KeyPressMask, &ev);
        }
        x_l_pressed = x_l_state;
    }

    if (x_r_state != x_r_pressed) {
        bool pressed = x_r_state;
        if (display) {
            ev.type = pressed ? KeyPress : KeyRelease;
            ev.xkey.keycode = KEY_D;
            XSendEvent(display, win, True, KeyPressMask, &ev);
        }
        x_r_pressed = x_r_state;
    }

    if (y_u_state != y_u_pressed) {
        bool pressed = y_u_state;
        if (display) {
            ev.type = pressed ? KeyPress : KeyRelease;
            ev.xkey.keycode = KEY_R;
            XSendEvent(display, win, True, KeyPressMask, &ev);
        }
        y_u_pressed = y_u_state;
    }

    if (y_d_state != y_d_pressed) {
        bool pressed = y_d_state;
        if (display) {
            ev.type = pressed ? KeyPress : KeyRelease;
            ev.xkey.keycode = KEY_F;
            XSendEvent(display, win, True, KeyPressMask, &ev);
        }
        y_d_pressed = y_d_state;
    }
}

char s_filepath[4096];

int get_filepath() {
    struct dirent *dp;
    DIR *dfd;

    char *dir ;
    dir = "/home/tom/games/elite_log/" ;

    if ((dfd = opendir(dir)) == NULL) {
        fprintf(stderr, "Can't open %s\n", dir);
        return 0;
    }

    time_t max_t = 0;

    while ((dp = readdir(dfd)) != NULL) {
        struct stat stbuf ;
        char filename_qfd[4096];
        sprintf(filename_qfd , "%s/%s",dir,dp->d_name) ;
        if (stat(filename_qfd, &stbuf) == -1)
        {
            printf("Unable to stat file: %s\n",filename_qfd) ;
            continue ;
        }

        if ((stbuf.st_mode & S_IFMT) == S_IFDIR)
        {
            continue;
            // Skip directories
        }
        else
        {
            if (!strncmp(dp->d_name, "Journal.", 8) && stbuf.st_mtime > max_t)
            {
                max_t = stbuf.st_mtime;
                sprintf(s_filepath, "%s%s", dir, dp->d_name) ;
            }
        }
    }
}

static char remain_str[8];
static bool jumping = false;

void assess(json_object *jobj) {
    json_object *eventobj = json_object_object_get(jobj, "event");
    if (eventobj) {
        const char *event = json_object_get_string(eventobj);

        if (!strcmp(event, "FSDTarget")) {
            json_object *remainobj = json_object_object_get(jobj, "RemainingJumpsInRoute");
            if (remainobj) {
                const char *remain = json_object_get_string(remainobj);
                snprintf(remain_str, sizeof(remain_str), "%s", remain);
                remain_str[sizeof(remain_str) - 1] = '\0';
            } else {
                remain_str[0] = '\0';
            }
        }

        // Hyperspce start
        if (!strcmp(event, "StartJump")) {
            json_object *jumptypeobj = json_object_object_get(jobj, "JumpType");
            if (jumptypeobj) {
                const char *val = json_object_get_string(jumptypeobj);
                if (!strcmp(val, "Hyperspace")) {
                    jumping = true;
                    g13_clear_lcd();
                    g13_draw_sentence(60, 2, "JUMPING");
                    const char* star_class = json_object_get_string(json_object_object_get(jobj, "StarClass"));
                    g13_draw_sentence(4, 10, json_object_get_string(json_object_object_get(jobj, "StarSystem")));
                    g13_draw_sentence(4, 20, star_class);
                    g13_draw_sentence(4, 30, remain_str);

                    bool scoopable = false;
                    bool danger = false;

                    if (   !strcmp(star_class, "O")
                        || !strcmp(star_class, "B")
                        || !strcmp(star_class, "A")
                        || !strcmp(star_class, "F")
                        || !strcmp(star_class, "G")
                        || !strcmp(star_class, "K")
                        || !strcmp(star_class, "M")) {
                        scoopable = true;
                    }

                    // White dwarfs
                    if (   !strcmp(star_class, "D")
                        || !strcmp(star_class, "DA")
                        || !strcmp(star_class, "DB")
                        || !strcmp(star_class, "DC")
                        || !strcmp(star_class, "DO")
                        || !strcmp(star_class, "DQ")
                        || !strcmp(star_class, "DX")) {
                        danger = true;
                    }

                    // TODO: Find Neutron classifications

                    if (danger) {
                        g13_set_color(0xff, 0x00, 0x00);
                    } else if (scoopable) {
                        g13_set_color(0x00, 0xff, 0x00);
                    } else {
                        g13_set_color(0x00, 0x00, 0xff);
                    }

                    g13_render();
                }
            }
        }
        // Hyperspace end
        if (jumping && !strcmp(event, "ReceiveText")) {
            jumping = false;
            reset_screen(true);
        }

	if (!strcmp(event, "DockingGranted")) {
	    const char* landing_pad = json_object_get_string(json_object_object_get(jobj, "LandingPad"));
            int pad_number = atoi(landing_pad);
            char pad_formatted[4];
            snprintf(pad_formatted, sizeof(pad_formatted), "%02d", pad_number);
	    g13_clear_lcd();
	    g13_set_color(0x00, 0xff, 0x00);
            g13_draw_sentence_scaled(36, 2, "GRANTED", 2);
	    g13_draw_sentence_scaled(60, 18, pad_formatted, 3);
	    g13_render();
	}

	if (!strcmp(event, "DockingDenied")) {
	    g13_clear_lcd();
	    g13_set_color(0xff, 0x00, 0x00);
            g13_draw_sentence_scaled(28, 10, "DENIED", 3);
	    g13_render();
	}

	if (!strcmp(event, "Docked") || !strcmp(event, "DockingCancelled") || !strcmp(event, "DockingTimeout")) {
		reset_screen(true);
	}
    } else {
    }
}

struct json_tokener* s_tok;

// Parse a line from the file as a JSON object
void parse(char* str) {
    json_object *jobj = NULL;
    int stringlen = 0;
    enum json_tokener_error jerr;
    stringlen = strlen(str);
    jobj = json_tokener_parse_ex(s_tok, str, stringlen);
    jerr = json_tokener_get_error(s_tok);
    if (jerr == json_tokener_success) {
        assess(jobj);
    } else {
        fprintf(stderr, "Error: %s\n", json_tokener_error_desc(jerr));
        // Handle errors, as appropriate for your application.
    }
}

static int last_position=0;
int find_new_text(FILE* file) {
    fseek(file, 0, SEEK_END);
    int filesize = ftell(file);

    // Check if the new file started
    if (filesize < last_position) {
        last_position=0;
    }

    // Read file from last position until newline is found

    static bool first = true;

    for (int n=last_position; n<filesize; n++) {
        fseek(file, last_position, SEEK_SET);
        char *ptr = NULL;
        size_t n2 = 0;

        getline(&ptr, &n2, file);

        if (ptr) {
            last_position = ftell(file);
            /*printf("Char: %s Last %d\n", ptr, last_position);*/

            if (!first) {
                parse(ptr);
            }

            // EOF
            if (filesize == last_position) {
                return filesize;
            }
            free(ptr);
        }

    }

    first = false;

    return 0;
}

int main(int argc, char** argv) {
    display = XOpenDisplay(NULL);
    XSetErrorHandler(handle_x11_error);

    g13_init();

    g13_bind_all_keys(key_handler);
    g13_bind_stick(stick);

    reset_screen(true);

    /*
    g13_bind_key(CLICK1, click1);
    g13_bind_key(CLICK2, click2);
    */

    get_filepath();
    /*sprintf(s_filepath, "/home/tom/drivers/elite/testfile");*/
    printf("File is %s\n", s_filepath);
    FILE* file = fopen(s_filepath, "r");

    remain_str[0] = '\0';

    s_tok = json_tokener_new();

    while (1) {
        // Update focus window
        int revert_to;
        XGetInputFocus(display, &win, &revert_to);
        find_new_text(file);
        usleep(10000);
    }

    g13_clear_lcd();
    g13_render();

    json_tokener_free(s_tok);

    return 0;
}


