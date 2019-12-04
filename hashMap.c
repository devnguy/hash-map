/*
 * CS 261 Data Structures
 * Assignment 5
 * Name: Huy Nguyen
 * Date: 12/3/19
 */

#include "hashMap.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

int hashFunction1(const char *key)
{
    int r = 0;
    for (int i = 0; key[i] != '\0'; i++)
    {
        r += key[i];
    }
    return r;
}

int hashFunction2(const char *key)
{
    int r = 0;
    for (int i = 0; key[i] != '\0'; i++)
    {
        r += (i + 1) * key[i];
    }
    return r;
}

/**
 * Creates a new hash table link with a copy of the key string.
 * @param key Key string to copy in the link.
 * @param value Value to set in the link.
 * @param next Pointer to set as the link's next.
 * @return Hash table link allocated on the heap.
 */
HashLink *hashLinkNew(const char *key, int value, HashLink *next)
{
    HashLink *link = malloc(sizeof(HashLink));
    link->key = malloc(sizeof(char) * (strlen(key) + 1));
    strcpy(link->key, key);
    link->value = value;
    link->next = next;
    return link;
}

/**
 * Free the allocated memory for a hash table link created with hashLinkNew.
 * @param link
 */
static void hashLinkDelete(HashLink *link)
{
    free(link->key);
    free(link);
}

/**
 * Initializes a hash table map, allocating memory for a link pointer table with
 * the given number of buckets.
 * @param map
 * @param capacity The number of table buckets.
 */
void hashMapInit(HashMap *map, int capacity)
{
    map->capacity = capacity;
    map->size = 0;
    map->table = malloc(sizeof(HashLink *) * capacity);
    for (int i = 0; i < capacity; i++)
    {
        map->table[i] = NULL;
    }
}

/**
 * Removes all links in the map and frees all allocated memory. You can use
 * hashLinkDelete to free the links.
 * @param map
 */
void hashMapCleanUp(HashMap *map)
{
    // FIXME: implement
    // iterate through the array
    // current = array[i]
    // while current != null
    // nextLink = current->next
    // delete current
    // current = nextLink
    HashLink *current;
    HashLink *nextLink;

    // free all links
    for (int i = 0, cap = map->capacity; i < cap; i++)
    {
        current = map->table[i];
        while (current != NULL)
        {
            nextLink = current->next;
            hashLinkDelete(current);
            current = nextLink;
        }
    }
    // free the table
    free(map->table);
    // map->table = NULL;
}

/**
 * Creates a hash table map, allocating memory for a link pointer table with
 * the given number of buckets.
 * @param capacity The number of buckets.
 * @return The allocated map.
 */
HashMap *hashMapNew(int capacity)
{
    HashMap *map = malloc(sizeof(HashMap));
    hashMapInit(map, capacity);
    return map;
}

/**
 * Removes all links in the map and frees all allocated memory, including the
 * map itself.
 * @param map
 */
void hashMapDelete(HashMap *map)
{
    hashMapCleanUp(map);
    free(map);
}

/**
 * Returns a pointer to the value of the link with the given key  and skip traversing as well. Returns NULL
 * if no link with that key is in the table.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket. Also make sure to search the entire list.
 * 
 * @param map
 * @param key
 * @return Link value or NULL if no matching link.
 */
int *hashMapGet(HashMap *map, const char *key)
{
    assert(map != 0);
    assert(key != 0);

    int hashIndex = HASH_FUNCTION(key) % hashMapCapacity(map);
    struct HashLink *current = map->table[hashIndex];

    while (current != NULL)
    {
        if (strcmp(current->key, key) == 0)
        {
            return &current->value;
        }
        else
        {
            current = current->next;
        }
    }
    return NULL;
}

/**
 * Resizes the hash table to have a number of buckets equal to the given 
 * capacity (double of the old capacity). After allocating the new table, 
 * all of the links need to rehashed into it because the capacity has changed.
 * 
 * Remember to free the old table and any old links if you use hashMapPut to
 * rehash them.
 * 
 * @param map
 * @param capacity The new number of buckets.
 */
void resizeTable(HashMap *map, int capacity)
{
    HashMap *newMap = hashMapNew(capacity);
    HashLink *current;

    // hash each link in oldMap and put in newMap
    for (int i = 0, cap = map->capacity; i < cap; i++)
    {
        current = map->table[i];
        while (current != NULL)
        {
            hashMapPut(newMap, current->key, current->value);
            current = current->next;
        }
    }

    hashMapCleanUp(map);        // should free the old table
    map->table = newMap->table; // set table to newTable
    map->capacity = newMap->capacity;
    map->size = newMap->size;
    free(newMap); // delete the newly allocated table
}

/**
 * Updates the given key-value pair in the hash table. If a link with the given
 * key already exists, this will just update the value and skip traversing. Otherwise, it will
 * create a new link with the given key and value and add it to the table
 * bucket's linked list. You can use hashLinkNew to create the link.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket.
 * 
 * @param map
 * @param key
 * @param value
 */
void hashMapPut(HashMap *map, const char *key, int value)
{
    assert(map != 0);
    assert(key != 0);

    int hashIndex = HASH_FUNCTION(key) % hashMapCapacity(map);
    struct HashLink *current = map->table[hashIndex];

    if (hashMapContainsKey(map, key))
    {
        // find the key, replace the value at that link
        while (strcmp(current->key, key) != 0)
        {
            current = current->next;
        }
        current->value = value;
    }
    else
    {
        // key does not exist so allocate a new link
        HashLink *newLink = hashLinkNew(key, value, NULL);

        // case 1: this is the first link being added to the list so set head to newLink
        if (current == NULL)
        {
            map->table[hashIndex] = newLink;
        }
        // case 2: list exists so traverse to the end
        else
        {
            while (current->next != NULL)
            {
                current = current->next;
            }
            current->next = newLink;
        }
        map->size++;
        if (hashMapTableLoad(map) > MAX_TABLE_LOAD)
        {
            resizeTable(map, map->capacity * 2);
        }
    }
}

/**
 * Removes and frees the link with the given key from the table. If no such link
 * exists, this does nothing. Remember to search the entire linked list at the
 * bucket. You can use hashLinkDelete to free the link.
 * @param map
 * @param key
 */
void hashMapRemove(HashMap *map, const char *key)
{
    // FIXME: implement
    assert(map != 0);
    assert(key != 0);

    int hashIndex = HASH_FUNCTION(key) % hashMapCapacity(map);
    struct HashLink *current = map->table[hashIndex];
    struct HashLink *prev = NULL;

    while (current != NULL)
    {
        if (strcmp(current->key, key) == 0)
        {
            if (prev == NULL)
            {
                map->table[hashIndex] = current->next;
            }
            else
            {
                prev->next = current->next;
            }
            // delete link and dec count, end function
            hashLinkDelete(current);
            map->size--;
            return;
        }
        prev = current;
        current = current->next;
    }
}

/**
 * Returns 1 if a link with the given key is in the table and 0 otherwise.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket. Also make sure to search the entire list.
 * 
 * @param map
 * @param key
 * @return 1 if the key is found, 0 otherwise.
 */
int hashMapContainsKey(HashMap *map, const char *key)
{
    assert(map != 0);
    assert(key != 0);

    // calculate the index
    int hashIndex = HASH_FUNCTION(key) % hashMapCapacity(map);

    struct HashLink *current = map->table[hashIndex];

    // check each link to see if it equals the key
    while (current != NULL)
    {
        if (strcmp((current->key), key) == 0)
        {
            return 1;
        }
        else
        {
            current = current->next;
        }
    }
    return 0;
}

/**
 * Returns the number of links in the table.
 * @param map
 * @return Number of links in the table.
 */
int hashMapSize(HashMap *map)
{
    assert(map != 0);
    return map->size;
}

/**
 * Returns the number of buckets in the table.
 * @param map
 * @return Number of buckets in the table.
 */
int hashMapCapacity(HashMap *map)
{
    assert(map != 0);
    return map->capacity;
}

/**
 * Returns the number of table buckets without any links.
 * @param map
 * @return Number of empty buckets.
 */
int hashMapEmptyBuckets(HashMap *map)
{
    int emptyBucketCounter = 0;

    for (int i = 0, cap = hashMapCapacity(map); i < cap; i++)
    {
        if (map->table[i] == NULL)
        {
            emptyBucketCounter++;
        }
    }
    return emptyBucketCounter;
}

/**
 * Returns the ratio of (number of links) / (number of buckets) in the table.
 * Remember that the buckets are linked lists, so this ratio tells you nothing
 * about the number of empty buckets. Remember also that the load is a floating
 * point number, so don't do integer division.
 * @param map
 * @return Table load.
 */
float hashMapTableLoad(HashMap *map)
{
    return (map->size / (float)map->capacity);
}

/**
 * Prints all the links in each of the buckets in the table.
 * @param map
 */
void hashMapPrint(HashMap *map)
{
    assert(map != 0);
    HashLink *current;

    for (int i = 0, cap = hashMapCapacity(map); i < cap; i++)
    {
        current = map->table[i];

        printf("%d: ", i);
        while (current != NULL)
        {
            printf("[%s, %d] ", current->key, current->value);
            current = current->next;
        }
        printf("\n");
    }
}
