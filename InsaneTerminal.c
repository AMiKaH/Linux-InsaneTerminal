/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Assignment1 - This program will disable the normal terminal functions and set its mode to raw. The
--                            functions defined in this program will replace the normal controls of the terminal.
--
-- PROGRAM: Assignment1
--
-- FUNCTIONS:
-- int main();
-- void inputFunction(int pipeToOutput, int pipeToTranslate);
-- void outputFunction(int pipeFromInput, int pipeFromTranslate);
-- void translateFunction(int pipeFromInput, int pipeToOutput);
--
--
-- DATE: January 22, 2020
--
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- NOTES:
-- This is an application that will disable all normal terminal behaviour then start 2
-- new processes for a total of 3. The main process is the Input process, from which the
-- Output and Translate processes are forked. The processes will take user input and use
-- the program-defined controls and translate all occurences of 'a' to 'z'. Pipes are used
-- to pass data between the processes.
----------------------------------------------------------------------------------------------------------------------*/

#include "InsaneTerminal.h"

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: main
--
-- DATE: January 22, 2020
--
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: int main()
--
-- RETURNS: int.
--
-- NOTES:
-- This is the start of the program. This function is called first whenthe program starts excuting. It creates all the
-- pipes necesssary for communication between the processes as well as starting 2 extra processes for Output and Translate
-- functionalty
----------------------------------------------------------------------------------------------------------------------*/

int main() {
    //The 3 pipes
    int inputToOutput[2];
    int inputToTranslate[2];
    int translateToOutput[2];
    
    inputPID = malloc(sizeof(int));
    outputPID = malloc(sizeof(int));
    xlatePID = malloc(sizeof(int));

    system("stty raw igncr -echo");

    if (pipe(inputToOutput) < 0)
    {
      perror("pipe call");
      exit(1);
    }
    
    if (pipe(inputToTranslate) < 0)
    {
      perror("pipe call");
      exit(1);
    }
    
    if (pipe(translateToOutput) < 0)
    {
      perror("pipe call");
      exit(1);
    }
    
    /*
     Output
     */
    
    if ((*inputPID = fork()) == 0){
        *outputPID = getpid();
        
        close(inputToOutput[PIPEW]);
        close(translateToOutput[PIPEW]);
        close(inputToTranslate[PIPER]);
        close(inputToTranslate[PIPEW]);
                
        outputFunction(inputToOutput[PIPER], translateToOutput[PIPER]);
        return 0;
    }
    
    *inputPID = fork();
    
    /*
     Translate
     */
    if (*inputPID == 0){
        *xlatePID = getpid();

        close(inputToOutput[PIPER]);
        close(inputToOutput[PIPEW]);
        close(translateToOutput[PIPER]);
        close(inputToTranslate[PIPEW]);
                
        translateFunction(inputToTranslate[PIPER], translateToOutput[PIPEW]);
        return 0;
    }
    
    /*
     Parent
     */
    if (inputPID > 0){
        *inputPID = getpid();

        close(inputToOutput[PIPER]);
        close(inputToTranslate[PIPER]);
        close(translateToOutput[PIPER]);
        close(translateToOutput[PIPEW]);

        inputFunction(inputToOutput[PIPEW], inputToTranslate[PIPEW]);
        
        system("stty -raw -igncr echo");
        return 0;
    }
    
    return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: inputFunction
--
-- DATE: January 22, 2020
--
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: void inputFunction(int inputToOutput, int translateToOutput)
--            - int inputToOutput: the file descriptor for the writing end of the pipe that writes from input process to the
--              output process
--            - int translateToOutput: the file descriptor for the writig end of the pipe that connects the input process
--              to the translate process
--
-- RETURNS: void.
--
-- NOTES:
-- This function takes in user input character by character and evaluates each of the characters. If a user enters 'T' the
-- program will gracefully terminate by translating the buffer then exiting. If the user enters 'E' then the buffer is passed
-- to the Translate process through the pipe for translation. If the user presses the combination of 'control + k' then the
-- program will abruptly exit. Otherwise, and for all other inputs, the process will pass that input through the pipe to the
-- Output process to display them onto the screen.
----------------------------------------------------------------------------------------------------------------------*/

void inputFunction(int inputToOutput, int translateToOutput){
    char c = ' ';
    char translator[BUFFERSIZE];
    int index = 0;
    
    
    while (c != 'T' && (c = getchar()) != EOF){

        write(inputToOutput, &c, BUFFERSIZE);
        translator[index] = c;
        translator[index + 1] = '\0';
        index++;
        
        switch (c){
            case 'T':
                
            case('E'):
                if (write(translateToOutput, translator, BUFFERSIZE) > 0){
                    index = 0;
                    translator[index] = '\0';
                }
                break;
            case (CTLK):
                kill(*outputPID, SIGKILL);
                kill(*xlatePID, SIGKILL);
                kill(*inputPID, SIGKILL);
                break;
                
        }
    }
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: outputFunction
--
-- DATE: January 22, 2020
--
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: void inputFunction(int inputToOutput, int translateToOutput)
--            - int inputToOutput: the file descriptor for the reading end of the pipe that reads from input process to the
--              output process
--            - int translateToOutput: the file descriptor for the reading end of the pipe that connects the translate process
--              to the output process
--
-- RETURNS: void.
--
-- NOTES:
-- This functions main purpose is to echo the characters that the user enetered to the screen. The process echos characters
-- that are passed to it through either the input pipe (character by character) and evaluates those characters. Or it echos
-- it outputs an entire buffer as it is passed by the translatorFunction and that buffer contains the translated string as
-- defined by the program and that function below.
--
-- If the character passed is an 'E' then the process starts reading from the pipe coming from the translation process.
-- If it is a 'T' (for graceful termination), then it will disable the loop and start reading from the translation pipe,
-- otherwise it will echo the characters as it receives them.
----------------------------------------------------------------------------------------------------------------------*/

void outputFunction(int inputToOutput, int translateToOutput){
    char inBuff[BUFFERSIZE];
    char translated[BUFFERSIZE];
    int cont = 1;
    
    while (cont && (read(inputToOutput, inBuff, BUFFERSIZE)) >= 1){
        
        switch (inBuff[0]) {
            case 'T':
                cont = -1;
                
            case 'E':
                printf("%s\n\r", inBuff);
                if (read(translateToOutput, translated, BUFFERSIZE) >= 1){
                    printf("%s\n\r", translated);
                }
                break;
                
            default:
                
                printf("%s", inBuff);
                fflush(stdout);
                
        }
    }
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: translateFunction
--
-- DATE: January 22, 2020
--
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: void inputFunction(int inputToOutput, int translateToOutput)
--            - int inputToTranslate: the file descriptor for the reading end of the pipe that reads from input process to the
--              translate process
--            - int translateToOutput: the file descriptor for the writing end of the pipe that connects the translate process
--              to the output process
--
-- RETURNS: void.
--
-- NOTES:
-- This function takaes in buffers from the input function when the user enters 'E'. The function is constantly waiting for
-- the buffer to come in from the input process. Once the buffer is read, the function will evaluate it character by character
-- and translating it as per the program's requirements:
--      - All 'a' to be converted to 'z'
--      - If an 'X' is encountered, delete the 1 character that preceds it and start writing in its place
--      - If a 'K' is encountered, delete all the characters that precede it and start writing from the beginning of the buffer
--      - For all other characters, copy them as is
-- After the translation of the entire buffer is completed, the processes writes the new buffer to the output process for it to
-- display it onto the screen.
----------------------------------------------------------------------------------------------------------------------*/

void translateFunction(int inputToTranslate, int translateToOutput){
    char xlBuff[BUFFERSIZE];
    char translator[BUFFERSIZE];
    size_t j;
    size_t i;
    
    while (read(inputToTranslate, xlBuff, BUFFERSIZE) >= 1){
        for (i = 0, j = 0; i < strlen(xlBuff); i++){
            switch (xlBuff[i]){
                case 'a':
                    translator[j] = 'z';
                    j++;
                    break;
                    
                case 'X':
                    if (j > 0){
                        j -= 1;
                    } else {
                        j = 0;
                    }
                    break;
                    
                case 'K':
                    j = 0;
                    break;
                    
                default:
                    translator[j] = xlBuff[i];
                    j++;
            }
        }
        
        translator[j - 1] = '\0';
        write(translateToOutput, translator, j);
    }
}
