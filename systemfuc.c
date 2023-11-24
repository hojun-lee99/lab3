//system_test.c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]){
    char *buf = strdup(argv[1]);

    if (argc != 2)
    {
        printf("Usage: wron use!\n");
        exit(1);
    }

    int ret;
    ret = system(buf);
    free(buf);
    printf("system함수 종료 :%d\n",WEXITSTATUS(ret));
}