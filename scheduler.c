/* Fill in your Name and GNumber in the following two comment fields
 * Name: Nooreldean Koteb
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clock.h"
#include "structs.h"
#include "constants.h"
#include "scheduler.h"

void scheduler_exit(Schedule *schedule);
Schedule *scheduler_init();
int scheduler_add(Schedule *schedule, Process *process);
int scheduler_stop(Schedule *schedule, int pid);
int scheduler_continue(Schedule *schedule, int pid);
int scheduler_reap(Schedule *schedule, int pid);
Process *scheduler_generate(char *command, int pid, int time_remaining, int is_sudo);
Process *scheduler_select(Schedule *schedule);
int scheduler_count(List *ll);
void scheduler_free(Schedule *scheduler);


/* Called when the program terminates.
 * You may use this to clean up memory or for any other purpose.
 */
void scheduler_exit(Schedule *schedule) {

  //clean memory
  return;
}

/* Initialize the Schedule Struct
 * Follow the specification for this function.
 * Returns a pointer to the new Schedule or NULL on any error.
 */
Schedule *scheduler_init() {
  //Initializing Lists & schedule In Memory
  List *ready_list = (List *)malloc(sizeof(List));
  List *stopped_list = (List *)malloc(sizeof(List));
  List *defunct_list = (List *)malloc(sizeof(List));
  Schedule *schedule = (Schedule *)malloc(sizeof(Schedule));
  
  //Checking if there are any memeory issues
  if(ready_list == NULL || stopped_list == NULL 
  || defunct_list == NULL || schedule == NULL){
    return NULL;
  }
  
  //Initializing List Values
  ready_list->count = 0;
  ready_list->head = NULL;
  stopped_list->count = 0;
  stopped_list->head = NULL;
  defunct_list->count = 0;
  defunct_list->head = NULL;

  //Initializing Schedule Values
  schedule->ready_list = ready_list;
  schedule->stopped_list = stopped_list;
  schedule->defunct_list = defunct_list;

  return schedule;
}

/* Add the process into the appropriate linked list.
 * Follow the specification for this function.
 * Returns a 0 on success or a -1 on any error.
 */
int scheduler_add(Schedule *schedule, Process *process) {
  Process *temp;
  //Switch Case checking for state
  switch (process->flags & 0xF){
  //Created State
  case STATE_CREATED:
  //State Being changed to Ready
    process->flags = process->flags & 0xFFFFFFF0; 
    process->flags = process->flags | STATE_READY;
    //Finding the appropriate place to place the process in ready_list
    //Also adding to readylist count
    if (schedule->ready_list->head == NULL){
      schedule->ready_list->head = process;
    }else{
      temp = schedule->ready_list->head; 
      //Checks if PID of current proccess is lower than head
      if(process->pid < schedule->ready_list->head->pid){
        process->next = schedule->ready_list->head;
        schedule->ready_list->head = process;
        schedule->ready_list->count += 1;
        return 0;
      }else{
        //Loop that finds a proccess in ready list that has lower PID
        while(temp->next != NULL){
          if(process->pid < temp->next->pid){
            process->next = temp->next;
            temp->next = process;
            schedule->ready_list->count += 1;
            return 0;
          }
          temp = temp->next;
        }
        temp->next = process;
      }
    }
    //Adding 1 to readylist count
    schedule->ready_list->count += 1;
    return 0;
  
  //Ready State
  case STATE_READY:
  //Checks if time remaining is greater than 0
  //and finds a pid smaller than current proccess pid
  //to place proccess in ready_list
    if (process->time_remaining >0){
      //Checks head if not loops through readylist
      if (schedule->ready_list->head == NULL){
        schedule->ready_list->head = process;
        
      }else{

        temp = schedule->ready_list->head; 
        if(process->pid < schedule->ready_list->head->pid){
          process->next = schedule->ready_list->head;
          schedule->ready_list->head = process;
          schedule->ready_list->count += 1;
          return 0;
        }else{
          while(temp->next != NULL){
            if(process->pid < temp->next->pid){
              process->next = temp->next;
              temp->next = process;
              schedule->ready_list->count += 1;
              return 0;
            }
            temp = temp->next;
          }
          temp->next = process;
        }
      }
      schedule->ready_list->count += 1;
    }else{
      //Changing state to Defunct if time remaining is less than or == to 0
      process->flags = process->flags & 0xFFFFFFF0; 
      process->flags = process->flags | STATE_DEFUNCT;
      
      //checks if head is empty if not finds next appropriate spot to place proccess
      if (schedule->defunct_list->head == NULL){
        schedule->defunct_list->head = process;
      }else{
        //Checks if head proccess is better than current proccess
        temp = schedule->defunct_list->head; 
        if(process->pid < schedule->defunct_list->head->pid){
          process->next = schedule->defunct_list->head;
          schedule->defunct_list->head = process;
          schedule->defunct_list->count += 1;
          return 0;
        }else{
          //loops through defunct list to find where to place proccess based on PID
          while(temp->next != NULL){
            if(process->pid < temp->next->pid){
              process->next = temp->next;
              temp->next = process;
              schedule->defunct_list->count += 1;
              return 0;
            }
            temp = temp->next;
          }
          temp->next = process;
          
        }
      }
      //increments defunct list
      schedule->defunct_list->count += 1;
    }

    return 0;
  //Defunct State
  case STATE_DEFUNCT:
    //Checks if defunct list head is empty
    if (schedule->defunct_list->head == NULL){
      schedule->defunct_list->head = process;
    }else{
      //Loops through defunct list to find last proccess
      temp = schedule->defunct_list->head; 
      while(temp->next != NULL){
        temp = temp->next;
      }
      //Places new proccess at the end of the list
      temp->next = process;
    }
    //Increments defunct list
    schedule->defunct_list->count += 1;
    return 0;
  }

  return -1;
}

/* Move the process with matching pid from Ready to Stopped.
 * Change its State to Stopped.
 * Follow the specification for this function.
 * Returns a 0 on success or a -1 on any error.
 */
int scheduler_stop(Schedule *schedule, int pid) {
  Process *temp;
  Process *tempProc;

  //Initialize pointers
  temp = schedule->ready_list->head;
  tempProc = NULL;

  //Check if head is empty and returns error if true
  if(schedule->ready_list->head == NULL){
    return -1;
  }

  //Checks if head is the proccess we are looking for
  if(schedule->ready_list->head->pid == pid){
    tempProc = schedule->ready_list->head;

    //Removes proccess from list
    if (temp->next != NULL){
      schedule->ready_list->head = temp->next;
    }else{
      schedule->ready_list->head = NULL;
    }

  }else{
      //Loops through list to find proccess and remove it
      while (temp->next != NULL){
        if(temp->next->pid == pid){
          
          tempProc = temp->next;
          if(temp->next->next != NULL){
            temp->next = temp->next->next;
          }else{
            temp->next = NULL;
          }

          return 0;
        }else{
          temp = temp->next;
        }
      }
      //Checks if proccess was found if not returns error
      if (tempProc == NULL){
        return -1;
      }
  }

  //Increments readylist count
  schedule->ready_list->count -= 1;
  //converts flag to stopped state
    tempProc->flags = tempProc->flags & 0xFFFFFFF0;
    tempProc->flags = tempProc->flags | STATE_STOPPED;

  //Checks if head is empty if so places removed proccess there
  if (schedule->stopped_list->head == NULL){
    schedule->stopped_list->head = tempProc;

  }else{
    //checks if removed proccess is better than stopped head
    temp = schedule->stopped_list->head; 
    if(tempProc->pid < schedule->stopped_list->head->pid){
      tempProc->next = schedule->stopped_list->head;
      schedule->stopped_list->head = tempProc;
    }else{

      //Loops through stopped list and places proccess there
      while(temp->next != NULL){
        if(tempProc->pid < temp->next->pid){
          tempProc->next = temp->next;
          temp->next = tempProc;
          schedule->stopped_list->count += 1;
          return 0;
        }else{
          temp = temp->next;
        }
      }
      temp->next = tempProc;
    }
  }

  //Increments stopped list count
  schedule->stopped_list->count += 1; 
  return 0;
}

/* Move the process with matching pid from Stopped to Ready.
 * Change its State to Ready.
 * Follow the specification for this function.
 * Returns a 0 on success or a -1 on any error.
 */
int scheduler_continue(Schedule *schedule, int pid) {
  Process *temp;
  Process *tempProc;

  //Initialize pointers
  temp = schedule->stopped_list->head;
  tempProc = NULL;

  //If stopped head is empty return error
  if(schedule->stopped_list->head == NULL){
    return -1;
  }

  //If stopped list has proccess wanted save it and remove it from list
  if(schedule->stopped_list->head->pid == pid){
    tempProc = schedule->stopped_list->head;

    //Remove proccess from list
    if (temp->next != NULL){
      schedule->stopped_list->head = temp->next;
    }else{
      schedule->stopped_list->head = NULL;
    }

  }else{

      //Loops through stopped list to find wanted process
      while (temp->next != NULL){
        if(temp->next->pid == pid){
          tempProc = temp->next;
          if(temp->next->next != NULL){
            temp->next = temp->next->next;
          }else{
            temp->next = NULL;
          }
          return 0;
        }else{
          temp = temp->next;
        }
      }

      //If proccess wasnt found return error
      if (tempProc == NULL){
        return -1;
      }
  }
  
  //Increments stopped list count
  schedule->stopped_list->count -= 1;
  //Change flag to ready
    tempProc->flags = tempProc->flags & 0xFFFFFFF0;
    tempProc->flags = tempProc->flags | STATE_READY;

  //If ready list is empty place removed proccess there
  if (schedule->ready_list->head == NULL){
    schedule->ready_list->head = tempProc;
  }else{

    //Checks if head is better than new proccess if not sets proccess there
    temp = schedule->ready_list->head; 
    if(tempProc->pid < schedule->ready_list->head->pid){
      tempProc->next = schedule->ready_list->head;
      schedule->ready_list->head = tempProc;
    }else{

      //Loops to find the right place to insert removed proccess
      while(temp->next != NULL){
        if(tempProc->pid < temp->next->pid){
          tempProc->next = temp->next;
          temp->next = tempProc;
          schedule->ready_list->count += 1;
          return 0;
        }else{
          temp = temp->next;
        }
      }
      temp->next = tempProc;
    }
  }

  //Increments ready list count
  schedule->ready_list->count += 1;
  return 0;
}

/* Remove the process with matching pid from Defunct.
 * Follow the specification for this function.
 * Returns its exit code (from flags) on success or a -1 on any error.
 */
int scheduler_reap(Schedule *schedule, int pid) {
  Process *temp;
  Process *tempProc;

  //Initialize pointers
  temp = schedule->defunct_list->head;
  tempProc = NULL;

  //Checks if head is null if so returns error
  if(schedule->defunct_list->head == NULL){
    return -1;
  }

  //If proccess is in head remove it and save it
  if(schedule->defunct_list->head->pid == pid){
    tempProc = temp;

    //remove proccess from list
    if (temp->next != NULL){
      schedule->defunct_list->head = temp->next;
    }else{
      schedule->defunct_list->head = NULL;
    }

  }else{

      //Loops through list and finds proccess
      while (temp->next != NULL){
        if(temp->next->pid == pid){
          tempProc = temp->next;
          
          if(temp->next->next != NULL){
            temp->next = temp->next->next;
          }else{
            temp->next = NULL;
          }
        }else{
          temp = temp->next;
        }
      }

      //If process was not found return error
      if (tempProc == NULL){
        return -1;
      }
  }

  //Increments defunct list count
  schedule->defunct_list->count -= 1;

  //Changes flags
  tempProc->flags = tempProc->flags & 0xFFFFFFF0; 
  tempProc->flags = tempProc->flags | STATE_TERMINATED;

  //Removes exit code and Frees the proccess
  int exit = tempProc->flags >>6;
  free(tempProc->command);
  free(tempProc);

  return exit;
}

/* Create a new Process with the given information.
 * - Malloc and copy the command string, don't just assign it!
 * Set the STATE_CREATED flag.
 * If is_sudo, also set the PF_SUPERPRIV flag.
 * Follow the specification for this function.
 * Returns the Process on success or a NULL on any error.
 */
Process *scheduler_generate(char *command, int pid, int time_remaining, int is_sudo) {
  Process *process = (Process *)malloc(sizeof(Process));
  char *cmd = (char *)malloc(strlen(command)*sizeof(char));

  //Checks If memeory intialization failed
  if (process == NULL || cmd == NULL){
    return NULL;
  }

  //copies string from command to malloced pointer
  strcpy(cmd, command);

  //Initializing proccess values
  process->command = cmd;
  process->pid = pid;
  process->flags = STATE_CREATED;
  process->next = NULL;

  //Checks if sudo is used and if so flag is set to reflect that
  if (is_sudo == 1){
    process->flags = process->flags & 0xEF;
    process->flags = process->flags | PF_SUPERPRIV;
  }

  //rest of initializations
  process->time_remaining = time_remaining;
  process->time_last_run = clock_get_time();
  return process;
}

/* Select the next process to run from Ready List.
 * Follow the specification for this function.
 * Returns the process selected or NULL if none available or on any errors.
 */
Process *scheduler_select(Schedule *schedule) {
  Process *prev;
  Process *temp;
  Process *starved = NULL;
  Process *lowest = NULL;

  //Initializing pointer
  temp = schedule->ready_list->head;

  //Checking if head is null if so returns error
  if(schedule->ready_list->head == NULL){
    return NULL;
  }
  
  //Checks if head is starved
  if(clock_get_time() - temp->time_last_run >= TIME_STARVATION){
      prev = temp;
      starved = temp;
  }

  //Loops through rest of the list checking for priority starved proccesses
  while(temp->next != NULL){
    if(clock_get_time() - temp->next->time_last_run >= TIME_STARVATION){
      if(starved == NULL){
        prev = temp;
        starved = temp->next;
      }else{
        if(temp->next->pid < starved->pid){
          prev = temp;
          starved = temp->next;
        }
      }
    }
    temp = temp->next;
  }

    //If Starved proccess is found return it
    if(starved != NULL){

      //removed proccess from list
      if(prev == starved){
        schedule->ready_list->head = NULL;
      }else{
        if(prev->next->next != NULL){
          prev->next = prev->next->next;
        }else{
          prev->next = NULL;
        }
      }
      //Increments ready list count and returns starved proccess
      schedule->ready_list->count -= 1;
      return starved;
    }else{

      //Pointers initialized
      temp = schedule->ready_list->head;
      prev = temp;
      lowest = temp;

      //Loops to find proccesses with lower time if no starved proccesses are found
      while (temp->next != NULL){
        if (temp->next->time_remaining <= lowest->time_remaining){
          if (temp->next->time_remaining == lowest->time_remaining){
            if (temp->next->pid < lowest->pid){
              prev = temp;
              lowest = temp->next;
            }
          }else{
            prev = temp;
            lowest = temp->next;
          }
        }
        temp = temp->next;
      }

      //If the head is the lowest time this will remove it from the head
      if(prev == lowest){
        if(prev->next != NULL){
          schedule->ready_list->head = prev->next;
        }else{
          schedule->ready_list->head = NULL;
        }
        
      }else{

        //Removes proccess from list
        if(prev->next->next != NULL){
          prev->next = prev->next->next;
        }else{
          prev->next = NULL;
        }
      }

      //Increments ready list count and returns it
      schedule->ready_list->count -= 1;
      return lowest;
    }

  return NULL;
}

/* Returns the number of items in a given List
 * Follow the specification for this function.
 * Returns the count of the List, or -1 on any errors.
 */
int scheduler_count(List *ll) {
  //Checks if list is empty and returns and error
  //If not empty than it returns the count
  if (ll == NULL){
    return -1;
  }else{
    return ll->count;
  }
}

/* Completely frees all allocated memory in the scheduler
 * Follow the specification for this function.
 */
void scheduler_free(Schedule *scheduler) {
  Process *temp;
  Process *delTemp;

  //If schedule is empty does nothing
  if(scheduler == NULL){
    return;
  }

  //Removes Ready List and all proccesses withing it
  if(scheduler->ready_list != NULL){
    if(scheduler->ready_list->head != NULL){
      temp = scheduler->ready_list->head;
      
      if (temp->next == NULL){
        free(scheduler->ready_list->head->command);
        free(scheduler->ready_list->head);
      }else{
        if (temp->next->next == NULL){
          free(temp->next->command);
          free(temp->next);
          free(temp->command);
          free(temp);
        }else{
          while(temp->next->next != NULL){
            delTemp = temp->next;
            temp->next = temp->next->next;
            free(delTemp->command);
            free(delTemp);
          }
          free(temp->next->command);
          free(temp->next);
          free(temp->command);
          free(temp);
        }
      }
    }
    free(scheduler->ready_list);
  }

  //Removes Stopped List and all proccesses within it
  if(scheduler->stopped_list != NULL){
    if(scheduler->stopped_list->head != NULL){
      temp = scheduler->stopped_list->head;

      if (temp->next == NULL){
        free(scheduler->stopped_list->head->command);
        free(scheduler->stopped_list->head);
      }else{
        if (temp->next->next == NULL){
          free(temp->next->command);
          free(temp->next);
          free(temp->command);
          free(temp);
        }else{
          while(temp->next->next != NULL){
            delTemp = temp->next;
            temp->next = temp->next->next;
            free(delTemp->command);
            free(delTemp);
          }
          free(temp->next->command);
          free(temp->next);
          free(temp->command);
          free(temp);
        }
      }
    }
    free(scheduler->stopped_list);
  }

  //Removes Defunct List and all proccesses within it
  if(scheduler->defunct_list != NULL){
    if(scheduler->defunct_list->head != NULL){
      temp = scheduler->defunct_list->head;

      if (temp->next == NULL){
        free(scheduler->defunct_list->head->command);
        free(scheduler->defunct_list->head);
      }else{
        if (temp->next->next == NULL){
          free(temp->next->command);
          free(temp->next);
          free(temp->command);
          free(temp);
        }else{
          while(temp->next->next != NULL){
            delTemp = temp->next;
            temp->next = temp->next->next;
            free(delTemp);
            free(delTemp->command);
          }
          free(temp->next->command);
          free(temp->next);
          free(temp->command);
          free(temp);
        }
      }
    }
    free(scheduler->defunct_list);
  }

  //removes schedule
  free(scheduler);
  return;
}
 
