# 50005Lab1

In order to to distribute jobs to the child processes, I ran a for loop that iterates through the job buffer for every input line from the file. At every loop, I check whether the process assigned to this index is alive (``waitpid(children_processes[i], NULL, WNOHANG) == 0``), as well as the status of the job (``shmPTR_jobs_buffer[i].task_status``). If it is alive and there is no job, a job is added, and the semaphore for that child process is.

If the process is not alive, I spawn a new process with ``fork()`` and assign the input line currently being processed to it, and then signal the new child with the semaphore.

When all the inputs from the file are read and assigned, a termination signal will be sent to all processes. The goal is to send the signal to only processes that are alive, and that they have completed their current job (if any). Thus, while iterating through the job buffer, and checking whether they are alive, I also busy wait if the child process has not completed its job (``while(shmPTR_jobs_buffer[i].task_status == 1);``). 