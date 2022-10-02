#ifndef ____LINKED_LIST___H___
#define ____LINKED_LIST___H___

typedef struct Process {
    int pid;
    char* cmdline_args;
    struct Process *next;
} Process;

void printList();

void insertFirst(int pid, char* cmdline_args);

struct Process* find(int pid);

struct Process* delete(int pid);

#endif