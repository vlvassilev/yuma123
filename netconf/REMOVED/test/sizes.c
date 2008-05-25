
#include <stdio.h>


int main(int argc, char **argv) {

    char  c, *pc;
    unsigned char u, *pu;


    printf("\nsizeof(char) = %d", sizeof(char));
    printf("\nsizeof(short) = %d", sizeof(short));
    printf("\nsizeof(int) = %d", sizeof(int));
    printf("\nsizeof(long) = %d", sizeof(long));
    printf("\nsizeof(long long) = %d", sizeof(long long));
    printf("\nsizeof(float) = %d", sizeof(float));
    printf("\nsizeof(double) = %d", sizeof(double));
    printf("\n");

    c = 'g';
    u = (unsigned char)c;
    c = (char)u;

    pc = "foo";
    pu = (unsigned char *)pc;
    pc = (char *)pu;

    return(0);
}
