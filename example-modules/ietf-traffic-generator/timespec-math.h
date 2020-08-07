#include <time.h>

void timespec_add(struct timespec* a, struct timespec* b, struct timespec* sum);
void timespec_sub(struct timespec* after, struct timespec* before, struct timespec* diff);
