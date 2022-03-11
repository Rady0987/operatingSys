/* File: FCFSscheduler.c */
/* Authors: Denis Garabajiu S4142551 and Radu Rusu S */
/* Description:  */

#include <stdlib.h>
#include <stdio.h>

typedef struct Node {
   int index;
   struct Node *next;
} Node;

//typedef Node* node;

typedef struct Queue {
   Node *first;
   Node *last;
} Queue;

//typedef queue_t* queue;

// Function to safely allocate memory
void *safeMalloc(int sz) {
  void *p = calloc(sz, 1);
  if (p == NULL) {
    fprintf(stderr, "Fatal error: safeMalloc(%d) failed.\n", sz);
    exit(EXIT_FAILURE);
  }
  return p;
}

Node* makeNewNode(int num) {
   Node *n;
   n = safeMalloc(sizeof(Node));
   n->index = num;
   n->next = NULL;
   return n;
}

Queue* makeNewQueue() {
   Queue *q;
   q = safeMalloc(sizeof(Queue));
   q->first = NULL;
   q->last = NULL;
   return q;
}

void enqueueNode(Queue *q , int num) {
   Node *n = makeNewNode(num);
   if (q->first == NULL) {
      q->first = n;
      q->last = n;
   } else {
      q->last->next = n;
      q->last = q->last->next;
   }
}

Node* dequeueNode(Queue *q) {
   Node *aux;
   Node *n;
   if (q->first != NULL) {
      aux = q->first;
      q->first = q->first->next;
      n = aux;
      free(aux);
   }
   return n;
}

void freeQueue(Queue *q) {
   Node *n;
   while(q->first != NULL) {
      n = dequeueNode(q);
      printf("%d\n", n->index);
   }
   free(q);
}

//Function used to read the input line given by the user.
int *read_process(int *isEOF) {
  int buffer_size = 1024, index = 0;
  int *buffer = safeMalloc(buffer_size * sizeof(int));
  int c;

  while (1) {
    scanf("%d", &c);

    // If we hit EOF, replace it with a null character and return.
    if(c == -1) {
      //printf("sfarsit proces\n");
      c = getchar();
      if(c == EOF)
        *isEOF = 1;
      buffer[index] = -1;
      return buffer;
    } else {
      buffer[index] = c;
    }
    index++;

    // If we have exceeded the buffer, reallocate.
    if (index >= buffer_size) {
      printf("realoc\n");
      buffer_size += 1024;
      buffer = realloc(buffer, buffer_size);

      if (!buffer) {
        fprintf(stderr, "Shell: Buffer reallocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

void freeArr(int **arr, int size) {
   for(int i = 0; i < size; i++) {
      free(arr[i]);
   }
   free(arr);
}

int main(int argc, char **argv) {
   int *process, **processArr;
   int isEOF = 0, processNr = 0, buffer_size = 1024;

   processArr = safeMalloc(buffer_size * sizeof(int));

   Queue *q = makeNewQueue();

   while(!isEOF) { 
      process = read_process(&isEOF);
      processArr[processNr] = process;
      
      // //print test
      // int i = 0;
      // do {
      //    printf("%d, ", processArr[processNr][i]);
      //    i++;
      // } while (processArr[processNr][i] != -1);
      // printf("\n");
      // i = 0;

      processNr++;
   }
   
   enqueueNode(q,1); 
   enqueueNode(q,2); 
   enqueueNode(q,3); 
   enqueueNode(q,4); 
   dequeueNode(q);

   freeArr(processArr, processNr);
   freeQueue(q);
   return 0;
}