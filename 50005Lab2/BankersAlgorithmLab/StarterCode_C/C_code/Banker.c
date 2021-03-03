/**
 * CSE lab project 2 -- C version
 * 
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int numberOfCustomers = 0; // the number of customers
int numberOfResources = 0; // the number of resources

int *available;	  // the available amount of each resource
int **maximum;	  // the maximum demand of each customer
int **allocation; // the amount currently allocated
int **need;		  // the remaining needs of each customer

// Utility function to allocate an int vector.
int *mallocIntVector(int size)
{
	int i, *a = malloc(sizeof(int) * size);
	for (i = 0; i < size; i++)
		a[i] = 0;
	return a;
}

// Utility function to free an int vector.
void freeIntVector(int *a)
{
	free(a);
}

// Utility function to allocate an int matrix.
int **mallocIntMatrix(int numRows, int numColumns)
{
	int i, j, **a = malloc(sizeof(int *) * (numRows + 1));
	for (i = 0; i < numRows; i++)
	{
		a[i] = malloc(sizeof(int) * numColumns);
		for (j = 0; j < numColumns; j++)
			a[i][j] = 0;
	}
	a[numRows] = 0;
	return a;
}

// Utility function to free an int matrix.
void freeIntMatrix(int **a)
{
	int i;
	for (i = 0; a[i] != 0; i++)
		free(a[i]);
	free(a);
}

/**
 * Initializes the state of the bank.
 * @param resources  An array of the available count for each resource.
 * @param m          The number of resources.
 * @param n          The number of customers.
 */
void initBank(int *resources, int m, int n)
{
	// Allocate memory for the vector and matrices
	available = mallocIntVector(m);
	need = mallocIntMatrix(n, m);
	allocation = mallocIntMatrix(n, m);
	maximum = mallocIntMatrix(n, m);

	// TODO: initialize the numberOfCustomers and numberOfResources
	numberOfCustomers = n;
	numberOfResources = m;

	// TODO: initialize the available vector
	//take the values from the input resources and put them in available
	for(int i = 0; i < m; i++){
		available[i] = resources[i];
	}
}

/**
 * Frees the memory used to store the state of the bank.
 */
void freeBank()
{
	freeIntVector(available);
	freeIntMatrix(need);
	freeIntMatrix(allocation);
	free(maximum);
}

/**
 * Prints the state of the bank.
 */
void printState()
{
	int i;
	int j;
	// TODO: print the current state with a tidy format
	printf("\nCurrent state:\n");

	// TODO: print available
	printf("Available:\n");
	for (i = 0; i < numberOfResources; i++)
	{
		printf("%d ", available[i]); // leave a space between each print
	}
	printf("\n"); // Leave a line after printing available

	// TODO: print maximum
	printf("Maximum:\n");
	for (i = 0; i < numberOfCustomers; i++)
	{
		for (j = 0; j < numberOfResources; j++)
		{
			printf("%d ", maximum[i][j]); // Leave a space between each print
		}
		printf("\n"); // Leave a line after each customer
	}
	printf("\n"); // Leave a line after printing maximum

	// TODO: print allocation
	printf("Allocation:\n");
	for (i = 0; i < numberOfCustomers; i++)
	{
		for (j = 0; j < numberOfResources; j++)
		{
			printf("%d ", allocation[i][j]); // Leave a space between each print
		}
		printf("\n"); // Leave a line after each customer
	}
	printf("\n"); // Leave a line after printing Allcoation

	// TODO: print need
	printf("Need:\n");
	for (i = 0; i < numberOfCustomers; i++)
	{
		for (j = 0; j < numberOfResources; j++)
		{
			printf("%d ", need[i][j]); // Leave a space bettern each print
		}
		printf("\n"); // Leave a line after each customer
	}
	printf("\n"); // Leave a line after printing need
}

/**
 * Sets the maximum number of demand of each resource for a customer.
 * @param customerIndex  The customer's index (0-indexed).
 * @param maximumDemand  An array of the maximum demanded count for each resource.
 */
void setMaximumDemand(int customerIndex, int *maximumDemand)
{
	// TODO: add customer, update maximum and need
	// since need = maximum when there is no allocation just initialise it here
	for(int i = 0; i < numberOfResources; i++){
		maximum[customerIndex][i] = maximumDemand[i];
		need[customerIndex][i] = maximumDemand[i];
	}
}

/**
 * Checks if the request will leave the bank in a safe state.
 * @param customerIndex  The customer's index (0-indexed).
 * @param request        An array of the requested count for each resource.
 * @return 1 if the requested resources will leave the bank in a
 *         safe state, else 0
 */
int checkSafe(int customerIndex, int *request)
{
	// Allocate temporary memory to copy the bank state.
	int *work = mallocIntVector(numberOfResources);
	int **tempNeed = mallocIntMatrix(numberOfCustomers, numberOfResources);
	int **tempAllocation = mallocIntMatrix(numberOfCustomers, numberOfResources);

	// 0 -> finish = false, 1 -> finish = true
	int *finish = mallocIntVector(numberOfCustomers);
	//a pointer for looping through the customers
	int i = 0;
	//-1 -> no current customer, find one to look through; 0 to numberOfCustomers-1 -> currently looking through this customer
	int current_customer = -1;
	// if loop doesn't find a safe path numberOfCustomer times in a row then it is a failure
	int failed_loop_count = 0;
	//if loop finds numberOfCustomer finishes in a row it can break out
	int success_loop_count = 0;


	// TODO: copy the bank's state to the temporary memory and update it with the request.
	for(int i = 0; i < numberOfResources; i++){
		work[i] = available[i];
		work[i] -= request[i];		

		for(int j = 0; j < numberOfCustomers; j++){
			tempNeed[j][i] = need[j][i];
			tempNeed[customerIndex][i] -= request[i];		
			tempAllocation[j][i] = allocation[j][i];
			tempAllocation[customerIndex][i] += request[i];
			finish[j] = 0;
		}
	}

	//debug printf("i managed to set up the temp arrays!\n");

	// TODO: check if the new state is safe
	//while loop lets it keep running until there is some kind of break in the middle
	while(1){
		//when faced with no current customer, iterates through 0 to numberOfCustomers-1 over and over
		// debug printf("i am here. current customer = %d, failed loops = %d, successful loops = %d.\n", current_customer, failed_loop_count, success_loop_count);
		if(current_customer < 0){
			i = i%numberOfCustomers;
			if(finish[i] == 0){
				current_customer = i;
				i++;
				continue;
			}
			i++;
			success_loop_count++;
		}
		
		//when faced with current customer, look through it to see if need <= work. update accordingly.
		else if(current_customer >= 0){
			
			//some scuffed looping since the for loop causes a segmentation fault for some reason
			int loop_failed = 0;
			int x = 0;
			while(x < numberOfResources){
				//if the current customer is a no go, set to no current customer and let it iterate again.
				if(tempNeed[current_customer][x] > work[x]){
					current_customer = -1;
					failed_loop_count++;
					success_loop_count = 0;
					//debug printf("there was a failed loop\n");
					
					loop_failed = 1;
					x = numberOfResources;
				}
				x++;
			}
			//if all is good then add the work
			if(loop_failed == 0){
				for(int j = 0; j < numberOfResources; j++){
					work[j] += tempAllocation[current_customer][j];
				}
				finish[current_customer] = 1;
				current_customer = -1;
				failed_loop_count = 0;
				//debug printf("there was a successful loop\n");
			}

			//debug printf("i reached here 2 \n");
			
		}

		//if it failed numberOfResources times in a row, it means there is no safe path
		//debug printf("i reached here/\n");
		if(failed_loop_count >= numberOfCustomers){
			return 0;
		}
		if(success_loop_count >= numberOfCustomers){
		//debug	printf("there is a safe path!\n");
			return 1;
		}

	}

	return 1;
}

/**
 * Requests resources for a customer loan.
 * If the request leave the bank in a safe state, it is carried out.
 * @param customerIndex  The customer's index (0-indexed).
 * @param request        An array of the requested count for each resource.
 * @return 1 if the requested resources can be loaned, else 0.
 */
int requestResources(int customerIndex, int *request)
{
	// TODO: print the request
	printf("Customer %d requesting\n", customerIndex);
	for (int i = 0; i < numberOfResources; i++)
	{
		printf("%d ", request[i]); // Leave a space between each request
	}
	printf("\n"); // Leave a line after each customer

	// TODO: judge if request larger than need
	// TODO: judge if request larger than available
	
	//DONT USE sizeof(request) HERE!!! IT TAKES LIKE THE WHOLE SIZE OF THE MALLOCED SPACE FOR THE VECTOR AND NOT JUST THE LENGTH OF THE ARRAY
	for(int i = 0; i < numberOfResources; i++){
		if(request[i] > need[customerIndex][i]){
			printf("request of amount %d of resource %d from customer %d rejected: requested more than it needed\n", request[i], i, customerIndex);
			return 1;
		}
		if(request[i] > available[i]){
			printf("request of amount %d of resource %d from customer %d rejected: requested more than is available\n", request[i], i, customerIndex);
			return 1;
		}
	}
	
	//debug printState();

	// TODO: judge if the new state is safe if grants this request (for question 2)
	if(checkSafe(customerIndex, request) != 1){
		return 1;
	}

	// TODO: request is granted, update state
	for(int i = 0; i < numberOfResources; i++){
		allocation[customerIndex][i] += request[i];
		available[i] -= request[i];
		need[customerIndex][i] -= request[i];
	}

	return 1;
}

/**
 * Releases resources borrowed by a customer. Assume release is valid for simplicity.
 * @param customerIndex  The customer's index (0-indexed).
 * @param release        An array of the release count for each resource.
 */
void releaseResources(int customerIndex, int *release)
{
	// TODO: print the release
	printf("Customer %d releasing\n", customerIndex);
	//taken from requestResources
	for (int i = 0; i < numberOfResources; i++)
	{
		printf("%d ", release[i]); // Leave a space between each request
	}
	printf("\n"); // Leave a line after each customer
	
	// TODO: deal with release (:For simplicity, we do not judge the release request, just update directly)
	for(int i = 0; i < numberOfResources; i++){
		allocation[customerIndex][i] -= release[i];
		available[i] += release[i];
		need[customerIndex][i] += release[i];
	}
}

/**
 * Parses and runs the file simulating a series of resource request and releases.
 * Provided for your convenience.
 * @param filename  The name of the file.
 */
void runFile(const char *filename)
{
	FILE *fp = fopen(filename, "r");
	int c = 0, i = 0, j = 0, m = 0, n = 0, bankInited = 0,
		lineLen = 0, prevLineEnd = 0, maxLineLen = 0;
	do
	{
		if (c == '\n' || c == EOF)
		{
			lineLen = i - prevLineEnd;
			prevLineEnd = i;
			if (lineLen > maxLineLen)
				maxLineLen = lineLen;
		}
		i++;
	} while ((c = fgetc(fp)) != EOF);

	rewind(fp);
	lineLen++;
	char *line = malloc(lineLen), *token;
	i = 0;
	while (fgets(line, lineLen, fp) != NULL)
	{
		for (j = 0; j < lineLen - 1; j++)
			if (line[j] == '\n')
				line[j] = '\0';
		if (i == 0)
		{
			token = strtok(line, ",");
			token = strtok(NULL, ",");
			n = atoi(token);
		}
		else if (i == 1)
		{
			token = strtok(line, ",");
			token = strtok(NULL, ",");
			m = atoi(token);
		}
		else if (i == 2)
		{
			token = strtok(line, ",");
			token = strtok(NULL, ",");
			int *resources = malloc(sizeof(int) * m);
			for (j = 0; j < m; j++)
			{
				resources[j] = atoi(strtok(j == 0 ? token : NULL, " "));
			}
			initBank(resources, m, n);
			bankInited = 1;
			free(resources);
		}
		else
		{
			int *resources = malloc(sizeof(int) * m);
			token = strtok(line, ",");
			if (strcmp(token, "c") == 0)
			{
				int customerIndex = atoi(strtok(NULL, ","));
				int *resources = malloc(sizeof(int) * m);
				token = strtok(NULL, ",");
				for (j = 0; j < m; j++)
				{
					resources[j] = atoi(strtok(j == 0 ? token : NULL, " "));
				}
				setMaximumDemand(customerIndex, resources);
				free(resources);
			}
			else if (strcmp(token, "r") == 0)
			{
				int customerIndex = atoi(strtok(NULL, ","));
				int *resources = malloc(sizeof(int) * m);
				token = strtok(NULL, ",");
				for (j = 0; j < m; j++)
				{
					resources[j] = atoi(strtok(j == 0 ? token : NULL, " "));
				}
				requestResources(customerIndex, resources);
				free(resources);
			}
			else if (strcmp(token, "f") == 0)
			{
				int customerIndex = atoi(strtok(NULL, ","));
				int *resources = malloc(sizeof(int) * m);
				token = strtok(NULL, ",");
				for (j = 0; j < m; j++)
				{
					resources[j] = atoi(strtok(j == 0 ? token : NULL, " "));
				}
				releaseResources(customerIndex, resources);
				free(resources);
			}
			else if (strcmp(token, "p") == 0)
			{
				printState();
			}
		}
		i++;
	}
	if (bankInited)
		freeBank();
	free(line);
	fclose(fp);
}

/**
 * Main function
 * @param args  The command line arguments
 */
int main(int argc, const char **argv)
{
	if (argc > 1)
	{
		runFile(argv[1]);
	}
	return 0;
}
