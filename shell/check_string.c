#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

char* find_after_delimiter(char* str, char delimiter){
    char* file_name = strchr(str, delimiter);
    if(file_name != NULL) {
        file_name++;
        file_name = remove_white_spaces(file_name);
        return file_name;
    } else {
        printf("No such symbol!\n");
    }
    return NULL;
}

//Fiks memory leak senere!!!!!!!! malloc og free
char* find_input(char* str){
    char* mstr = malloc(sizeof(str));
    strcpy(mstr, str);
    char* input = find_after_delimiter(mstr, '<');
    printf("%s\n", input);
    char* input_stripped = strchr(input, '>');
    if (input_stripped != NULL){
        *input_stripped = '\0';
    }
    return input;
}

//Fiks memory leak senere!!!!!!!! malloc og free
char* find_output(char* str){
    char* mstr = malloc(sizeof(str));
    strcpy(mstr, str);
    char* input = find_after_delimiter(mstr, '>');
    printf("%s\n", input);
    char* input_stripped = strchr(input, '<');
    if (input_stripped != NULL){
        *input_stripped = '\0';
    }
    return input;
}

char** split_on_pipes(char* str){
    char* pipestr = malloc(strlen(str));
    strcpy(pipestr, str);

    int init_size = strlen(pipestr);
    char delimeters[] = "|";

    char* token = strtok(pipestr, delimeters);

    char* strings[init_size];
    char **ptr = malloc(init_size);
    int j = 0;
    while(token != NULL){
        ptr[j] = token;
        j++;
        token = strtok(NULL, delimeters);
    }
    ptr[j] = NULL;

    for(int i = 0; i < j; i++){
        ptr[i] = remove_white_spaces(ptr[i]);
    }

    return ptr;
}

int main(){
    char* str_less_than = "ls < in > yo";
    char* result = find_input(str_less_than);
    printf("Result IN: %s.\n", result);
    printf("Original string: %s\n", str_less_than);

    char* str_greater_than = "ls < in > yo";
    char* result_out = find_output(str_greater_than);
    printf("Result OUT: %s.\n", result_out);
    printf("Original string: %s\n", str_greater_than);

    char* str_pipe = "2 | dfs |sdf|lld  | fgfg |ld";
    printf("Original string with pipes: %s\n", str_pipe);
    char** pipe_separated_strings = split_on_pipes(str_pipe);
    int i = 0;
    while(pipe_separated_strings[i] != NULL){
        printf("%d. argument after white space removed: %s\n", i+1 ,pipe_separated_strings[i]);
        i++;
    }

    // char str_greater_than[32] = "ls > out";
    // find_after_delimiter(str_greater_than, '>');
    

    return 0;
}