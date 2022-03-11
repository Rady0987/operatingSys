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
int *read_line(int *index) {
  int buffer_size = 20;
  int *buffer = safeMalloc(buffer_size * sizeof(int));
  int c;

  while (scanf("%d", &c) != EOF || scanf("%d", &c) == '\n') {

    buffer[*index] = c;

    // If we have exceeded the buffer, reallocate.
    if (*index >= buffer_size) {
      buffer_size += 20;
      buffer = realloc(buffer, buffer_size);

      if (!buffer) {
        fprintf(stderr, "Shell: Buffer reallocation error\n");
        exit(EXIT_FAILURE);
      }
    }
    (*index)++;
  }
  return buffer;
}

int main(int argc, char **argv) {
  int frames, numPages = 0;
  int *referenceStream;
  int faultCounter = 0, flag;
  int *queue; // Data structure Queue (FIFO)

  scanf("%d ", &frames);
  queue = safeMalloc(frames * sizeof(int));
  referenceStream = read_line(&numPages); // Passing pointer to find the number of pages

  for (int i = 0; i < numPages; i++) {
    flag = 0;
    for (int j = 0; j < frames; j++) {
      // If the page is already in queue do not add as a page fault. Set flag to 1 to avoid incrementing faultCounter.
      if (queue[j] == referenceStream[i]) {
        flag++;
      }
    }
    if (flag) {
      faultCounter--;
    }
    faultCounter++;
    // To order queue for first n elements, where n = number of frames. in case there are empty slots, not taken by any value.
    if ((flag == 0) && (frames >= faultCounter)) {
      queue[i] = referenceStream[i];
    } else if (flag == 0) { // to order the queue when there are no empty slots available 
      queue[(faultCounter - 1) % frames] = referenceStream[i];
    }
  }
  printf("%d\n", faultCounter);
  free(referenceStream);
  free(queue);
  return EXIT_SUCCESS;
}
