#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "utils.hpp"
#include "udp_receiver.hpp"

int num_places(unsigned int n){
	if (n < 10) return 1;
	if (n < 100) return 2;
	if (n < 1000) return 3;
	if (n < 10000) return 4;
	if (n < 100000) return 5;
	if (n < 1000000) return 6;
	if (n < 10000000) return 7;
	if (n < 100000000) return 8;
	if (n < 1000000000) return 9;
	/*      4294967296 is 2^32 -1, so... */
	return 10;
}

void read_folder(char* folder, size_t buffer_size){
	struct stat sb;

	printf(">");
	while(1){
		fflush(stdout);
		fgets(folder, buffer_size, stdin);
		folder[strlen(folder)-1] = 0;

		if(stat(folder, &sb) == 0 && S_ISDIR(sb.st_mode))
			break;  // Successfully read a folder

		printf("Please insert a valid path for a folder. "
				"Has to be a full path.\n>");
	}
}

void read_nick(char *nick, size_t buffer_size){
	int valid;

	printf(">");
	while(1){
		fflush(stdout);
		fgets(nick, buffer_size, stdin);
		remove_n_newline(nick, buffer_size);

		valid = 1;
		unsigned int i;
		for(i=0; i<buffer_size; i++){
			char c = nick[i];
			if(c == '\0')
				break;
			if(c == ';'){
				valid = 0;
				break;
			}
		}
		if(valid && strlen(nick) <= NICK_MAX_DIGITS) break;
		printf("The nickname you entered is invalid. Try again.\n>");
	}
}

size_t remove_newline(char* string){
	return remove_n_newline(string, strlen(string));
}

size_t remove_n_newline(char* string, size_t string_size){
	if(string_size == 0) return 0;
	size_t i;
	for(i=0; i<string_size; i++){
		if(string[i] == '\n'){
			string[i] = '\0';
			break;
		}
	}
	return i+1;
}

int compare_timespec(struct timespec* time1, struct timespec* time2){
	if(time1 == NULL || time2 == NULL)
		return 2; // Not an expected value but... -1, 0 and 1 are taken... :(

	if(time1->tv_sec == time2->tv_sec){
		if(time1->tv_nsec == time2->tv_nsec){
			return 0;
		}else if(time1->tv_nsec < time2->tv_nsec){
			return 1;
		}else{
			return -1;
		}
	}else if(time1->tv_sec < time2->tv_sec){
		return 1;
	}else{
		return -1;
	}
}

int substract_timespec(struct timespec* basetime, struct timespec* optime){
	if(basetime == NULL || optime == NULL)
		return -1;

	basetime->tv_sec -= optime->tv_sec;
	if(basetime->tv_nsec < optime->tv_nsec){
		basetime->tv_sec--;
		basetime->tv_nsec += 1000000000;
	}
	basetime->tv_nsec -= optime->tv_nsec;

	return 0;
}

int add_timespec(struct timespec* basetime, struct timespec* optime){
	if(basetime == NULL || optime == NULL)
		return -1;

	basetime->tv_sec += optime->tv_sec;
	basetime->tv_nsec += optime->tv_nsec;
	if(basetime->tv_nsec >= 1000000000){
		basetime->tv_nsec -= 1000000000;
		basetime->tv_sec++;
	}

	return 0;
}

int init_timeres(struct timespec* timeres, int updates_per_sec){
	if(timeres == NULL)
		return -1;

	if(updates_per_sec <= 1){
		timeres->tv_sec = 1;
		timeres->tv_nsec = 0;
	}else{
		timeres->tv_sec = 0;
		timeres->tv_nsec = 1000000000 / updates_per_sec;
	}
	fprintf(stderr, "At %d updates per sec, timeres = {%d, %ld}\n",
			updates_per_sec, (int)timeres->tv_sec, timeres->tv_nsec);
	return 0;
}

double timeres_to_double(struct timespec* timeres){
	if(timeres == NULL)
		return 0.0;

	return (double)timeres->tv_sec + ((double)timeres->tv_nsec / 1000000000);
}
