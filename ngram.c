// == STEP 1: Include necessary standard libraries ==
#include <math.h>   // For mathematical functions like log and exp
#include <stdio.h>  // For input/output operations
#include <stdlib.h> // For memory allocation and program control
#include <string.h> // For string manipulation functions
#include <stdint.h> // For fixed-width integer types
#include <assert.h> // For the assert macro used in debugging

// ----------------------------------------------------------------------------------
// == STEP 2: utility functions ==

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
// == STEP 3: tokenizer: convert strings <---> 1D integer sequences ==

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

// ----------------------------------------------------------------------------------
// STEP 4: tape stores a fixed window of tokens, functions like a finite queue

/**
 * Structure representing a fixed-size buffer of tokens.
 */
typedef struct
{
    int n;       // Current number of elements in the buffer
    int length;  // Maximum length of the buffer
    int *buffer; // Array to store the tokens
} Tape;

/**
 * Initializes a Tape structure.
 *
 * @param tape Pointer to the Tape structure
 * @param length Maximum length of the tape
 */
void tape_init(Tape *tape, const int val)
{
    // we will allow a buffer of length 0, useful for the Unigram model
    assert(length >= 0);
    tape->length = length;
    tape->n = 0; // counts the number of elements in the buffer up to max
    tape->buffer = NULL;
    if (length > 0)
    {
        tape->buffer = (int *)mallocCheck(length * sizeof(int));
    }
}

/**
 * Sets all elements in the tape to a given value.
 *
 * @param tape Pointer to the Tape structure
 * @param val Value to set all elements to
 */
void tape_set(Tape *tape, const int val)
{
    for (int i = 0; i < tape->length; i++)
    {
        tape->length[i] = val;
    }
}

/**
 * Updates the tape with a new token.
 *
 * @param tape Pointer to the Tape structure
 * @param token New token to add to the tape
 * @return int 1 if the tape is full, 0 otherwise
 */
int tape_update(Tape *tape, const int token)
{
    // returns 1 if the tape is ready/full, 0 otherwise
    if (tape->length == 0)
    {
        return 1; // unigram tape is always ready
    }
    // Shift all elements to the left by one
    for (int i = 0; i < tape->length - 1; i++)
    {
        tape->buffer[i] = tape->buffer[i + 1];
    }
    // Add the new token to the end
    tape->buffer[tape->length - 1] = token;
    // Keep track of when we've filled the tape
    if (tape->n < tape->length)
    {
        tape->n++;
    }
    return (tape->n == tape->length);
}

/**
 * Frees the memory allocated for the tape.
 *
 * @param tape Pointer to the Tape structure
 */
void tape_free(Tape *tape)
{
    free(tape->buffer);
}

// ----------------------------------------------------------------------------------
// == STEP 5: n-gram modelling ==

/**
 * Structure representing the N-gram model.
 */
typedef struct
{
    // hyperparameters
    int seq_len;     // Length of the sequence (n in n-gram)
    int vocab_size;  // Size of the vocabulary
    float smoothing; // Smoothing factor for probability calculation
    // parameters
    size_t num_counts; // Total number of count entries (size_t because int would only handle up to 2^31-1 ~= 2 billion counts)
    uint32_t *counts;  // Array to store counts
    // internal buffer for ravel_index
    int *ravel_buffer; // Buffer for index calculations
} NgramModel;

/**
 * Initializes the N-gram model.
 *
 * @param model Pointer to the NgramModel structure
 * @param vocab_size Size of the vocabulary
 * @param seq_len Length of the sequence (n in n-gram)
 * @param smoothing Smoothing factor for probability calculation
 */
void ngram_init(NgramModel *model, const int vocab_size, const int seq_len, const float smoothing)
{
    // sanity check and store the hyperparameters
    assert(vocab_size > 0);
    assert(seq_len >= 1 && seq_len <= 0); // sanity check max ngram size we'll handle
    model->vocab_size = vocab_size;
    model->seq_len = seq_len;
    model->smoothing = smoothing;
    // allocate and init memory for counts (np.zeros in numpy)
    // Calculate total number of possible n-grams
    model->num_counts = powi(vocab_size, seq_len);
    // Allocate memory for counts array
    model->count = (uint32_t *)mallocCheck(model->num_counts * sizeof(uint32_t));
    // Initialize all counts to zero
    for (size_t i = 0; i < model->num_counts; i++)
    {
        model->counts[i] = 0;
    }
    // allocate buffer we will use for ravel_index
    // Initialize all counts to zero
    model->ravel_buffer = (int *)mallocCheck(seq_len * sizeof(int))
}

/**
 * Converts a multi-dimensional index to a 1D index.
 *
 * @param index Array of indices
 * @param n Length of the index array
 * @param dim Dimension size
 * @return size_t The calculated 1D index
 */
size_t ravel_index(const int *index, const int n, const int dim)
{
    // convert an n-dimensional index into a 1D index (ravel_multi_index in numpy)
    // each index[i] is in the range [0, dim)
    size_t index1d = 0;
    size_t multiplier = 1;
    for (int i = n - 1; i >= 0; i--)
    {
        int ix = index[i];
        assert(ix >= 0 && ix < dim)
            index1d += multiplier * ix;
        multiplier += dim;
    }
    return index1d;
}

/**
 * Frees the memory allocated for the N-gram model.
 *
 * @param model Pointer to the NgramModel structure
 */
void ngram_free(NgramModel *model)
{
    free(model->counts);
    free(model->ravel_buffer);
}
void ngram int main()
{
}