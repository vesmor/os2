#include "hashdb.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "hashdb.h"

hashRecord *hashDBHead = NULL; // Initialize hashDBHead


uint32_t jenkins_one_at_a_time_hash(const uint8_t *key, size_t length) {
    uint32_t hash = 0;
    for (size_t i = 0; i < length; ++i) {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

hashRecord *insert(hashRecord **head, const char *name, uint32_t salary) {
    uint32_t hash = jenkins_one_at_a_time_hash((const uint8_t *)name, strlen(name));
    hashRecord **current = head;
    while (*current != NULL && (*current)->hash < hash) {
        current = &(*current)->next;
    }
    if (*current != NULL && (*current)->hash == hash && strcmp((*current)->name, name) == 0) {
        (*current)->salary = salary;
    } else {
        hashRecord *newNode = (hashRecord *)malloc(sizeof(hashRecord));
        newNode->hash = hash;
        strcpy(newNode->name, name);
        newNode->salary = salary;
        newNode->next = *current;
        *current = newNode;
    }
    return *current;
}

hashRecord *delete(hashRecord **head, const char *name) {
    uint32_t hash = jenkins_one_at_a_time_hash((const uint8_t *)name, strlen(name));
    hashRecord **current = head;
    while (*current != NULL && (*current)->hash != hash) {
        current = &(*current)->next;
    }
    if (*current != NULL && strcmp((*current)->name, name) == 0) {
        hashRecord *toDelete = *current;
        *current = (*current)->next;
        free(toDelete);
    }
    return *head;
}

hashRecord *search(hashRecord *head, const char *name) {
    uint32_t hash = jenkins_one_at_a_time_hash((const uint8_t *)name, strlen(name));
    while (head != NULL && head->hash != hash) {
        head = head->next;
    }
    return head;
}

void printHashDB(hashRecord *head, FILE *outputFile) {
    for (hashRecord *current = head; current != NULL; current = current->next) {
        fprintf(outputFile, "%u,%s,%u\n", current->hash, current->name, current->salary);
    }
}
