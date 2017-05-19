#include <string.h>
#include <stdio.h>
#include <time.h>

#include "progress_bar.h"

#define TERMINAL_SIZE           80
#define PROGRESS_BAR_SIZE       50    // [####-------...-----]
#define PROGRESS_IND_SIZE       8     // XXX.XX%_   "_" is a space :P
#define SPEED_IND_SIZE          9     // XXXXKB/s_  "_" is same as above :P
#define UPDATES_PER_SEC         60

//  What the bar should look like
//  XXXXKB/s_XXX.XX%_[####-----...---------------------]

static int whitespace = 0;
static int lastIndex = 0;
static char progress_buffer[PROGRESS_BAR_SIZE + PROGRESS_IND_SIZE +
                            SPEED_IND_SIZE + 2];
static clock_t last_update = 0;
static unsigned int last_progress = 0;
static const clock_t timeres = CLOCKS_PER_SEC / UPDATES_PER_SEC;
static const char speed_ch[] = {' ', 'K', 'M', 'G'};

void create_progress_bar(){
    char *p;
    int i;

    strcpy(progress_buffer, "   0 B/s  00.00%% [");
    p = progress_buffer+PROGRESS_IND_SIZE+SPEED_IND_SIZE+1;
    for(i=0; i<PROGRESS_BAR_SIZE-2; i++){
        *p = '-';
        p++;
    }
    *p = ']';
    p++;
    *p = '\0';

    whitespace = (TERMINAL_SIZE - PROGRESS_BAR_SIZE - PROGRESS_IND_SIZE -
                  SPEED_IND_SIZE) / 2;
    lastIndex = 0;
    last_update = clock();
    last_progress = 0;
    printf("\n\e[A");
    for(i=0; i<whitespace; i++){
        printf(" ");
    }
    printf("\e[s\n");
    update_progress_bar(0, 1);
}

void update_progress_bar(unsigned long progress, unsigned long target){
    if(target == 0) return;

    clock_t currtime = clock();
    if(currtime - last_update < timeres && progress != target) return;

    // Calculate speed
    int speed_ch_idx = 0;
    double speed = (double)(progress - last_progress) /
                    ((double) (currtime - last_update) / CLOCKS_PER_SEC);
    last_progress = progress;
    while(speed > 1024){
      speed /= 1024;
      speed_ch_idx++;
    }

    // Calculate percentage
    float percentage = (float)progress / target;
    int nProgress = percentage * (PROGRESS_BAR_SIZE - 2);
    percentage *= 100;

    // Fill progress bar
    if(nProgress > lastIndex){
        int i;
        for(i=lastIndex; i<nProgress; i++){
            progress_buffer[PROGRESS_IND_SIZE+SPEED_IND_SIZE+i+1] = '#';
        }
        lastIndex = nProgress;
    }

    // Fill other stuff
    snprintf(progress_buffer, sizeof(progress_buffer), "%4d%cB/s %6.2f%% ",
            (int) speed, speed_ch[speed_ch_idx], percentage);
    progress_buffer[PROGRESS_IND_SIZE+SPEED_IND_SIZE] = '[';

    last_update = currtime;

    printf("\e[u%s\n", progress_buffer);
}
