#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define STR_SIZE 256

struct programmDelay
{
    size_t delay;
    char name[STR_SIZE];
};
int cmp(const void *a, const void *b);

int main(int argc, char** argv, char** envp)
{

    FILE *input = NULL;
    if (argc == 1)
        input = fopen("input.txt", "rt");
    if (argc == 2)
        input = fopen("input.txt", "rt");
    if (input == NULL)
    {
        perror("Incorrect filename");
        exit(1);
    }

    struct programmDelay curr;
    struct programmDelay *arr = (struct programmDelay*)malloc(sizeof(struct programmDelay));
    size_t amount = 0;

    while ((fscanf(input, "%zd", &curr.delay) > 0) && (fgets(curr.name, STR_SIZE, input)))
    {
        printf("%zd\n", curr.delay);
        amount++;
        arr = (struct programmDelay*)realloc(arr, amount * sizeof(struct programmDelay));
        arr[amount - 1].delay = curr.delay;
        strcpy(arr[amount - 1].name, curr.name);
    }
    fclose(input);

    qsort(arr, amount, sizeof(struct programmDelay), cmp);

    size_t prev = 0;
    for (size_t i = 0; i < amount; i++)
    {
        sleep((arr[i].delay - prev));
        system(arr[i].name);
        prev = arr[i].delay;
    }

    free(arr);

    return 0;
}

int cmp(const void* a, const void* b)
{
    return ((struct programmDelay*)a)->delay - ((struct programmDelay*)b)->delay;
}
