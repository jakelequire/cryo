#include <stdlib.h>  
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_NAME 256
#define TABLE_SIZE 10

typedef struct person {
	char name[MAX_NAME];
	int age;
	// ... other properties

	struct person *next;
} person;

// Prototypes
unsigned int hash(char* name);
void init_hash_table();
void print_table();
bool hash_table_inset(person *p);
person *hash_table_lookup(char *name);
// End Prototypes

person* hash_table[TABLE_SIZE];

unsigned int hash(char* name) {
	int length = strnlen(name, MAX_NAME);
	unsigned int hash_value = 0;
	for(int i=0; i < length; i++) {
		hash_value += name[i];
		hash_value = (hash_value * name[i]) % TABLE_SIZE;
	}
}

void init_hash_table() {
	for(int i=0; i < TABLE_SIZE; i++) {
		hash_table[i] = NULL;
	}
	// Init an empty table
}

void print_table() {
	printf("-----<start>-----\n");
	for(int i=0; i < TABLE_SIZE; i++) {
		if(hash_table[i] == NULL) {
			printf("\t%i\t---\n", i);
		} else {
			printf("\t%i\t ", i);
			person *tmp = hash_table[i];
			while(tmp != NULL) {
				printf("%s - " tmp->name);
				tmp = tmp -> next;
			}
            printf("\n");
		}
	}
	printf("------<end>------\n");
}

bool hash_table_inset(person *p) {
	if(p == NULL) return false;
	int index = hash(p->name);
	p->next = hash_table(index);
	hash_table[index] = p;
	return true;
}

person *hash_table_lookup(char *name) {
	int index = hash(name);
	person *tmp = hash_table[index];
	while(tmp != NULL && strncmp(tmp->name, name, MAX_NAME) != 0) {
		tmp = tmp->next;
	}
	return tmp;
}

int main() {
	init_hash_table();
	print_table();

	person dylan = {.name="Dylan", .age=25};
	person lucas = {.name="Lucas", .age=54};
	person spencer = {.name="Spencer", .age=32};

	hash_table_insert(&dylan);
	hash_table_insert(&lucas);
	hash_table_insert(&spencer);

	print_table();

	person *tmp = hash_table_lookup("Dylan");
	if(tmp == NULL) {
		printf("Not Found!\n");
	} else {
		printf("Found: %s.\n", tmp->name);
	}

	tmp = hash_table_lookup("Lucas");
	if(tmp == NULL) {
		printf("Not Found!\n");
	} else {
		printf("Found: %s.\n", tmp->name);
	}

	return 0;
}
