#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* getLine()
{
    char* line = malloc(sizeof(char));
    line[0] = '\0';
    size_t memSize = 1;
    char c, str[2];
    while ((c = getc(stdin)) != '\n')
    {
        memSize++;
        str[0] = c;
        str[1] = '\0';
        line = (char*)realloc(line, (memSize + 1) * sizeof(char));
        strcat(line, str);  
    }
    return line;
}

int main()
{
    char* line = getLine();
    printf("%s", line);
    return 0;
}