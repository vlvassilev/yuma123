
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>


int main(int argc, char **argv) {

    FILE *fp;

    char *buff;

    buff = (char *)malloc(1024);
    if (buff) {
        fp = fopen("/home/andy/ncx-modules/test.ncx", "r");
        if (fp) {
            printf("\n");
            while (fgets(buff, 1024, fp)) {
                printf("%s", buff);
            }
        }
        fclose(fp);
        free(buff);
    }
    return(0);
}
