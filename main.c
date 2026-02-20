#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#define STATEMENT_TYPE_SYSCALL 0

#define EXPRESSION_TYPE_UINT64 0

struct expression {
    unsigned char type;
    union {
        uint64_t uint64_value;
    } value;
};

struct statement {
    unsigned char type;
    struct expression* args;
    int num_args;
};

uint64_t parse_uint64(const char* str, int length) {
    uint64_t result = 0;
    for(int i = 0; i < length; i++) {
        if(str[i] >= '0' && str[i] <= '9') {
            result = result * 10 + (str[i] - '0');
        } else {
            break; // Stop parsing on non-digit
        }
    }
    return result;
}

int visualise_statement(struct statement* stmt) {
    if(stmt->type == STATEMENT_TYPE_SYSCALL) {
        printf("System call with arguments:\n");
        for(int i = 0; i < stmt->num_args; i++) {
            if(stmt->args[i].type == EXPRESSION_TYPE_UINT64) {
                printf("Arg %d: %lu\n", i + 1, stmt->args[i].value.uint64_value);
            }
        }
    }
    return 0;
}

int tokenizer(char* str) {
    if(*str == '\0') {
        return 0;
    }

    if(*str == ' ') {
        return 1;
    }

    if(*str == ';') {
        return 1;
    }

    int i = 1;
    while(str[i] != ' ' && str[i] != ';' && str[i] != '\0') {
        i++;
    }
    return i;
}

int main() {
    FILE *file = fopen("main.gougoule", "r");

    char* fileContent = malloc(sizeof(char));
    char buffer[1024];

    while (fgets(buffer, sizeof(buffer), file)) {
        fileContent = realloc(fileContent, strlen(fileContent) + strlen(buffer) + 1);
        strcat(fileContent, buffer);
    }

    unsigned char ELF_headers[64] = {
        0x7F, 'E', 'L', 'F', // Magic : ELF
        0x02, // 64-bit
        0x01, // Little-endian
        0x01, // Version
        0x00, // System V ABI
        // Padding
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x02, 0x00, // Type: Executable
        0x3E, 0x00, // Machine: x86-64
        0x01, 0x00, 0x00, 0x00, // Version
        0x78, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, // Entry point
        0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Program header table offset
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Section header table offset
        0x00, 0x00, 0x00, 0x00, // Flags
        0x40, 0x00, // ELF header size
        0x38, 0x00, // Program header entry size
        0x01, 0x00, // Number of program header entries
        0x00, 0x00, // Section header entry size
        0x00, 0x00, // Number of section header entries
        0x00, 0x00 // Section header string table index
    };

    unsigned char programHeader[56] = {
        0x01, 0x00, 0x00, 0x00, // Type: Load
        0x05, 0x00, 0x00, 0x00, // Flags: Read + Execute
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Offset
        0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, // Virtual address
        0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, // Physical address
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // File size
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Memory size
        0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // Alignment
    };

    /*
    // print the syscall
    printf("System call number: %d\n", syscall.syscall_number);
    printf("Argument 1: %d\n", syscall.arg1);

    unsigned char* code = malloc(sizeof(unsigned char) * (5 + 5 + 2));
    code[0] = 0xb8; // mov rax, syscall_number
    code[1] = syscall.syscall_number; // syscall number
    code[2] = 0x00; // padding
    code[3] = 0x00; // padding
    code[4] = 0x00; // padding
    code[5] = 0xbf; // mov rdi, arg1
    code[6] = syscall.arg1; // arg1
    code[7] = 0x00; // padding
    code[8] = 0x00; // padding
    code[9] = 0x00; // padding
    code[10] = 0x0f; // syscall
    code[11] = 0x05; // syscall

    char* finalCode = malloc(sizeof(char) * (64 + 56 + 12));
    memcpy(finalCode, ELF_headers, 64);
    memcpy(finalCode + 64, programHeader, 56);
    memcpy(finalCode + 64 + 56, code, 12);
    finalCode[64 + 32] = 12; // File size in program header
    finalCode[64 + 40] = 12; // Memory size in program header

    FILE* outputFile = fopen("output.elf", "wb");
    fwrite(finalCode, sizeof(char), 64 + 56 + 12, outputFile);
    fclose(outputFile);
    */

    struct statement** statements = NULL;
    int num_statements = 0;
    struct statement* c_statement = NULL;
    int argsI = 0;

    char* currentToken = fileContent;
    int token_length = tokenizer(currentToken);
    while(token_length > 0) {
        if(*currentToken != ' ' && *currentToken != ';') {
            if(c_statement == NULL) {
                c_statement = malloc(sizeof(struct statement));
                if(strncmp(currentToken, "sys", token_length) == 0) {
                    c_statement->type = STATEMENT_TYPE_SYSCALL;
                }
            } else {
                c_statement->args = realloc(c_statement->args, sizeof(struct expression) * (argsI + 1));
                c_statement->num_args = argsI + 1;

                c_statement->args[argsI].type = EXPRESSION_TYPE_UINT64;
                c_statement->args[argsI].value.uint64_value = parse_uint64(currentToken, token_length);
                argsI++;
            }
        } else if (currentToken[0] == ';') {
            statements = realloc(statements, sizeof(struct statement*) * (num_statements + 1));
            statements[num_statements] = c_statement;
            num_statements++;

            c_statement = NULL;
            argsI = 0;
        }
        currentToken += token_length;
        token_length = tokenizer(currentToken);
    }

    for(int i = 0; i < num_statements; i++) {
        visualise_statement(statements[i]);
    }

}
