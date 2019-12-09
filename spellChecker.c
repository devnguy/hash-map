#include "hashMap.h"
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Required by Levenshtein calculation.
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
 * that meets the program's specifications: One word, no nonalpha chars.
 * Stores in the validated string in the provided argument.
 */
void getString(char *input)
{
    int isInvalid = 0;

    do
    {
        if (isInvalid)
        {
            printf("\nInvalid input\n\n");
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
    // Convert validated string to lowercase.
    toLowerCase(input);
}

/**
 * Compares two strings and calculates their Leveinshtein distance.
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
 * Compares the given string to each word in the dictionary. If an exact match is found, the 
 * function returns true. If the word is not a match, calculates the Levenshtein distance 
 * and assigns that value for the key in the table. If the entire table is traversed and no 
 * match is found, returns false.
 */
int findMatch(HashMap *map, char *word)
{
    HashLink *current;

    for (int i = 0, cap = map->capacity; i < cap; i++)
    {
        current = map->table[i];
        while (current != NULL)
        {
            // Exact match is found, return true.
            if (strcmp((current->key), word) == 0)
            {
                return 1;
            }
            // Assign Lev distance and move on to next word.
            else
            {
                current->value = levenshtein(word, current->key);
                current = current->next;
            }
        }
    }
    return 0;
}

/**
 * Traverses the given map, maintaining an array of 5 words with the 
 * lowest Levenshtein distance.
 * @param map to traverse
 * @param relatedWords an array to store the words
 */
void findRelatedWords(HashMap *map, char **relatedWords, int size)
{
    HashLink *current;
    int maxIndex = size - 1;
    int indexToAdd = 0;
    int currentMax = 1000;

    for (int i = 0, cap = map->capacity; i < cap; i++)
    {
        current = map->table[i];
        while (current != NULL)
        {
            // Fill the array with words because the array is empty.
            if (i < size)
            {
                relatedWords[indexToAdd] = current->key;
                if (current->value <= currentMax)
                {
                    currentMax = current->value;
                }
                indexToAdd++;
            }
            // Only add the word if the value <= currentMax.
            else
            {
                if (current->value <= currentMax)
                {
                    relatedWords[indexToAdd] = current->key;
                    currentMax = current->value;
                    indexToAdd++;
                }
            }

            // Reset the index to start over once the end of the array is reached.
            if (indexToAdd > maxIndex)
            {
                indexToAdd = 0;
            }
            current = current->next;
        }
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
    HashMap *map = hashMapNew(1000);
    int numberOfRelatedWords = 5;
    char **relatedWords = malloc(sizeof(char *) * numberOfRelatedWords);

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
        getString(inputBuffer); // replaced provided scanf with input validation function

        // Case 1: User types "quit" to exit program.
        if (strcmp(inputBuffer, "quit") == 0)
        {
            quit = 1;
        }
        // Case 2: Input matches word found in dictionary.
        else if (findMatch(map, inputBuffer))
        {
            printf("The inputted word, \"%s\" is spelled correctly.\n\n", inputBuffer);
        }
        // Case 3: Input is spelled incorrectly. Find and print related words.
        else
        {
            findRelatedWords(map, relatedWords, numberOfRelatedWords);
            printf("The inputted word \"%s\" is spelled incorrectly.\n", inputBuffer);
            printf("Did you mean ...\n");
            for (int i = 0; i < numberOfRelatedWords; i++)
            {
                printf("    %s\n", relatedWords[i]);
            }
            printf("\n");
        }
    }

    free(relatedWords);
    hashMapDelete(map);
    return 0;
}
