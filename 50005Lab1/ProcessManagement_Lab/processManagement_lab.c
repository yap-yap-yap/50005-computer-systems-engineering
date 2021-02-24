#include "processManagement_lab.h"

/**
 * The task function to simulate "work" for each worker process
 * TODO#3: Modify the function to be multiprocess-safe 
 * */
void task(long duration)
{
    // simulate computation for x number of seconds
    usleep(duration*TIME_MULTIPLIER);

    // TODO: protect the access of shared variable below
    //when task() is called it locks it with the semaphore, and unlocks it when done updating.
    sem_wait(sem_global_data);
    //debug //printf("global data semaphore has been acquired\n");
    // update global variables to simulate statistics
    ShmPTR_global_data->sum_work += duration;
    ShmPTR_global_data->total_tasks ++;
    if (duration % 2 == 1) {
        ShmPTR_global_data->odd++;
    }
    if (duration < ShmPTR_global_data->min) {
        ShmPTR_global_data->min = duration;
    }
    if (duration > ShmPTR_global_data->max) {
        ShmPTR_global_data->max = duration;
    }

    sem_post(sem_global_data);
    //debug //printf("global data semaphore has been released\n");
}


/**
 * The function that is executed by each worker process to execute any available job given by the main process
 * */
void job_dispatch(int i){

    // TODO#3:  a. Always check the corresponding shmPTR_jobs_buffer[i] for new  jobs from the main process
    //          b. Use semaphore so that you don't busy wait
    //          c. If there's new job, execute the job accordingly: either by calling task(), usleep, exit(3) or kill(getpid(), SIGKILL)
    //          d. Loop back to check for new job 
    
    //debug //printf("Hello from child %d with pid %d and parent id %d\n", i, getpid(), getppid());
    
    while(true){ //a. constanly looping to check job
        if(shmPTR_jobs_buffer[i].task_status == 0){
            sem_wait(sem_jobs_buffer[i]); //b. uses sem_wait() to avoid busy waiting
        }
        //debug //printf("child %d has acquired semaphore\n", i);
        //c. handling different job types
        if(shmPTR_jobs_buffer[i].task_status == 1){
            if(shmPTR_jobs_buffer[i].task_type == 't'){
                task(shmPTR_jobs_buffer[i].task_duration);
                //debug //printf("child %d has completed job: %c%d\n", i, shmPTR_jobs_buffer[i].task_type, shmPTR_jobs_buffer[i].task_duration);
                shmPTR_jobs_buffer[i].task_status = 0;
            }
            else if(shmPTR_jobs_buffer[i].task_type == 'w'){
                usleep(shmPTR_jobs_buffer[i].task_duration * TIME_MULTIPLIER);
                //debug //printf("child %d has completed job: %c%d\n", i, shmPTR_jobs_buffer[i].task_type, shmPTR_jobs_buffer[i].task_duration);
                shmPTR_jobs_buffer[i].task_status = 0;
            }
            else if(shmPTR_jobs_buffer[i].task_type == 'z'){
                shmPTR_jobs_buffer[i].task_status = 0;
                //debug //printf("child %d has completed job: %c%d\n", i, shmPTR_jobs_buffer[i].task_type, shmPTR_jobs_buffer[i].task_duration);
                exit(3);
            }
            else if(shmPTR_jobs_buffer[i].task_type == 'i'){
                shmPTR_jobs_buffer[i].task_status = 0;
                //debug //printf("child %d has completed job: %c%d\n", i, shmPTR_jobs_buffer[i].task_type, shmPTR_jobs_buffer[i].task_duration);
                kill(getpid(), SIGKILL);
            }
            else{
                printf("task type unknown. exiting...\n");
                exit(1);
            }
        }


    }
    //exit(0); 

}

/** 
 * Setup function to create shared mems and semaphores
 * **/
void setup(){

    // TODO#1:  a. Create shared memory for global_data struct (see processManagement_lab.h)
    //          b. When shared memory is successfully created, set the initial values of "max" and "min" of the global_data struct in the shared memory accordingly
    // To bring you up to speed, (a) and (b) are given to you already. Please study how it works. 

    //          c. Create semaphore of value 1 which purpose is to protect this global_data struct in shared memory 
    //          d. Create shared memory for number_of_processes job struct (see processManagement_lab.h)
    //          e. When shared memory is successfully created, setup the content of the structs (see handout)
    //          f. Create number_of_processes semaphores of value 0 each to protect each job struct in the shared memory. Store the returned pointer by sem_open in sem_jobs_buffer[i]
    //          g. Return to main

    //a. create shared memory for gloabl_data 
    ShmID_global_data = shmget(IPC_PRIVATE, sizeof(global_data), IPC_CREAT | 0666);
    if (ShmID_global_data == -1){
        printf("Global data shared memory creation failed\n");
        exit(EXIT_FAILURE);
    }
    ShmPTR_global_data = (global_data *) shmat(ShmID_global_data, NULL, 0);
    if ((int) ShmPTR_global_data == -1){
        printf("Attachment of global data shared memory failed \n");
        exit(EXIT_FAILURE);
    }

    //b. set global data min and max
    ShmPTR_global_data->max = -1;
    ShmPTR_global_data->min = INT_MAX;
    
    //c. create semaphore
    sem_global_data = sem_open("semglobaldata", O_CREAT | O_EXCL, 0644, 1); 
    //initialised at value = 1 because it should allow one process to access right from initialisation
    
    //loop this until it works basically
    while (true){
        if (sem_global_data == SEM_FAILED){
            sem_unlink("semglobaldata");
            sem_global_data = sem_open("semglobaldata", O_CREAT | O_EXCL, 0644, 1);
        }
        else{
            break;
        }
    }

    //d. create shared memory for number_of_processes
    ShmID_jobs = shmget(IPC_PRIVATE, sizeof(job)*number_of_processes, IPC_CREAT | 0666);
    if (ShmID_jobs == -1){
        printf("Process count shared memory creation failed\n");
        exit(EXIT_FAILURE);
    }

    //requires an array of length = number_of_processes, each space is a job struct
    shmPTR_jobs_buffer = (job *) shmat(ShmID_jobs, NULL, 0);
    if ((int) shmPTR_jobs_buffer == -1){
        printf("Attachment of job buffer shared memory failed \n");
        exit(EXIT_FAILURE);
    }

    //e. set up job buffer values
    //need to iterate through every job struct in the shm 
    for(int i = 0; i < number_of_processes; i++){
        shmPTR_jobs_buffer[i].task_type = 'z';
        shmPTR_jobs_buffer[i].task_duration = 0;
        shmPTR_jobs_buffer[i].task_status = 0;
    }

    //f. make semaphores to protect each job struct
    for(int i = 0; i < number_of_processes; i++){
        char name[BUFSIZ] = "";
        sprintf(name, "%s%d", "semjobs", i);
        //printf("%s\n", name);
        
        sem_jobs_buffer[i] = sem_open(name , O_CREAT | O_EXCL, 0644, 0);
        //initialised at 0 because it is meant to be a 'queue' 
        //loop this until it works basically
        while (true){
            if (sem_jobs_buffer[i] == SEM_FAILED){
                sem_unlink(name);
                sem_jobs_buffer[i] = sem_open(name, O_CREAT | O_EXCL, 0644, 0);
            }
            else{
                break;
            }
        }
    }
    

    return;

}

/**
 * Function to spawn all required children processes
 **/
 
void createchildren(){
    // TODO#2:  a. Create number_of_processes children processes
    //          b. Store the pid_t of children i at children_processes[i]
    //          c. For child process, invoke the method job_dispatch(i)
    //          d. For the parent process, continue creating the next children
    //          e. After number_of_processes children are created, return to main 

    //everything is accomplished here (i think)
    for(int i = 0; i < number_of_processes; i++){
        pid_t pid;
        
        pid = fork();

        if(pid < 0){
            printf("fork failed. exiting...\n");
            exit(1);
        }
        else if(pid > 0){
            children_processes[i] = pid;
        }
        else if(pid == 0){
            job_dispatch(i);
            break;
        }
        
    }

    return;
}

/**
 * The function where the main process loops and busy wait to dispatch job in available slots
 * */
void main_loop(char* fileName){

    // load jobs and add them to the shared memory
    FILE* opened_file = fopen(fileName, "r");
    char action; //stores whether its a 'p' or 'w'
    long num; //stores the argument of the job 
    bool line_assigned;

    while (fscanf(opened_file, "%c %ld\n", &action, &num) == 2) { //while the file still has input
        line_assigned = false;
        //debug //printf("new line read: %c%ld\n", action, num);
        //TODO#4: create job, busy wait
        //      a. Busy wait and examine each shmPTR_jobs_buffer[i] for jobs that are done by checking that shmPTR_jobs_buffer[i].task_status == 0. You also need to ensure that the process i IS alive using waitpid(children_processes[i], NULL, WNOHANG). This WNOHANG option will not cause main process to block when the child is still alive. waitpid will return 0 if the child is still alive. 
        //      b. If both conditions in (a) is satisfied update the contents of shmPTR_jobs_buffer[i], and increase the semaphore using sem_post(sem_jobs_buffer[i])
        //      c. Break of busy wait loop, advance to the next task on file 
        //      d. Otherwise if process i is prematurely terminated, revive it. You are free to design any mechanism you want. The easiest way is to always spawn a new process using fork(), direct the children to job_dispatch(i) function. Then, update the shmPTR_jobs_buffer[i] for this process. Afterwards, don't forget to do sem_post as well 
        //      e. The outermost while loop will keep doing this until there's no more content in the input file. 
        
        while(line_assigned == false){ //a. busy wait. needs to notice when the current line in the input file has been assigned, and move on.
            for(int i = 0; i < number_of_processes; i++){
                int alive = waitpid(children_processes[i], NULL, WNOHANG);

                if(alive == 0){
                    //b. when both conditions are true the job is added to the buffer, semaphore queue is updated to let a child in to work on it
                    if(shmPTR_jobs_buffer[i].task_status == 0){
                        shmPTR_jobs_buffer[i].task_type = action;
                        shmPTR_jobs_buffer[i].task_duration = num;
                        shmPTR_jobs_buffer[i].task_status = 1;
                        sem_post(sem_jobs_buffer[i]);
                        //debug //printf("process %d received job: %c%i\n", i, shmPTR_jobs_buffer[i].task_type, shmPTR_jobs_buffer[i].task_duration);
                        line_assigned = true;
                        break;
                    //c. break busy wait
                    }
                }
                //d. if child is dead, revive them and set the current task on the file reading loop to it.
                else if(alive != 0){
                    pid_t pid;
                    pid = fork();
                    
                    if(pid < 0){
                        printf("fork failed. exiting...\n");
                        exit(1);
                    }
                    else if(pid > 0){
                        children_processes[i] = pid;
                    }
                    else if(pid == 0){
                        job_dispatch(i);
                        shmPTR_jobs_buffer[i].task_type = action;
                        shmPTR_jobs_buffer[i].task_duration = num;
                        shmPTR_jobs_buffer[i].task_status = 1;
                        sem_post(sem_jobs_buffer[i]);
                        line_assigned = true;
                        
                    }

                }
            }
        }


    }
    fclose(opened_file);

    //debug //printf("Main process is going to send termination signals\n");

    // TODO#4: Design a way to send termination jobs to ALL worker that are currently alive 
    for(int i = 0; i < number_of_processes; i++){
        //checks both if the process is alive and if it is currently doing nothing
        int alive = waitpid(children_processes[i], NULL, WNOHANG);
        if(alive == 0){
            //since this loop doesn't go more than one round, busy wait here to make sure every process in progress completes before assigning task 'z'
            while(shmPTR_jobs_buffer[i].task_status == 1);
            shmPTR_jobs_buffer[i].task_type = 'z';
            shmPTR_jobs_buffer[i].task_duration = 0;
            shmPTR_jobs_buffer[i].task_status = 1;
            sem_post(sem_jobs_buffer[i]);

        }
    }


    //wait for all children processes to properly execute the 'z' termination jobs
    int process_waited_final = 0;
    pid_t wpid;
    while ((wpid = wait(NULL)) > 0){
        process_waited_final ++;
    }
    
    // print final results
    printf("Final results: sum -- %ld, odd -- %ld, min -- %ld, max -- %ld, total task -- %ld\n", ShmPTR_global_data->sum_work, ShmPTR_global_data->odd, ShmPTR_global_data->min, ShmPTR_global_data->max, ShmPTR_global_data->total_tasks);
}

void cleanup(){
    //TODO#4: 
    // 1. Detach both shared memory (global_data and jobs)
    // 2. Delete both shared memory (global_data and jobs)
    // 3. Unlink all semaphores in sem_jobs_buffer
    
    //copied from testmain_todo3.txt
    //detach and remove shared memory locations
    int detach_status = shmdt((void *) ShmPTR_global_data); //detach
    if (detach_status == -1) printf("Detach shared memory global_data ERROR\n");
    int remove_status = shmctl(ShmID_global_data, IPC_RMID, NULL); //delete
    if (remove_status == -1) printf("Remove shared memory global_data ERROR\n");
    detach_status = shmdt((void *) shmPTR_jobs_buffer); //detach
    if (detach_status == -1) printf("Detach shared memory jobs ERROR\n");
    remove_status = shmctl(ShmID_jobs, IPC_RMID, NULL); //delete
    if (remove_status == -1) printf("Remove shared memory jobs ERROR\n");


    //unlink all semaphores before exiting process
    int sem_close_status = sem_unlink("semglobaldata");
    if (sem_close_status == 0){
        //debug //printf("Semaphore globaldata closes succesfully.\n");
    }
    else{
        printf("Semaphore globaldata fails to close.\n");
    }

    for (int i = 0; i<number_of_processes; i++){
        char *sem_name = malloc(sizeof(char)*16);
        sprintf(sem_name, "semjobs%d", i);
        sem_close_status = sem_unlink(sem_name);
        if (sem_close_status == 0){
             //debug //printf("Semaphore jobs %d closes succesfully.\n", i);
        }
        else{
            printf("Semaphore jobs %d fails to close.\n", i);
        }
        free(sem_name);
    }

}

// Real main
int main(int argc, char* argv[]){

    //debug //printf("Lab 1 Starts...\n");

    struct timeval start, end;
    long secs_used,micros_used;

    //start timer
    gettimeofday(&start, NULL);

    //Check and parse command line options to be in the right format
    if (argc < 2) {
        printf("Usage: sum <infile> <numprocs>\n");
        exit(EXIT_FAILURE);
    }


    //Limit number_of_processes into 10. 
    //If there's no third argument, set the default number_of_processes into 1.  
    if (argc < 3){
        number_of_processes = 1;
    }
    else{
        if (atoi(argv[2]) < MAX_PROCESS) number_of_processes = atoi(argv[2]);
        else number_of_processes = MAX_PROCESS;
    }

    setup();
    createchildren();
    main_loop(argv[1]);

    //parent cleanup
    cleanup();

    //stop timer
    gettimeofday(&end, NULL);

    double start_usec = (double) start.tv_sec * 1000000 + (double) start.tv_usec;
    double end_usec =  (double) end.tv_sec * 1000000 + (double) end.tv_usec;

    printf("Your computation has used: %lf secs \n", (end_usec - start_usec)/(double)1000000);


    return (EXIT_SUCCESS);
}



