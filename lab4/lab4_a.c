#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char* argv[]){
    if (argc <= 2){
        printf("Missing Arguments\n");
    }
    int n = atoi(argv[1]);
    double d = atof(argv[2]);

    pid_t p = fork();
    if (p == 0){
        for(int k = 0; k < n; k = k + 1){
            printf("%g ", k*d);
        }
        return 0;
    }
    else if (p > 0){
        waitpid(p, NULL, 0);
        for (int k = n; k < n + 3; k = k + 1){
            printf("%g ", k*d);
        }
    }
    else printf("Fork failed!");
    printf("\n");
}