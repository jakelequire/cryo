#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 10

// Defines the structure of the Array
typedef struct Array {
    // Properties of the Array
    int *data;
    int size;
    int capacity;
    // Declare the function pointers for each Array method
    void (*push)(struct Array*, int);
    int (*pop)(struct Array*);
    int (*lengh)(struct Array*);
    void (*destroy)(struct Array*);
    // Maybe a destroy method is needed?
} Array;


void push(Array* array, int value) {
    if(array->size >= array->capacity) {
        array->capacity *= 2;
        array->data = realloc(array->data, array->capacity * sizeof(int));
        if(!array->data) {
            printf("Error in reallocating memory\n");
            exit(1);
        }
    }
    array->data[array->size] = value;
    array->size++;
}

int pop(Array* array) {
    if(array->size > 0) {
        array->size--;
        return array->data[array->size];
    } else {
        printf("Error: Array is empty and cannot remove an element.\n");
        return -1; // Indicates an Error
    }
}

int length(Array* array) {
    return array->size;
}

void destroy(Array* array) {
    
}

void initArray(Array* array) {
    array->size = 0;
    array->push = push;
    array->pop = pop;
}

int main() {
    Array array;
    initArray(&array);
    
    // Push Elements
    push(&array, 1);
    push(&array, 2);
    push(&array, 3);

    // Print Elements
    printf("\nArray Elements: \n");
    printf("[ ");
    for(int i=0; i < array.size; i++) {
        printf("%d ", array.data[i]);
    }
    printf("]");
    printf("\n");

    int arrLen = length(&array);
    printf("\nArrays Length: %i\n", arrLen);

    int value = pop(&array);
    printf("\nPopped an Element: [%d]\n", value);
    printf("[ ");
    for(int i=0; i < array.size; i++) {
        printf("{%d} ", array.data[i]);
    }
    printf("]");
    printf("\n");
    
    arrLen = length(&array);
    printf("\nArrays Length: %i\n", arrLen);

    return 0;
}
