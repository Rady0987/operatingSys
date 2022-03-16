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
  int *referenceBit, *useBit, hand = 0;

  scanf("%d ", &frames);
  referenceStream = read_line(&numPages); // Passing pointer to find the number of pages
  referenceBit = safeMalloc(frames * sizeof(int));
  useBit = safeMalloc(frames * sizeof(int));

  for(int i = 0; i < frames; i++) {
    referenceBit[i] = 0;
    useBit[i] = 0;
  }

  for (int i = 0; i < numPages; i++) {
    flag = 0;
    for (int j = 0; j < frames; j++) {
      // If we already have the page in memory set use bit to 1
      if (referenceStream[i] == referenceBit[j]) {
        useBit[j] = 1;
        flag = 1;
      }
    }
    if (!flag) {
      do {
        if (useBit[hand] == 0) {
          useBit[hand] = 1;
          flag = 1;
          faultCounter++;
          referenceBit[hand] = referenceStream[i];
        } else useBit[hand] = 0; // If useBit was 1, reduce it to 0
        hand++;
        if (hand == frames) hand = 0; // When we reach the end of the useBit array, reset iterator.
      } while (!flag);
    }
  }

  printf("%d\n", faultCounter);
  free(referenceStream);
  free(useBit);
  free(referenceBit);
  return EXIT_SUCCESS;
}
