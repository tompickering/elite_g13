#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

#include <json-c/json.h>

#include <libg13.h>

unsigned char x, y;

void red() {
    g13_set_color(255, 0, 0);
}

void green() {
    g13_set_color(0, 255, 0);
}

void blue() {
    g13_set_color(0, 0, 255);
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
    g13_init();
    /*g13_bind_key(G1, red);*/
    /*g13_bind_key(G2, green);*/
    /*g13_bind_key(G3, blue);*/
    /*g13_bind_key(ROUND, clear);*/
    /*g13_bind_stick(stick);*/
    g13_clear_lcd();

    get_filepath();
    /*sprintf(s_filepath, "/home/tom/drivers/elite/testfile");*/
    printf("File is %s\n", s_filepath);
    FILE* file = fopen(s_filepath, "r");

    while (1) {
        find_new_text(file);
        usleep(10000);
    }

    return 0;
}


