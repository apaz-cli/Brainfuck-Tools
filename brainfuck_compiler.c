#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *readProgram(char *name) {
    FILE *file;
    char *buffer;
    unsigned long fileLen;
    /* Open file */
    file = fopen(name, "rb");
    if (!file) {
        fprintf(stderr, "Unable to open file %s", name);
        exit(1);
    }
    /* Get file length */
    fseek(file, 0, SEEK_END);
    fileLen = ftell(file);
    fseek(file, 0, SEEK_SET);

    /* Malloc exactly enough memory */
    buffer = (char *)malloc(fileLen + 1);
    if (!buffer) {
        fprintf(stderr, "Memory error, could not allocate to load program.");
        fclose(file);
        exit(2);
    }

    /* Copy buffer */
    fread(buffer, fileLen, 1, file);
    fclose(file);

    /* Null terminate at end */
    buffer[fileLen + 1] = '\0';

    /* Return a handle to the beginning of the text */
    return buffer;
}

void validate_program(char *program) {
    int matches = 0;
    char *programPtr = program;
    while (*programPtr != '\0') {
        switch (*programPtr) {
            case '[':
                ++matches;
                break;
            case ']':
                --matches;
                if (matches < 0) {
                    fprintf(stderr, "Unmatched closing brace.\n");
                    exit(1);
                }
                break;
            default:
                break;
        }
        ++programPtr;
    }
    if (matches > 0) {
        fprintf(stderr, "Unmatched starting brace.\n");
        exit(1);
    }
}

void writeHeader(FILE *f) {
    fputs("#include <stdio.h>\n", f);
    fputs("#include <stdlib.h>\n", f);
    fputs("int main() {\n", f);
    fputs("\tchar* tape = (char*)malloc(sizeof(char) * 30000);\n", f);
    fputs("\tchar* ptr = tape;\n\n", f);
}

void writeFooter(FILE *f) {
    fputs("\treturn 0;\n", f);
    fputs("}\n", f);
}

char *tabIn(unsigned int t, char *instruction) {
    size_t str_len = strlen(instruction);
    char *tabbed = malloc(sizeof(char) * t + str_len + 1);
    memset(tabbed, '\t', t);
    memcpy(tabbed + sizeof(char) * t, instruction, strlen(instruction) + 1);
    return tabbed;
}

void writeC(char *program, char *cPath) {
    FILE *f = fopen(cPath, "w");
    writeHeader(f);

    char *instruction;
    int tabs = 1;
    while (*program) {
        switch (*program) {
            case '+':
                instruction = tabIn(tabs, "++*ptr;\n");
                fputs(instruction, f);
                free(instruction);
                break;
            case '-':
                instruction = tabIn(tabs, "--*ptr;\n");
                fputs(instruction, f);
                free(instruction);
                break;
            case '>':
                instruction = tabIn(tabs, "++ptr;\n");
                fputs(instruction, f);
                free(instruction);
                break;
            case '<':
                instruction = tabIn(tabs, "--ptr;\n");
                fputs(instruction, f);
                free(instruction);
                break;
            case '[':
                instruction = tabIn(tabs, "while (*ptr) {\n");
                fputs(instruction, f);
                free(instruction);
                tabs++;
                break;
            case ']':
                instruction = tabIn(tabs, "}\n");
                fputs(instruction, f);
                free(instruction);
                tabs--;
                break;
            case '.':
                instruction = tabIn(tabs, "putchar(*ptr);\n");
                fputs(instruction, f);
                free(instruction);
                break;
            case ',':
                instruction = tabIn(tabs, "*ptr = getchar();\n");
                fputs(instruction, f);
                free(instruction);
                break;
            default:
                break;
        }
        ++program;
    }
    writeFooter(f);
    fclose(f);
}

void compileC(char *cPath, char *bfPath) {
    char *command[] = {"gcc ", cPath, " -o ", bfPath, " -O3"};
    size_t lens[] = {4, strlen(cPath), 4, strlen(bfPath), 4};

    size_t totalsize = lens[0] + lens[1] + lens[2] + lens[3] + lens[4] + 5;

    char *commandstr = (char *)malloc(sizeof(char) * totalsize);

    char *cmdloc = commandstr;
    for (size_t i = 0; i < 5; ++i) {
        memcpy(cmdloc, command[i], lens[i]);
        cmdloc += lens[i];
    }
    commandstr[totalsize] = '\0';
    
    system(commandstr);

    free(commandstr);
}

int main(int argc, char *argv[]) {
    char *bfPath = "a.out";
    char *cPath = "temp.c";

    /* Parse command line switches */
    if (argc < 2) {
        fprintf(stderr, "Please provide a program to compile.\n");
        exit(1);
    }
    if (argc == 3) bfPath = argv[2];

    char *program = readProgram(argv[1]);
    printf("%s", readProgram);

    /* Compile and delete temp C file */
    writeC(program, cPath);
    compileC(cPath, bfPath);
    remove(cPath);

    return 0;
}
