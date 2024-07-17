// Include necessary standard libraries
#include <math.h>   // For mathematical functions like log and exp
#include <stdio.h>  // For input/output operations
#include <stdlib.h> // For memory allocation and program control
#include <string.h> // For string manipulation functions
#include <stdint.h> // For fixed-width integer types
#include <assert.h> // For the assert macro used in debugging

// ----------------------------------------------------------------------------------
// == utility functions ==

/**
 * Computes integer exponentiation.
 *
 * @param base The base number
 * @param exp The exponent
 * @return size_t The result of base raised to the power of exp
 */
size_t powi(int base, int exp)
{
    size_t result = 1;
    for (int i = 0; i < exp; i++)
    {
        result *= base;
    }
    return result;
}

/**
 * Safely opens a file and checks for errors.
 *
 * @param path The path to the file to be opened
 * @param mode The mode in which to open the file (e.g., "r" for read)
 * @param file The name of the source file calling this function (__FILE__)
 * @param line The line number where this function is called (__LINE__)
 * @return FILE* A pointer to the opened file
 */
FILE *fopen_check(const char *path, const char *mode, char *file, int line)
{
    FILE *fp = fopen(path, mode);
    if (fp == NULL)
    {
        // If file opening fails, print an error message and exit the program
        fprintf(stderr, "Error: Failed to open file '%s' at %s:%d\n", path, file, line);
        exit(EXIT_FAILURE);
    }
    return fp;
}
// Macro to automatically pass __FILE__ and __LINE__ to fopen_check
#define fopenCheck(path, mode) fopen_check(path, mode, __FILE__, __LINE__)

/**
 * Safely allocates memory and checks for errors.
 *
 * @param size The number of bytes to allocate
 * @param file The name of the source file calling this function (__FILE__)
 * @param line The line number where this function is called (__LINE__)
 * @return void* A pointer to the allocated memory
 */
void *malloc_check(size_t size, const char *file, int line)
{
    void *ptr = malloc(size);
    if (ptr == NULL)
    {
        // If memory allocation fails, print an error message and exit the program
        fprintf(stderr, "Error: Memory allocation failed at %s:%d\n", file, line);
        exit(EXIT_FAILURE);
    }
    return ptr;
}
// Macro to automatically pass __FILE__ and __LINE__ to malloc_check
#define mallocCheck(size) malloc_check(size, __FILE__, __LINE__)

// ----------------------------------------------------------------------------------
// == tokenizer: convert strings <---> 1D integer sequences ==

// 26 lowercase letters + 1 end-of-text token

// Define the number of tokens in our vocabulary
#define NUM_TOKENS = 27
// Define the end-of-text token
#define EOT_TOKEN = 0

/**
 * Encodes a character to its corresponding token ID.
 *
 * @param c The character to encode
 * @return int The token ID of the character
 */
int tokenizer_encode(const char c)
{
    // characters a-z are encoded as 1-26, and '\n' is encoded as 0
    assert(c == '\n' || ('a' <= c && c <= 'z'));
    int token = (c == '\n') ? EOT_TOKEN : (c - 'a' + 1);
    return token;
}

/**
 * Decodes a token ID back to its corresponding character.
 *
 * @param token The token ID to decode
 * @return char The character corresponding to the token ID
 */
char tokenizer_decode(const int token)
{
    // tokens 0-25 are decoded as a-z, and token 26 is decoded as '\n'
    assert(token >= 0 && token < NUM_TOKEN);
    char c = (token == EOT_TOKEN) ? '\n' : 'a' + (token - 1);
    return c;
}
int main()
{
}