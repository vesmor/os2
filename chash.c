#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "hashdb.h"
#include "rwlocks.h"

extern rwlock_t mutex;
FILE *outputFile;

void logCommand(const char *command, const char *name, const char *salaryStr) {
    fprintf(outputFile, "%s,%s,%s\n", command, name, salaryStr);
}

void logLock(const char *lockType) {
    fprintf(outputFile, "%s LOCK %s\n", lockType, "ACQUIRED");
}

void logUnlock(const char *lockType) {
    fprintf(outputFile, "%s LOCK %s\n", lockType, "RELEASED");
}

void executeCommand(const char *command, const char *name, const char *salaryStr) {
    uint32_t salary = strtoul(salaryStr, NULL, 10);
    logCommand(command, name, salaryStr);

    if (strcmp(command, "insert") == 0) {
        logLock("WRITE");
        rwlock_acquire_writelock(&mutex);
        insert(&hashDBHead, name, salary);
        rwlock_release_writelock(&mutex);
        logUnlock("WRITE");
    } else if (strcmp(command, "delete") == 0) {
        logLock("WRITE");
        rwlock_acquire_writelock(&mutex);
        delete(&hashDBHead, name);
        rwlock_release_writelock(&mutex);
        logUnlock("WRITE");
    } else if (strcmp(command, "search") == 0) {
        logLock("READ");
        rwlock_acquire_readlock(&mutex);
        hashRecord *result = search(hashDBHead, name);
        if (result != NULL) {
            fprintf(outputFile, "%s,%s,%u\n", command, result->name, result->salary);
        } else {
            fprintf(outputFile, "No record found\n");
        }
        rwlock_release_readlock(&mutex);
        logUnlock("READ");
    } else if (strcmp(command, "print") == 0) {
        logLock("READ");
        rwlock_acquire_readlock(&mutex);
        printHashDB(hashDBHead, outputFile);
        rwlock_release_readlock(&mutex);
        logUnlock("READ");
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

    while ((read = getline(&line, &len, inputFile)) != -1) {
        char *cmd = strdup(line);
        pthread_create(&threads[tidx++], NULL, threadFunction, cmd);
    }

    for (int i = 0; i < tidx; i++) {
        pthread_join(threads[i], NULL);
    }

    fclose(inputFile);
    fclose(outputFile);
    if (line) {
        free(line);
    }
    return EXIT_SUCCESS;
}
