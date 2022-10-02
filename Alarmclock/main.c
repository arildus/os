#include <stdio.h>

//Need this to prevent warning with strptime
//https://stackoverflow.com/questions/15334558/compiler-gets-warnings-when-using-strptime-function-c
#define __USE_XOPEN 

#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

#define MAX_ALARMS 10

//Structure of alarm
typedef struct Alarm{
	time_t alarm_time;
	int duration_from_start;
	int pid;
} Alarm;

// Holding the alarms
int nb_of_alarms = 0;
struct Alarm alarms[MAX_ALARMS];

// We are using paplay since that worked on our systems
int play_sound() {
	char *args[] = {"paplay", "wilhelm.ogg", NULL};
	execvp("paplay", args);

	//If mpg123 would have worked we would have done it like this:
	// char *args[] = {"mpg123", "wilhelm.mp3", NULL};
	// execvp("mpg123", args);
}

// How each individual should behave when called
int alarm_behaviour(Alarm *alarm) {
	sleep((*alarm).duration_from_start);
	play_sound();
	exit(1);
}

// What happens when we want to start the alarm. Starts child procesesses
int start_alarm(Alarm *alarm) {
	int pid = fork();
	if (pid == 0) {
		alarm_behaviour(alarm);
	} else if(pid < 0) {
		printf("\nCould not start alarm");
	} else if(pid > 0) {
		(*alarm).pid = pid;
	}
}

// Add alarms to list of alarms and start the alarm
int add_alarm(Alarm alarm) {
	if(nb_of_alarms < MAX_ALARMS) {
		start_alarm(&alarm);
		alarms[nb_of_alarms] = alarm;
		nb_of_alarms += 1;
	} else {
		printf("\nMaximum number of alarms reached!!!");
		return 0;
	}
}

// What happens when the user wants to schedule an alarm
int schedule() {
	if(nb_of_alarms == MAX_ALARMS) {
		printf("Maximum number of alarms reached! Cant schedule before one of the alarms are canceled or rings");
		return 0;
	}

	printf("Schedule alarm at which date and time? ");
	char input_time[20]; //Allocate 20 chars since YYYY-MM-DD HH:MM:SS\n is 20 chars
	scanf("%20c", input_time);

	struct tm time_struct;
	strptime(input_time, "%Y-%m-%d %T", &time_struct); //Create struct of time
	time_struct.tm_isdst = -1; 	//To avoid issues with timezones
	time_t input_time_t = mktime(&time_struct); //Parse timestruct to time_t

	//Get current time
	time_t now = time(NULL);
	struct tm current_time = *localtime(&now);
	now = mktime(&current_time);


	//Calculate difference from the time the alarm was entered to the current time
	int time_difference = difftime(input_time_t, now);
	printf("Scheduling alarm in %d seconds \n", time_difference);

	Alarm talarm = {
		.alarm_time = input_time_t,
		.duration_from_start = time_difference
	};

	add_alarm(talarm);
}

int list() {
	for (int i=0; i<nb_of_alarms; i++) {
		struct tm time = *localtime(&(alarms[i].alarm_time));
		printf("\nAlarm %d at %d-%02d-%02d %02d:%02d:%02d",
				i+1,
				time.tm_year + 1900,
				time.tm_mon + 1,
				time.tm_mday,
				time.tm_hour,
				time.tm_min,
				time.tm_sec
			);
	}
}

int remove_alarm_from_list(int alarm_number) {
	// Remove the alarm and shift every alarm with an alarm id higher on number down
	for(int i=alarm_number-1; i<nb_of_alarms; i++) {
		alarms[i] = alarms[i+1];
	}

	// Reduce the size of the list of alarams
	nb_of_alarms--;
}

// Assume that we shift every alarm up one number when a alarm is deleted
int cancel() {
	printf("\nCancel which alarm? ");
	int desired_delete;
	scanf("%d", &desired_delete);

	// Signal to kill the ongoing alarm
	kill(alarms[desired_delete-1].pid, SIGKILL);

	remove_alarm_from_list(desired_delete);
}


int main() {
	char input_char;
	
	//Get current time
	time_t t = time(NULL);
	struct tm current_time = *localtime(&t);

	printf("Welcome to the alarm clock! ");
	printf("It is currently %d-%02d-%02d %02d:%02d:%02d", 
		current_time.tm_year + 1900, 
		current_time.tm_mon + 1, 
		current_time.tm_mday, 
		current_time.tm_hour, 
		current_time.tm_min, 
		current_time.tm_sec
	);
	printf("\nPlease enter \"s\" (schedule), \"l\" (list), \"c\" (cancel), \"x\" (exit)");
	while(1) {


		printf("\n> ");

		// To avoid new line character getting stored in the buffer of getchar()
		//https://stackoverflow.com/questions/7898215/how-to-clear-input-buffer-in-c
		while((input_char = getchar()) != '\n' && input_char != EOF) {
				// Clear zombies from alarms which has finished
				int deletedPid = waitpid(-1, 0,WNOHANG);
				
				// Find the index of the alarm and delete it
				for(int i = 0; i < nb_of_alarms; i++) {
					if(alarms[i].pid == deletedPid) {
						remove_alarm_from_list(i+1);
					} 
				}

				switch(input_char) {
					case 's':
						schedule();
						break;
					case 'l':
						list();
						break;
					case 'c':
						cancel();
						break;
					case 'x':
						return 0;
				}
		}
	}

	return 0;
}
