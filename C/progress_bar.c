#include <string.h>
#include <stdio.h>
#include <time.h>

#include "progress_bar.h"

#define TERMINAL_SIZE           80
#define PROGRESS_BAR_SIZE       60    // [####-------...-----]
#define PROGRESS_IND_SIZE       8     // XXX.XX%_
#define UPDATES_PER_SEC         5

static int whitespace = 0;
static int lastIndex = 0;
static char progress_buffer[PROGRESS_BAR_SIZE+PROGRESS_IND_SIZE+2];
static clock_t last_update = 0;
static const clock_t timeres = CLOCKS_PER_SEC / UPDATES_PER_SEC;

void create_progress_bar(){
    char *p;
    int i;

    strcpy(progress_buffer, " 00.00% [");
    p = progress_buffer+PROGRESS_IND_SIZE+1;
    for(i=0; i<PROGRESS_BAR_SIZE-2; i++){
        *p = '-';
        p++;
    }
    *p = ']';
    p++;
    *p = '\0';

    whitespace = (TERMINAL_SIZE-PROGRESS_BAR_SIZE-PROGRESS_IND_SIZE) / 2;
    lastIndex = 0;
    last_update = clock();
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
    last_update = currtime;

    float percentage = (float)progress / target;
    int nProgress = percentage * (PROGRESS_BAR_SIZE - 2);
    percentage *= 100;

    if(nProgress > lastIndex){
        int i;
        for(i=lastIndex; i<nProgress; i++){
            progress_buffer[PROGRESS_IND_SIZE+i+1] = '#';
        }
        lastIndex = nProgress;
    }

    snprintf(progress_buffer, sizeof(progress_buffer), "%6.2f%%", percentage);
    progress_buffer[PROGRESS_IND_SIZE-1] = ' ';

    printf("\e[u%s\n", progress_buffer);
}
