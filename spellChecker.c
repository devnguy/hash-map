#include "hashMap.h"
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MIN3(a, b, c) ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)))

/**
 * Allocates a string for the next word in the file and returns it. This string
 * is null terminated. Returns NULL after reaching the end of the file.
 * @param file
 * @return Allocated string or NULL.
 */
char *nextWord(FILE *file)
{
    int maxLength = 16;
    int length = 0;
    char *word = malloc(sizeof(char) * maxLength);
    while (1)
    {
        char c = fgetc(file);
        if ((c >= '0' && c <= '9') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            c == '\'')
        {
            if (length + 1 >= maxLength)
            {
                maxLength *= 2;
                word = realloc(word, maxLength);
            }
            word[length] = c;
            length++;
        }
        else if (length > 0 || c == EOF)
        {
            break;
        }
    }
    if (length == 0)
    {
        free(word);
        return NULL;
    }
    word[length] = '\0';
    return word;
}

/**
 * Loads the contents of the file into the hash map.
 * @param file
 * @param map
 */
void loadDictionary(FILE *file, HashMap *map)
{
    char *word = nextWord(file);

    while (word != NULL)
    {
        hashMapPut(map, word, -1);
        free(word);
        word = nextWord(file);
    }
}

/**
 * Returns true if ch is a non alpha character.
 */
int isNonAlpha(char ch)
{
    if (ch < 65 || (ch >= 91 && ch <= 96) || ch > 123)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * Iterates through each char of a string and returns true if every char 
 * is a letter.
 */
int checkString(char *stringToCheck)
{
    int length = strlen(stringToCheck);

    if (length == 0)
    {
        return 0;
    }

    for (int i = 0; i < length; i++)
    {
        if (isNonAlpha(stringToCheck[i]))
        {
            return 0;
        }
    }
    return 1;
}

/**
 * Converts every char in the given string to lowercase.
 */
void toLowerCase(char *stringToConvert)
{
    for (int i = 0, length = strlen(stringToConvert); i < length; i++)
    {
        stringToConvert[i] = tolower(stringToConvert[i]);
    }
}

/**
 * Input validation function that continues to prompt the user for a string 
 * that meets the program's specifications: One word, no nonalpha chars
 * Stores in the validated string in the provided argument.
 */
void getString(char *input)
{
    int isInvalid = 0;

    do
    {
        if (isInvalid)
        {
            printf("Invalid input\n");
            printf("Enter one word with upper/lowercase letters only: ");
            isInvalid = 0;
        }
        scanf("%[^\n]%*c", input);
        fflush(stdin); // Clears the input buffer if input is just "\n"

        if (!checkString(input))
        {
            isInvalid = 1;
        }
    } while (isInvalid);

    printf("\n");
    // Convert validated string to lowercase
    toLowerCase(input);
}

/**
 * Compares two strings and calculates their LeveinShtein distance.
 */
int levenshtein(char *s1, char *s2)
{
    unsigned int s1len, s2len, x, y, lastdiag, olddiag;
    s1len = strlen(s1);
    s2len = strlen(s2);
    unsigned int column[s1len + 1];
    for (y = 1; y <= s1len; y++)
        column[y] = y;
    for (x = 1; x <= s2len; x++)
    {
        column[0] = x;
        for (y = 1, lastdiag = x - 1; y <= s1len; y++)
        {
            olddiag = column[y];
            column[y] = MIN3(column[y] + 1, column[y - 1] + 1, lastdiag + (s1[y - 1] == s2[x - 1] ? 0 : 1));
            lastdiag = olddiag;
        }
    }
    return (column[s1len]);
}

/**
 * Compares the given string to each word in the dictionary, calculating the Levenshtein 
 * distance. Assigns that value for each key in the table.
 */
void assignDistance(HashMap *map, char *word)
{
    int currentSize = 0;
    char **related[5]; // array keeping track of 5 words with the lowest distance
    HashLink *current;

    for (int i = 0, mapSize = map->capacity; i < mapSize; i++)
    {
        current = map->table[i];
        while (current != NULL)
        {
            current->value = levenshtein(word, current->key);

            // by default, add to array if there is room
        }
    }
}

struct RelatedWords
{
    char **words;
    int capacity;
    int size;
    int max;
};

/**
 * Creates a RelatedWords struct, allocating memory for a the array given the
 * number of words the array should hold.
 * @param capacity The number of words.
 * @return The allocated words struct.
 */
struct RelatedWords *relatedWordsNew(int capacity)
{
    struct RelatedWords *related = malloc(sizeof(struct RelatedWords));
    related->words = malloc(sizeof(char *) * capacity);
    related->capacity = capacity;
    related->size = 0;
    related->max = 0;
    return related;
}

void relatedWordsDelete(struct RelatedWords *related)
{
    free(related->words);
    related->words = NULL;
    free(related);
}

void relatedWordsPrint(struct RelatedWords *related)
{
    for (int i = 0, size = related->size; i < size; i++)
    {
    }
}

/**
 * Checks the spelling of the word provded by the user. If the word is spelled incorrectly,
 * print the 5 closest words as determined by a metric like the Levenshtein distance.
 * Otherwise, indicate that the provded word is spelled correctly. Use dictionary.txt to
 * create the dictionary.
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char **argv)
{
    // FIXME: implement
    HashMap *map = hashMapNew(1000);

    struct RelatedWords *suggestions =

        FILE *file = fopen("dictionary.txt", "r");
    clock_t timer = clock();
    loadDictionary(file, map);
    timer = clock() - timer;
    printf("Dictionary loaded in %f seconds\n", (float)timer / (float)CLOCKS_PER_SEC);
    fclose(file);

    char inputBuffer[256];
    int quit = 0;

    while (!quit)
    {
        printf("Enter a word or \"quit\" to quit: ");
        // scanf("%s", inputBuffer);
        getString(inputBuffer); // replaced provided scanf with input validation function

        // Implement the spell checker code here..

        if (strcmp(inputBuffer, "quit") == 0)
        {
            quit = 1;
        }
        else if (hashMapContainsKey(map, inputBuffer))
        {
            printf("The inputted word, \"%s\" is spelled correctly.\n\n", inputBuffer);
        }
        // The word is spelled incorrectly. Compute Levenshtein distance and print suggestions.
        else
        {
            // compute levenshtein distance for the value for each key in the table

            // generate an array of 5 words that are closest matches to inputBuffer based on lowest L distance
            printf("The inputted word \"%s\" is spelled incorrectly.\n", inputBuffer);
            printf("Did you mean ...\n");
            printf("    word1\n");
            printf("    word1\n");
            printf("    word1\n");
            printf("    word1\n");
            printf("    word5\n\n");
        }
    }

    hashMapDelete(map);
    return 0;
}
