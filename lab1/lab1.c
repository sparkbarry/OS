#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main(){
srand(time(NULL));
printf("Hello world! This is Introduction to Operating Systems, CS 6233, Fall 2026!");
printf("\n");
printf("Chengwen Yang");
printf("%d", rand() % 100);
printf("\n");

return 0;
}
