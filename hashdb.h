#ifndef HASHDB_H
#define HASHDB_H

#include <stdio.h>

#include <stdint.h>


typedef struct hash_struct {
    uint32_t hash;
    char name[50];
    uint32_t salary;
    struct hash_struct *next;
} hashRecord;

extern hashRecord *hashDBHead; // Declare hashDBHead globally

// Function declarations
hashRecord *insert(hashRecord **head, const char *name, uint32_t salary);
hashRecord *search(hashRecord *head, const char *name);
hashRecord *delete(hashRecord **head, const char *name);
void printHashDB(hashRecord *head, FILE *outputFile);
uint32_t jenkins_one_at_a_time_hash(const uint8_t *key, size_t length);

#endif
