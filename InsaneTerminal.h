
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>


#define BUFFERSIZE 1024
//pipe read end
#define PIPER      0
//pipe write end
#define PIPEW      1
#define CTLK       11

//Pointers to the PIDS of the 3 proces
pid_t *inputPID;
pid_t *outputPID;
pid_t *xlatePID;

int main();
void inputFunction(int pipeToOutput, int pipeToTranslate);
void outputFunction(int pipeFromInput, int pipeFromTranslate);
void translateFunction(int pipeFromInput, int pipeToOutput);



