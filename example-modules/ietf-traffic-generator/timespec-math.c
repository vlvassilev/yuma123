#include <time.h>

void timespec_add(struct timespec* a, struct timespec* b, struct timespec* sum)
{
    if ((a->tv_nsec + b->tv_nsec) >= 1000000000) {
        sum->tv_sec = a->tv_sec + b->tv_sec + 1;
        sum->tv_nsec = a->tv_nsec + b->tv_nsec - 1000000000;
    } else {
        sum->tv_sec = a->tv_sec + b->tv_sec;
        sum->tv_nsec = a->tv_nsec + b->tv_nsec;
    }
}

void timespec_sub(struct timespec* after, struct timespec* before, struct timespec* diff)
{
    if (after->tv_nsec < before->tv_nsec) {
        diff->tv_sec = after->tv_sec - before->tv_sec - 1;
        diff->tv_nsec = after->tv_nsec + 1000000000 - before->tv_nsec;
    } else {
        diff->tv_sec = after->tv_sec - before->tv_sec;
        diff->tv_nsec = after->tv_nsec - before->tv_nsec;
    }

    return;
}
