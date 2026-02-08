#include <stdio.h>

#define MAX_SIZE 100

/* Data structure definition */
typedef struct {
    int data[MAX_SIZE];
    int size;
} IntList;

/* Function prototypes */
void initList(IntList *list);
int insertEnd(IntList *list, int value);
int insertAt(IntList *list, int index, int value);
int removeAt(IntList *list, int index);
int search(const IntList *list, int value);
void printList(const IntList *list);
void printMenu(void);

int main(void) {
    IntList list;
    int choice;
    int value;
    int index;
    int result;

    initList(&list);

    do {
        printMenu();
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf("Enter value to insert at end: ");
                scanf("%d", &value);
                result = insertEnd(&list, value);
                if (result == -1) {
                    printf("List is full.\n");
                }
                break;

            case 2:
                printf("Enter index: ");
                scanf("%d", &index);
                printf("Enter value: ");
                scanf("%d", &value);
                result = insertAt(&list, index, value);
                if (result == -1) {
                    printf("Invalid index or list is full.\n");
                }
                break;

            case 3:
                printf("Enter index to remove: ");
                scanf("%d", &index);
                result = removeAt(&list, index);
                if (result == -1) {
                    printf("Invalid index.\n");
                }
                break;

            case 4:
                printf("Enter value to search: ");
                scanf("%d", &value);
                result = search(&list, value);
                if (result == -1) {
                    printf("Value not found.\n");
                } else {
                    printf("Value found at index %d.\n", result);
                }
                break;

            case 5:
                printList(&list);
                break;

            case 0:
                printf("Exiting program.\n");
                break;

            default:
                printf("Invalid choice.\n");
        }

    } while (choice != 0);

    return 0;
}

/* Initializes the list */
void initList(IntList *list) {
    /* Set list size to 0 */
    list->size = 0;
}

/* Inserts a value at the end of the list */
int insertEnd(IntList *list, int value) {
    /* 1. Check if list is full */
    if (list->size >= MAX_SIZE) {
        return -1;
    }

    /* 2. Insert value at the end */
    list->data[list->size] = value;

    /* 3. Update size */
    (list->size)++;

    return 0; /* operation complete successfully */
}

/* Inserts a value at a specific index */
int insertAt(IntList *list, int index, int value) {
    /* 1. Validate index */
    if (index < 0 || index > list->size || list->size >= MAX_SIZE) {
        return -1;
    }

     /* 2. Shift elements to the right */
    for (int i = list->size; i > index; i--) {
        list->data[i] = list->data[i - 1];
    }

    /* 3. Insert value */
    list->data[index] = value;

    /* 4. Update size */
    (list->size)++;

    return 0; /* operation complete successfully */
}

/* Removes the element at a specific index */
int removeAt(IntList *list, int index) {
    /* 1. Validate index */
    if (index < 0 || index >= list->size) {
        return -1;
    }

    /* 2. Shift elements to the left */
    for (int i = index; i < list->size - 1; i++) {
        list->data[i] = list->data[i + 1];
    }

    /* 3. Update size */
    (list->size)--;

    return 0; /* operation complete successfully */
}

/* Searches for a value and returns its index or -1 */
int search(const IntList *list, int value) {
    /* Perform a linear search */
    for (int i = 0; i < list->size; i++) {
        if (list->data[i] == value) {
            return i;
        }
    }

    return -1;  /* operation unsuccessful */
}

/* Prints all elements in the list */
void printList(const IntList *list) {
    if (list->size == 0) {
        printf("List is empty.\n");
        return;
    }

    printf("All elements in the list: ");
    for (int i = 0; i < list->size; i++) {
        printf("%d ", list->data[i]);
    }
    printf("\n");
}

/* Prints the menu options */
void printMenu(void) {
    printf("\nMenu:\n");
    printf("1. Insert at end\n");
    printf("2. Insert at index\n");
    printf("3. Remove at index\n");
    printf("4. Search\n");
    printf("5. Print list\n");
    printf("0. Exit\n");
}