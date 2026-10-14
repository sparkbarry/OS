#include <stdio.h>
#include <unistd.h>

int main(){
    pid_t fork1 = fork();
    // fork1 == 0 -> p1
    // fork1 > 0  -> parent
    if (fork1 == 0){
        printf("The first subprocess, return 1\n");
        return 1;
    }

    pid_t fork2 = fork();
    // fork2 == 0 -> p2
    // fork2 > 0  -> parent
    if (fork2 == 0){
        // p2
        pid_t fork3 = fork();
        // fork3 == 0 -> p3
        // fork3 > 0  -> p2
        if (fork3 == 0){
            printf("The third subprocess, return 3\n");
            return 3;
        } else {
            printf("The second subprocess, return 2\n");
            return 2;
        }
    }

    pid_t fork4 = fork();
    if (fork4 == 0) {
        printf("The forth subprocess, return 4\n");
        return 4;
    }

    printf("Parent Process Return 0\n");

    return 0;
}

