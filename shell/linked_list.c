#include "linked_list.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//source: https://www.tutorialspoint.com/data_structures_algorithms/linked_list_program_in_c.htm

struct Process *head = NULL;
struct Process *current = NULL;

//display the list
void printList() {
    struct Process *ptr = head;
    printf("\n");

    //start from the beginning
    while(ptr != NULL) {
        printf("PID: %d\tCMD: %s\n",ptr->pid,ptr->cmdline_args);
        ptr = ptr->next;
    }
    printf("\n");
}

//insert link at the first location
void insertFirst(int pid, char* cmdline_args) {
    //create a link
    struct Process *link = (struct Process*) malloc(sizeof(struct Process));
    link->pid = pid;
    link->cmdline_args = cmdline_args;

    //point it to old first process
    link->next = head;

    //point first to new first process
    head = link;
}

//delete first item
struct Process* deleteFirst() {

    //save reference to first link
    struct Process *tempLink = head;

    //mark next to first link as first 
    head = head->next;

    //return the deleted link
    free(tempLink);
    return tempLink;
}

//is list empty
int isEmpty() {
    return head == NULL;
}

int length() {
    int length = 0;
    struct Process *current;

    for(current = head; current != NULL; current = current->next) {
        length++;
    }

    return length;
}

//find a link with given pid
struct Process* find(int pid) {

    //start from the first link
    struct Process* current = head;

    //if list is empty
    if(head == NULL) {
        return NULL;
    }

    //navigate through list
    while(current->pid != pid) {

        //if it is last process
        if(current->next == NULL) {
            return NULL;
        } else {
            //go to next link
            current = current->next;
        }
    }      

    //if data found, return the current Link
    return current;
}

//delete a link with given pid
struct Process* delete(int pid) {

    //start from the first link
    struct Process* current = head;
    struct Process* previous = NULL;

    //if list is empty
    if(head == NULL) {
        return NULL;
    }

    //navigate through list
    while(current->pid != pid) {

        //if it is last process
        if(current->next == NULL) {
            return NULL;
        } else {
            //store reference to current link
            previous = current;
            //move to next link
            current = current->next;
        }
    }

    //found a match, update the link
    if(current == head) {
        //change first to point to next link
        head = head->next;
    } else {
        //bypass the current link
        previous->next = current->next;
    }    
    free(current);
    return current;
}

void tests() {
    insertFirst(1,"ls -al");
    insertFirst(2,"cd");
    insertFirst(3,"cd &");
    insertFirst(4,"yeet flush");
    insertFirst(5,"wtf man");
    insertFirst(6,"sweet baby jesus");
    insertFirst(7,"ls -al &");
    insertFirst(7,"ls -al &");

    printf("Original List: "); 

    //print list
    printList();
}