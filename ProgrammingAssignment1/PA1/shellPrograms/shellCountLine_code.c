#include "shellPrograms.h"

/*
Count the number of lines in a file 
*/
int shellCountLine_code(char **args)
{

    /** TASK 6  **/
    // ATTENTION: you need to implement this function from scratch and not to utilize other system program to do this
    // 1. Given char** args, open file in READ mode based on the filename given in args[1] using fopen()
    // 2. Check if file exists by ensuring that the FILE* fp returned by fopen() is not NULL
    // 3. Read the file line by line by using getline(&buffer, &size, fp)
    // 4. Loop, as long as getline() does not return -1, keeps reading and increment the count
    // 6. Close the FILE*
    // 7. Print out how many lines are there in this particular filename
    // 8. Return 1, to exit program

    ssize_t nread;
    char *line = NULL;
    size_t len = 0;
    int count = 0;
    printf("Checking %s...\n", args[1]);
    // 1. Given char** args, open file in READ mode based on the filename given in args[1] using fopen()
    FILE *fp = fopen(args[1], "r");
    // 2. Check if file exists by ensuring that the FILE* fp returned by fopen() is not NULL
    if (fp != NULL){
        // 3. Read the file line by line by using getline(&buffer, &size, fp)
        // 4. Loop, as long as getline() does not return -1, keeps reading and increment the count
        while((nread = getline(&line, &len, fp)) != -1){
            count++;
            // fwrite(line, nread, 1, stdout);
        }
    }
    // handling no file error
    if (fp == NULL){
        perror("CSEShell");
        exit(1);
    }

    // 6. Close the FILE*
    fclose(fp);

    // 7. Print out how many lines are there in this particular filename
    // selects appropriate grammar depending on the count value
    if(count == 1){
        printf("There is %d line in %s\n", count, args[1]);
    }
    else{
        printf("There are %d lines in %s\n", count, args[1]);
    }
    return 1;
}

int main(int argc, char **args)
{
    return shellCountLine_code(args);
}