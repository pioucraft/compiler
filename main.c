#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    FILE *file = fopen("main.gougoule", "r");

    char* fileContent = malloc(sizeof(char));
    char buffer[1024];

    while (fgets(buffer, sizeof(buffer), file)) {
        fileContent = realloc(fileContent, strlen(fileContent) + strlen(buffer) + 1);
        strcat(fileContent, buffer);
    }
    printf("%s", fileContent);
}
