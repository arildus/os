#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include "linked_list.h"
#define MAX_ARGLEN 50

int checkIfBackgroundProcess(char** ptr, char* args) {
	int numberOfArguments = -1;
	while (ptr[++numberOfArguments] != NULL) {}

	int background_process = 0;

	if (args[strlen(args)-1] == '&') {
		background_process = 1;
	}
	return background_process;

}

char** getArguments(char* str) {
	int init_size = strlen(str);
	char delimeters[] = " \t";

	char *token = strtok(str, delimeters);

	char *strings[init_size];
    char **intermediate_ptr = strings;

	int j = 0;
	while(token != NULL) {
		if(*token == '<' || *token == '>' || *token == '|') {
			break;
		}
		intermediate_ptr[j] = token;
		j++;
		token = strtok(NULL, delimeters);
	}
	intermediate_ptr[j] = NULL;

	char **ptr = malloc(j*sizeof(char*));

	for (int i = 0; i < j + 1; i++) {
       ptr[i] = malloc(MAX_ARGLEN*sizeof(char));
    }

	for (int i = 0; i < j + 1; i++ ) {
		ptr[i] = intermediate_ptr[i];
    }

	return ptr;
}

void trimTrailing(char* str) {
	// trim trailing whitespace/tab from: https://codeforwin.org/2016/04/c-program-to-trim-trailing-white-space-characters-in-string.html
	int index, i;

	/* Set default index */
	index = -1;

	/* Find last index of non-white space character */
	i = 0;
	while(str[i] != '\0') {
			if(str[i] != ' ' && str[i] != '\t') {
					index= i;
			}
			i++;
	}

	/* Mark next character to last non-white space character as NULL */
	str[index] = '\0';
}


//remove white space from: https://www.faceprep.in/c/program-to-remove-spaces-from-a-string/
char *remove_white_spaces(char *str)
{
	int i = 0, j = 0;
	while (str[i])
	{
		if (str[i] != ' ')
          str[j++] = str[i];
		i++;
	}
	str[j] = '\0';
	return str;
}

char* find_after_delimiter(char str[], char delimiter){
    char* file_name = strchr(str, delimiter);
    if(file_name != NULL) {
        file_name++;
        file_name = remove_white_spaces(file_name);
        return file_name;
    }
    return NULL;
}

// For input redirection
char* find_input(char* str){
	char* input = find_after_delimiter(str, '<');
	if(input == NULL) {
		return NULL;
	}

	// Remove & which should be the last element of the string
	char* sub_char_pointer = strchr(input, '&');
    if (sub_char_pointer != NULL){
       *sub_char_pointer = '\0';
    }

	// Remove output redirection from string
    char* input_stripped = strchr(input, '>');
    if (input_stripped != NULL){
        *input_stripped = '\0';
    }
    return input;
}

//For output redirection
char* find_output(char* str){
	char *output = find_after_delimiter(str, '>');
	if(output == NULL) {
		return NULL;
	}

	// Remove & which should be the last element of the string
	char* sub_char_pointer = strchr(output, '&');
    if (sub_char_pointer != NULL){
       *sub_char_pointer = '\0';
    }

	// Remove input redirection from string
    char* output_stripped = strchr(output, '<');
    if (output_stripped != NULL){
        *output_stripped = '\0';
    }
    return output;
}

int instantiate_process(char* str) {

	char* args = malloc(strlen(str));
	strcpy(args, str);

	char** ptr = getArguments(str);

	//If the user calls cd
	if(strcmp(ptr[0], "cd") == 0) {
		chdir(ptr[1]);
		free(ptr);
		free(args);
		return 0;
	}

	//If the user calls jobs we want to print every job in process and then return
	if(strcmp(ptr[0], "jobs") == 0) {
		printList();
		free(ptr);
		free(args);
		return 0;
	}

	int background_process = checkIfBackgroundProcess(ptr, args);
	int exit_status = 0;

	int pid = fork();

	// If in child process
	if (pid == 0){
		// We copy args since args is a string and will be changed inside find_output and find_input methods
		char* mstrOut = (char*) malloc(strlen(args));
		strcpy(mstrOut, args);
		char* out = find_output(mstrOut);

		char* mstrIn = (char*) malloc(strlen(args));
		strcpy(mstrIn, args);
		char* in = find_input(mstrIn);


		//If we have an input redirection
		if(in != NULL) {
			int fd0 = open(in, O_RDONLY);
			dup2(fd0, STDIN_FILENO);
			close(fd0);
		}

		// If we have an output
		if(out != NULL) {
			int fd1 = creat(out , 0644) ;
			dup2(fd1, STDOUT_FILENO);
			close(fd1);
		}

		// Free allocated memory for copied strings
		free(mstrOut);
		free(mstrIn);

		//Execute child process with given arguments
		execvp(ptr[0], ptr);
		exit(1);
		return -1;
	} else if(pid < 0) {
		printf("Could not start child process\n");
	} else if(pid > 0) {
		// In parent process
		if (background_process) {
			insertFirst(pid, args);
		} else {
			waitpid(pid, &exit_status, 0);
			printf("Exit status [%s] = %d\n", args, exit_status);
			free(args);
		}
	}

	return 0;
}

int main() {

	char cwd[PATH_MAX];
	char str[CHAR_MAX]; 

	while(1) {

		int exit_status;
		int pid = waitpid(-1, &exit_status, WNOHANG);
		if (pid > 0) {
			Process* background_process = find(pid);
			char* args = background_process->cmdline_args;
			printf("Exit status [%s] = %d\n", args, exit_status);
			delete(pid);
			free(args);
		}

		if (getcwd(cwd, sizeof(cwd)) != NULL) {
				printf("%s: ", cwd);
				char* input = fgets(str, CHAR_MAX, stdin);
				if(input == NULL) {
					return 0;
				} else if(strlen(input) == 1) { //If we have a new line character
					continue;
				}
		} else {
				perror("getcwd() error");
		}

		trimTrailing(str);
		instantiate_process(str);
	}

	return 0;
}
