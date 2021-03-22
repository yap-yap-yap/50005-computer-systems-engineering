#include "shellPrograms.h"

/*  A program that prints how many summoned daemons are currently alive */
int shellCheckDaemon_code()
{

   /* TASK 8 */
   //Create a command that trawl through output of ps -efj and contains "summond"
   char *command = malloc(sizeof(char) * 256);

   // there is no 'tty' in the output on my machine. i simply exclude 'grep' since among the processes containing 'summond', i want to exclude the one that has 'grep'. 
   sprintf(command, "ps -efj | grep summond  | grep -v grep > output.txt");

   // TODO: Execute the command using system(command) and check its return value
   while(1){
      if (system(command) != -1){
         break;
      }
   }

   free(command);

   int live_daemons = 0;
   // TODO: Analyse the file output.txt, wherever you set it to be. You can reuse your code for countline program
   // 1. Open the file
   // 2. Fetch line by line using getline()
   // 3. Increase the daemon count whenever we encounter a line
   // 4. Close the file
   // 5. print your result

   ssize_t nread;
   char *line = NULL;
   size_t len = 0;
   
   // 1. Open the file
   FILE *fp = fopen("output.txt", "r");
   if(fp != NULL){
      // 2. Fetch line by line using getline()
      while((nread = getline(&line, &len, fp)) != -1){
         // 3. Increase the daemon count whenever we encounter a line
         live_daemons++;
         fwrite(line, nread, 1, stdout);
      }
   }

   // 4. Close the file
   fclose(fp);
   // 5. print your result
   printf("live daemon count: %d\n", live_daemons);

   if (live_daemons == 0){
      printf("No daemon is alive right now\n");
   }
   else if(live_daemons == 1){
      printf("There are in total of %d live daemon \n", live_daemons);
   }
   else{
      printf("There are in total of %d live daemons \n", live_daemons);
   }

   // TODO: close any file pointers and free any dynamically allocated memory

   return 1;
}

int main(int argc, char **args)
{
   return shellCheckDaemon_code();
}