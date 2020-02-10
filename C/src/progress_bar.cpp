#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "progress_bar.hpp"
#include "utils.hpp"

#define TERMINAL_SIZE           80
#define PROGRESS_BAR_SIZE       50    // [####-------...-----]
#define TIME_ELAPSED_SIZE       8     // XXX.XXs_   "_" is a space :P
#define SPEED_IND_SIZE          10    // XXXX KB/s_  "_" is same as above :P
#define UPDATES_PER_SEC         10

// Eta calculation
#define NUM_ENTRIES 10

//  What the bar should look like
//  XXXX KB/s_XXX.XX%_[####-----...---------------------]

static int whitespace = 0;
static int lastIndex = 0;
static char progress_buffer[PROGRESS_BAR_SIZE + TIME_ELAPSED_SIZE +
SPEED_IND_SIZE + 2];
static struct timespec last_update, first_update, timeres;
static unsigned int last_progress = 0;
static const char speed_ch[] = {' ', 'K', 'M', 'G'};
static const char time_ch[] = {'s', 'm', 'h', 'd', 'w'};
static const int time_mul[] = {60, 60, 24, 7};
static double eta[NUM_ENTRIES];
static int eta_nmemb = 0;
static int eta_idx = 0;

void create_progress_bar(){
	char *p;
	int i;

	strcpy(progress_buffer, "   0  B/s      0s [");
	p = progress_buffer+TIME_ELAPSED_SIZE+SPEED_IND_SIZE+1;
	for(i=0; i<PROGRESS_BAR_SIZE-2; i++){
		*p = '-';
		p++;
	}
	*p = ']';
	p++;
	*p = '\0';

	whitespace = (TERMINAL_SIZE - PROGRESS_BAR_SIZE - TIME_ELAPSED_SIZE -
			SPEED_IND_SIZE);
	lastIndex = 0;
	if(clock_gettime(CLOCK_REALTIME, &last_update) == -1){
		return;
	}
	memcpy(&first_update, &last_update, sizeof(struct timespec));
	init_timeres(&timeres, UPDATES_PER_SEC);
	last_progress = 0;
	printf("\x1B[s");
	update_progress_bar(0, 1);
}

void update_progress_bar(unsigned long progress, unsigned long target){
	if(target == 0) return;

	struct timespec currtime;
	if(clock_gettime(CLOCK_REALTIME, &currtime) == -1){
		return;
	}
	substract_timespec(&currtime, &last_update);

	if(compare_timespec(&currtime, &timeres) > 0 && progress != target) return;
	double alpha_time = timeres_to_double(&currtime);

	// Calculate speed
	int speed_ch_idx = 0;
	double speed = (double)(progress - last_progress) / alpha_time;
	double pspeed = speed;
	while(pspeed > 1024){
		pspeed /= 1024;
		speed_ch_idx++;
	}

	// Calculate percentage
	float percentage = (float)progress / target;
	int nProgress = percentage * (PROGRESS_BAR_SIZE - 2);

	// Fill progress bar
	if(nProgress > lastIndex){
		int i;
		for(i=lastIndex; i<nProgress; i++){
			progress_buffer[TIME_ELAPSED_SIZE+SPEED_IND_SIZE+i+1] = '#';
		}
		lastIndex = nProgress;
	}

	// Fill other stuff
	eta[eta_idx++] = (target - progress) / speed;
	if(eta_idx == NUM_ENTRIES) eta_idx = 0;
	if(eta_nmemb != NUM_ENTRIES) eta_nmemb++;

	double m_eta = 0;
	int i;
	for(i=0; i<eta_nmemb; i++){
		m_eta += eta[i];
	}
	m_eta /= eta_nmemb;

	int m_eta_idx = 0;
	while(m_eta_idx < 4 && m_eta > time_mul[m_eta_idx]){
		m_eta /= time_mul[m_eta_idx];
		m_eta_idx++;
	}

	if(m_eta_idx == 4 && m_eta > 99.9){
		m_eta = 99.9;
	}

	snprintf(progress_buffer, sizeof(progress_buffer), "%4d %cB/s %6.2f%c ",
			(int) pspeed, speed_ch[speed_ch_idx], m_eta, time_ch[m_eta_idx]);
	progress_buffer[TIME_ELAPSED_SIZE+SPEED_IND_SIZE] = '[';

	// Update information for next iteration
	add_timespec(&last_update, &currtime);
	last_progress = progress;

	for(i=0; i<whitespace; i++){
		printf(" ");
	}
	printf("%s\x1B[u", progress_buffer);
	fflush(stdout);
	if(progress == target){
		printf("\n");
	}
}
