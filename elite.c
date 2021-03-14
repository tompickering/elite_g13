#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

#include <json-c/json.h>
#include <X11/Xlib.h>

#include <libg13.h>

Display *display;
Window win;

unsigned char x, y;

#define KEY_INSERT 118
#define KEY_L 46

int handle_x11_error(Display* display, XErrorEvent* error){
    fprintf(stderr, "ERROR: X11 error\n");
    return 1;
}

void g1(bool pressed) {
    if (display) {
        XEvent ev;
        ev.type = pressed ? KeyPress : KeyRelease;
        //ev.xkey.state = ShiftMask;
        ev.xkey.state = None;
        ev.xkey.keycode = KEY_INSERT;
        ev.xkey.same_screen = True;
        XSendEvent(display, win, True, KeyPressMask, &ev);
    }
}

void stick(unsigned char new_x, unsigned char new_y) {
    printf("Stick position: %d %d\n", new_x, new_y);

    if (new_x != x || new_y != y) {
        x = new_x;
        y = new_y;
        g13_set_color(x, 255, 255 - x);
        g13_clear_lcd();
        g13_draw_circle(2 + (int)(155.f * (float)x / 255.f),
                    2 + (int)(36.f * (float)y / 255.f), 2.);
        g13_render();
    }
}

void clear() {
    g13_clear_lcd();
    g13_render();
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
        if( stat(filename_qfd, &stbuf ) == -1 )
        {
            printf("Unable to stat file: %s\n",filename_qfd) ;
            continue ;
        }

        if ( ( stbuf.st_mode & S_IFMT ) == S_IFDIR )
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

void assess(json_object *jobj) {
    json_object *eventobj = json_object_object_get(jobj, "event");
    if (eventobj) {
        const char *event = json_object_get_string(eventobj);
        // Hyperspce start
        if (!strcmp(event, "StartJump")) {
            json_object *jumptypeobj = json_object_object_get(jobj, "JumpType");
            if (jumptypeobj) {
                const char *val = json_object_get_string(jumptypeobj);
                if (!strcmp(val, "Hyperspace")) {
                    g13_draw_sentence(60, 2, "JUMPING");
                    g13_draw_sentence(4, 10, json_object_get_string(json_object_object_get(jobj, "StarSystem")));
                    g13_draw_sentence(4, 20, json_object_get_string(json_object_object_get(jobj, "StarClass")));
                    g13_set_color(0xff, 0x00, 0x00);
                    g13_render();
                }
            }
        }
        // Hyperspace end
        if (!strcmp(event, "Music")) {
            json_object *jumptypeobj = json_object_object_get(jobj, "MusicTrack");
            if (jumptypeobj) {
                const char *val = json_object_get_string(jumptypeobj);
                if (!strcmp(val, "DestinationFromHyperspace")) {
                    g13_clear_lcd();
                    g13_set_color(0xff, 0x66, 0x00);
                    g13_render();
                }
            }
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

   // check if the new file started
   if(filesize < last_position){
      last_position=0;
   }  
   // read file from last position  untill new line is found 

   for(int n=last_position;n<filesize;n++) {
      fseek(file, last_position, SEEK_SET);
      char *ptr = NULL;
      size_t n2 = 0;
      getline(&ptr, &n2, file);
      if (ptr) {
          last_position = ftell(file);
          /*printf("Char: %s Last %d\n", ptr, last_position);*/
          parse(ptr);
          // end of file 
          if(filesize == last_position){
            return filesize;
          }
          free(ptr);
      }

  }

  return 0;
}

int main(int argc, char** argv) {
    display = XOpenDisplay(NULL);
    XSetErrorHandler(handle_x11_error);

    g13_init();
    g13_bind_key(G1, g1);
    /*g13_bind_key(ROUND, clear);*/
    /*g13_bind_stick(stick);*/
    g13_clear_lcd();

    g13_set_color(0xff, 0x66, 0x00);

    get_filepath();
    /*sprintf(s_filepath, "/home/tom/drivers/elite/testfile");*/
    printf("File is %s\n", s_filepath);
    FILE* file = fopen(s_filepath, "r");

    s_tok = json_tokener_new();

    while (1) {
        // Update focus window
        int revert_to;
        XGetInputFocus(display, &win, &revert_to);
        find_new_text(file);
        usleep(10000);
    }

    json_tokener_free(s_tok);

    return 0;
}


