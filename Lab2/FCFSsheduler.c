/* File: FCFSscheduler.c */
/* Authors: Denis Garabajiu S4142551 and Radu Rusu S */
/* Description:  */

#include <stdlib.h>
#include <stdio.h>

// Function to safely allocate memory
void *safeMalloc(int sz) {
  void *p = calloc(sz, 1);
  if (p == NULL) {
    fprintf(stderr, "Fatal error: safeMalloc(%d) failed.\n", sz);
    exit(EXIT_FAILURE);
  }
  return p;
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

void freeArr(int **arr) {
  for(int i = 0; i < 1024; i++) {
    free(arr[i]);
  }
}

int main(int argc, char **argv) {
  int *process, **processArr;
  int isEOF = 0, index = 0, buffer_size = 1024;

  processArr = safeMalloc(buffer_size * sizeof(int));

  while(!isEOF) { 
    process = read_process(&isEOF);
    processArr[index] = process;
    index++;
    //printf("isEOF = %d\n", isEOF);

    // while(process[i] != -1){
    //   printf("%d, ", process[i]);
    //   i++;
    // }
    // printf("\n");
    // i = 0;

  }
  freeArr(processArr);
  return 0;
}