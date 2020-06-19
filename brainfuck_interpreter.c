#include <stdio.h>
#include <stdlib.h>

char *readProgram(char *name) {
    FILE *file;
    char *buffer;
    unsigned long fileLen;
    // Open file
    file = fopen(name, "rb");
    if (!file) {
        fprintf(stderr, "Unable to open file %s", name);
        exit(1);
    }
    // Get file length
    fseek(file, 0, SEEK_END);
    fileLen = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Malloc exactly enough memory
    buffer = (char *)malloc(fileLen + 1);
    if (!buffer) {
        fprintf(stderr, "Memory error, could not allocate to load program.");
        fclose(file);
        exit(2);
    }

    // Copy buffer
    fread(buffer, fileLen, 1, file);
    fclose(file);

    // Null terminate at beginning and end

    buffer[fileLen + 1] = '\0';

    // Return a handle to the beginning of the text
    return buffer;
}

char *optimize_program(char *program) {
    // Scan for the number of non-ignored characters so that we can allocate the right amount of memory
    size_t used = 0;
    char *oldProgramPtr = program;
    while (*oldProgramPtr != '\0') {
        switch (*oldProgramPtr) {
            case '+':
            case '-':
            case '[':
            case ']':
            case '>':
            case '<':
            case '.':
            case ',':
                ++used;
                break;
            default:
                break;
        }
        ++oldProgramPtr;
    }

    // Make a new buffer of the correct size
    char *optimizedProgram = (char *)malloc(sizeof(char) * used + 1);
    if (!optimizedProgram) {
        fprintf(stderr, "Memory error, could not allocate to load program.");
        exit(2);
    }

    // Copy the relevant characters into the new buffer

    // Go back to the beginning of the old buffer
    oldProgramPtr = program;
    char *newProgramPtr = optimizedProgram;
    while (*oldProgramPtr != '\0') {
        switch (*oldProgramPtr) {
            case '+':
            case '-':
            case '[':
            case ']':
            case '>':
            case '<':
            case '.':
            case ',':
                *newProgramPtr = *oldProgramPtr;
                ++newProgramPtr;
                break;
            default:
                break;
        }
        ++oldProgramPtr;
    }
    optimizedProgram[used] = '\0';  // Null terminate the new buffer

    // Free the old buffer
    free(program);

    return optimizedProgram;
}

#define DEBUG 0
#if DEBUG != 0
#include <unistd.h>
#endif

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Please provide a program to execute\n");
        exit(0);
    }

    size_t tapePos = 0;
    size_t tapeCapacity = 5000;
    unsigned char *tape = (unsigned char *)malloc(tapeCapacity);
    char *programPtr = optimize_program(readProgram(argv[1]));

    if (DEBUG) printf("Your Program:\n");
    if (DEBUG) printf("%s\n", programPtr);
    if (DEBUG) printf("Starting Program\n");

    while (*programPtr != '\0') {
        if (DEBUG) {
            // sleep(1);
            printf("Character: %c\n", *programPtr);
            fflush(stdout);
        }
        switch (*programPtr) {
            case '+':
                ++tape[tapePos];
                break;
            case '-':
                --tape[tapePos];
                break;
            case '<':
                --tapePos;
                if (tapePos < 0) {
                    fprintf(stderr, "Program fell off the beginning of the tape.");
                    exit(1);
                }
                break;
            case '>':
                ++tapePos;
                // If necessary, allocate more memory for the tape.
                if (tapePos == tapeCapacity) {
                    tapeCapacity = (tapeCapacity * 1.2) + 500;
                    tape = (unsigned char *)realloc(tape, tapeCapacity);
                    if (!tape) {
                        fprintf(stderr, "Not enough memory to reallocate tape of size %lu", tapeCapacity);
                        exit(2);
                    }
                }
                break;
            case '[':
                if (tape[tapePos] == 0) {
                    int unmatched = 1;
                    while (unmatched == 0) {
                        ++programPtr;
                        if (*programPtr == '\0') {
                            fprintf(stderr, "Unmatched open brace at the end of your program.");
                        }
                        if (*programPtr == '[') ++unmatched;
                        if (*programPtr == ']') --unmatched;
                    }
                }
                // if the if is taken, ends on the closing ], gets advanced below.
                // otherwise, just moves on to the next character in the program.
                break;
            case ']':
                // Scan to the left for a matching [
                if (tape[tapePos] != 0) {
                    int unmatched = 1;
                    while (unmatched != 0) {
                        --programPtr;
                        if (*programPtr == '\0') {
                            fprintf(stderr, "Unmatched closing brace at the beginning of your program.");
                            exit(1);
                        }
                        if (*programPtr == ']') unmatched += 1;
                        if (*programPtr == '[') unmatched -= 1;
                    }
                }
                break;
            case '.':
                putchar(tape[tapePos]);
                break;
            case ',':
                tape[tapePos] = getchar();
                break;
            default:
                break;
        }
        programPtr++;
    }
}