#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[]){
if (argc<2) return -1;
char* filename = argv[1];
srand(time(NULL));

printf("%d\n", getppid());

// using complete residue system to narrow the rand number between 0 and 9
sleep(rand() % 10);

int fileDescriptor = open(filename, O_RDONLY);
if (fileDescriptor == -1){
    return -1;
}
char buffer[1];

// the return value of read() func can be 
// -1 for error, 0 for EOF and numbers bigger than 0 for size it read
// so the judge flag is whether the ret value is bigger than 0
while(read(fileDescriptor, buffer, 1)>0){
    printf("%c", buffer[0]);
}
close(fileDescriptor);

return 0;
}
