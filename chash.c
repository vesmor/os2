#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "hashdb.h"
#include "rwlocks.h"

extern rwlock_t mutex;
FILE *outputFile;

int lockAcquisitions = 0;
int lockReleases = 0;

void logCommand(const char *command, const char *name, const char *salaryStr ) {
	if (strcmp(command, "print") == 0||strcmp(command, "search") == 0){
		return;
	}
	uint32_t hash = jenkins_one_at_a_time_hash((const uint8_t *)name, strlen(name));
	fprintf(outputFile, "%s,%u,%s,%s\n", command,hash, name, salaryStr);
}

void logLock(const char *lockType) {
	fprintf(outputFile, "%s LOCK ACQUIRED\n", lockType);
	lockAcquisitions++;
}

void logUnlock(const char *lockType) {
	fprintf(outputFile, "%s LOCK RELEASED\n", lockType);
	lockReleases++;
}

void executeCommand(const char *command, const char *name, const char *salaryStr) {
	uint32_t salary = strtoul(salaryStr, NULL, 10);
	// if (strcmp(command, "print") == 0||strcmp(command, "search") == 0){

	logCommand(command, name, salaryStr);

	if (strcmp(command, "insert") == 0) {
		rwlock_acquire_writelock(&mutex);
		logLock("WRITE");
		insert(&hashDBHead, name, salary);
		logUnlock("WRITE");
		rwlock_release_writelock(&mutex);
	}  if (strcmp(command, "delete") == 0) {
		rwlock_acquire_writelock(&mutex);
		logLock("WRITE");
		delete(&hashDBHead, name);
		logUnlock("WRITE");
		rwlock_release_writelock(&mutex);
	}  if (strcmp(command, "search") == 0) {
		fprintf(outputFile ,"SEARCH,%s\n",name);

		rwlock_acquire_readlock(&mutex);
		logLock("READ");
		hashRecord *result = search(hashDBHead, name);
		if (result != NULL) {
			uint32_t hash = jenkins_one_at_a_time_hash((const uint8_t *)result->name, strlen(result->name));
			fprintf(outputFile, "%u,%s,%u\n",hash, result->name, result->salary);
		} else {
			fprintf(outputFile, "No record found\n");
		}
		logUnlock("READ");
		rwlock_release_readlock(&mutex);
	}  if (strcmp(command, "print") == 0) {
		rwlock_acquire_readlock(&mutex);
		logLock("READ");
		printHashDB(hashDBHead, outputFile);
		logUnlock("READ");
		rwlock_release_readlock(&mutex);
	}
}

int extractThreadCount(const char *str) {
	char *token;
	char temp[100]; // Temporary buffer to hold the input string

	// Copy the input string to the temporary buffer
	strcpy(temp, str);

	// Tokenize the string using comma as the delimiter
	token = strtok(temp, ",");
	token = strtok(NULL, ","); // Move to the second token

	if (token != NULL) {
		int threads = atoi(token); // Convert the token to an integer
		return threads;
	} else {
		// Return -1 to indicate an error
		return -1;
	}
}

void *threadFunction(void *arg) {
	char *cmd = (char *)arg;
	char command[10], name[50], salaryStr[20];
	sscanf(cmd, "%[^,],%[^,],%s", command, name, salaryStr);
	executeCommand(command, name, salaryStr);
	free(arg);
	return NULL;
}

int main() {
	outputFile = fopen("output.txt", "w");
	if (!outputFile) {
		perror("Failed to open output file");
		return EXIT_FAILURE;
	}

	FILE *inputFile = fopen("commands.txt", "r");
	if (!inputFile) {
		perror("Failed to open commands file");
		return EXIT_FAILURE;
	}

	rwlock_init(&mutex);
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	pthread_t threads[100]; // assuming a max of 100 commands for simplicity
	int tidx = 0;

	if ((read = getline(&line, &len, inputFile)) != -1) {
		char *temp = strdup(line); // Store the first line in temp
		fprintf(outputFile,"Running %d threads\n", extractThreadCount(temp));
		free(temp); // Free the memory allocated for temp
	}

	// Read and process the remaining lines
	while ((read = getline(&line, &len, inputFile)) != -1) {
		char *cmd = strdup(line);
		pthread_create(&threads[tidx++], NULL, threadFunction, cmd);
	}
	for (int i = 0; i < tidx; i++) {
		pthread_join(threads[i], NULL);
	}

	// fclose(inputFile);
	// fclose(outputFile);
	if (line) {
		free(line);
	}

	// Log final lock acquisition and release
	logLock("READ");
	rwlock_acquire_readlock(&mutex);
	printHashDB(hashDBHead, outputFile);
	rwlock_release_readlock(&mutex);
	// Print number of lock acquisitions and releases
	fprintf(outputFile, "Number of lock acquisitions: %d\n", lockAcquisitions);
	fprintf(outputFile, "Number of lock releases: %d\n", lockReleases);
	logUnlock("READ");

	return EXIT_SUCCESS;
}
