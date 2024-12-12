#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <string.h>

#include <json-c/json.h>
#include <X11/Xlib.h>

#include <libg13.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "keydefs.h"

Display *display;
Window win;

lua_State *L;

unsigned char x, y;

void reset_lcd(bool);

// *** LUA BINDINGS ***

int LUAWRAPPER_reset_lcd(lua_State *L) {
    reset_lcd(false);
    return 0;
}

int LUAWRAPPER_g13_clear_lcd(lua_State *L) {
    g13_clear_lcd();
    return 0;
}

int LUAWRAPPER_g13_set_color(lua_State *L) {
    int r = luaL_checkinteger(L, 1);
    int g = luaL_checkinteger(L, 2);
    int b = luaL_checkinteger(L, 3);
    g13_set_color(r, g, b);
    return 0;
}

int LUAWRAPPER_g13_draw_string(lua_State *L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    const char* str = luaL_checkstring(L, 3);
    g13_draw_string(x, y, str);
    return 0;
}

int LUAWRAPPER_g13_draw_string_scaled(lua_State *L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    const char* str = luaL_checkstring(L, 3);
    int scale = luaL_checkinteger(L, 4);
    g13_draw_string_scaled(x, y, str, scale);
    return 0;
}

#define MAX_ICON_SIZE 32

int LUAWRAPPER_g13_define_icon(lua_State *L) {
    uint32_t icon[MAX_ICON_SIZE];

    if (lua_istable(L, 1) != 0)
    {
        size_t size = lua_rawlen(L, 1);

        if (size > MAX_ICON_SIZE) {
            fprintf(stderr, "Error: Icon exceeds max size\n");
            size = MAX_ICON_SIZE;
        }

        for (size_t i = 0; i < size; ++i)
        {
            lua_rawgeti(L, 1, i + 1);
            icon[i] = lua_tonumber(L, -1);
            lua_pop(L, 1);
        }

        lua_pushinteger(L, g13_define_icon(icon, size));
        return 1;
    }

    return -1;
}

int LUAWRAPPER_g13_draw_icon(lua_State *L) {
    int id = luaL_checkinteger(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    g13_draw_icon(id, x, y);
}

///////////////////////

int handle_x11_error(Display* display, XErrorEvent* error){
    fprintf(stderr, "ERROR: X11 error\n");
    return 1;
}

void reset_lcd(bool img) {
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
            reset_lcd(true);
        }
    }

    if (ev.xkey.keycode != KEY_ESC) {
        XSendEvent(display, win, True, KeyPressMask, &ev);
    }
}

void click1(bool pressed) {
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

void lua_push_json_value(json_object*);
void lua_push_json_array(json_object*);
void lua_push_json_object(json_object*);

void lua_push_json_object(json_object *jobj) {
    lua_newtable(L);
    json_object_object_foreach(jobj, key, val) {
        lua_pushstring(L, key);
        lua_push_json_value(val);
        lua_settable(L, -3);
    }
}

void lua_push_json_array(json_object *jarr) {
    lua_newtable(L);
    int len = json_object_array_length(jarr);

    for (int i = 0; i < len; ++i) {
        json_object *val = json_object_array_get_idx(jarr, i);
        lua_pushinteger(L, i);
        lua_push_json_value(val);
        lua_settable(L, -3);
    }
}

void lua_push_json_value(json_object *jval) {
    switch (json_object_get_type(jval)) {
        case json_type_object:
            lua_push_json_object(jval);
            break;
        case json_type_null:
            lua_pushnil(L);
            break;
        case json_type_boolean:
            lua_pushboolean(L, json_object_get_boolean(jval));
            break;
        case json_type_double:
            lua_pushnumber(L, json_object_get_double(jval));
            break;
        case json_type_int:
            lua_pushinteger(L, json_object_get_int(jval));
            break;
        case json_type_array:
            lua_push_json_array(jval);
            break;
        case json_type_string:
        default:
            lua_pushstring(L, json_object_get_string(jval));
            break;
    }
}

void assess(json_object *jobj) {
    json_object *eventobj = json_object_object_get(jobj, "event");
    if (eventobj) {
        const char *event = json_object_get_string(eventobj);

        if (event != NULL && strnlen(event, 1) > 0) {
            lua_getglobal(L, event);

            if (lua_isfunction(L, -1)) {
                lua_checkstack(L, 1000);
                lua_push_json_value(jobj);
                lua_pcall(L, 1, 0, 0);
                lua_settop(L, 0);
                g13_render();
            }
        }
    }
}

struct json_tokener* s_tok;

// Parse a line from the file as a JSON object
bool parse(char* str) {
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
        fprintf(stderr, "%s\n", str);
        // The tokener can end up in a confused state - refresh so we can continue working
        json_tokener_free(s_tok);
        s_tok = json_tokener_new();
        // Handle errors, as appropriate for your application.
        return false;
    }

    return true;
}

char input_buf[1024 * 1024];
size_t input_buf_head = 0;

int update() {
    while (1) {
        char c = fgetc(stdin);

        if (c == EOF) {
            break;
        }

        input_buf[input_buf_head++] = c;

        if (input_buf_head >= sizeof(input_buf)) {
            fprintf(stderr, "Line too long\n");
            exit(1);
        }

        if (c == '\n') {
            input_buf[input_buf_head] = '\0';
            if (input_buf_head > 5) {
                parse(input_buf);
            }
            input_buf_head = 0;
        }
    }
}

int init_lua() {
    int status;

    L= NULL;

    L = luaL_newstate();

    if (L == NULL) {
        return 1;
    }

    luaL_openlibs(L);

    status = luaL_loadfile(L, "elite.lua");

    if (status != LUA_OK) {
        return 1;
    }

    lua_pcall(L, 0, 0, 0);

    lua_pushcfunction(L, LUAWRAPPER_reset_lcd);
    lua_setglobal(L, "reset_lcd");
    lua_pushcfunction(L, LUAWRAPPER_g13_clear_lcd);
    lua_setglobal(L, "clear_lcd");
    lua_pushcfunction(L, LUAWRAPPER_g13_set_color);
    lua_setglobal(L, "set_color");
    lua_pushcfunction(L, LUAWRAPPER_g13_draw_string);
    lua_setglobal(L, "draw_string");
    lua_pushcfunction(L, LUAWRAPPER_g13_draw_string_scaled);
    lua_setglobal(L, "draw_string_scaled");
    lua_pushcfunction(L, LUAWRAPPER_g13_define_icon);
    lua_setglobal(L, "define_icon");
    lua_pushcfunction(L, LUAWRAPPER_g13_draw_icon);
    lua_setglobal(L, "draw_icon");

    lua_getglobal(L, "init");
    if (lua_isfunction(L, -1)) {
        lua_pcall(L, 0, 0, 0);
        g13_render();
    }

    return 0;
}

int main(int argc, char** argv) {
    if (init_lua() != 0) {
        printf("COULD NOT INITIALISE LUA\n");
        return 1;
    }

    display = XOpenDisplay(NULL);
    XSetErrorHandler(handle_x11_error);

    if (g13_init() != 0) {
        printf("COULD NOT INITIALISE G13\n");
    }

    g13_bind_all_keys(key_handler);
    g13_bind_stick(stick);

    reset_lcd(true);

    /*
    g13_bind_key(CLICK1, click1);
    g13_bind_key(CLICK2, click2);
    */

    s_tok = json_tokener_new();

    // Configure stdin not to block when there's nothing to read
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags == -1) {
        fprintf(stderr, "Unable to read stdin flags\n");
        exit(1);
    }
    if (fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK) == -1) {
        fprintf(stderr, "Unable to set stdin flags\n");
        exit(1);
    }

    while (1) {
        // Update focus window
        int revert_to;
        XGetInputFocus(display, &win, &revert_to);
        update();
        usleep(10000);
    }

    g13_clear_lcd();
    g13_render();

    json_tokener_free(s_tok);
    lua_close(L);

    return 0;
}


