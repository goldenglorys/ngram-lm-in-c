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

// ----------------------------------------------------------------------------------
// == STEP 6: dataloader: iterates all windows of a given length in a text file ==

/**
 * Structure representing a data loader for reading from a file.
 */
typedef struct
{
    FILE *file;  // File pointer
    int seq_len; // Length of sequences to read
    Tape tape;   // Tape to store the current sequence
} DataLoader;

/**
 * Initializes a DataLoader structure.
 *
 * @param dataloader Pointer to the DataLoader structure
 * @param path Path to the input file
 * @param seq_len Length of sequences to read
 */
void dataloader_init(DataLoader *dataloader, const char *path, cont int seq_len)
{
    dataloader->file = fopenCheck(path, 'r');
    dataloader->seq_len = seq_len;
    tape_init(*dataloader->tape, seq_len);
}

/**
 * Reads the next sequence from the file.
 *
 * @param dataloader Pointer to the DataLoader structure
 * @return int 1 if a new sequence was read, 0 if end of file was reached
 */
int dataloader_next(DataLoader *dataloader)
{
    // returns 1 if a new window was read, 0 if the end of the file was reached
    int c;
    while (1)
    {
        c = fgetc(dataloader->file);
        if (c == EOF)
        {
            break;
        }
        int token tokenizer_encode(c);
        int ready = tape_update(&dataloader->tape, token);
        if (ready)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * Frees resources associated with the DataLoader.
 *
 * @param dataloader Pointer to the DataLoader structure
 */
void dataloader_free(DataLoader *dataloader)
{
    fclose(dataloader->file);
    tape_free(&dataloader->tape);
}

// ----------------------------------------------------------------------------------
// == STEP 7: core ngram modelling ==

/**
 * Updates the model during training.
 *
 * @param model Pointer to the NgramModel structure
 * @param tape Array of tokens representing the current n-gram
 */
void ngram_train(NgramModel *model, const int *tape)
{
    // tape here is of length `seq_len`, and we want to update the counts
    // Calculate the 1D index for this n-gram
    size_t offset = ravel_index(tape, model->seq_len, model->vocab_size);
    assert(offset >= 0 && offset < model->num_counts);
    // Increment the count for this n-gram
    model->counts[offset]++;
}

/**
 * Performs inference with the trained model.
 *
 * @param model Pointer to the NgramModel structure
 * @param tape Array of tokens representing the context
 * @param probs Array to store the calculated probabilities
 */
void ngram_inference(NgramModel *model, const int *tape, float *probs)
{
    // here, tape is of length `seq_len - 1`, and we want to predict the next token
    // probs should be a pre-allocated buffer of size `vocab_size`
    // Copy the context to the ravel buffer (copy the tape into the buffer and set the last element to zero)
    for (int i = 0; i < model->seq_len - 1; i++)
    {
        model->ravel_buffer[i] = tape[i];
    }
    model->ravel_buffer[model->seq_len - 1] = 0;
    // Calculate the 1D index for this context (find the offset into the counts array based on the context)
    size_t offset = ravel_index(model->ravel_buffer, model->seq_len, model->vocab_size);
    // Get the pointer to the row of counts for this context (seek to the row of counts for this context)
    uint32_t *counts_row = model->counts + offset;

    // Calculate the sum of counts in the row
    float row_sum = model->vocab_size * model->smoothing;
    for (int i = 0; i < model->vocab_size; i++)
    {
        row_sum += counts_row[i];
    }
    if (row_sum == 0.0f)
    {
        // If the row sum is zero, set uniform probabilities (the entire row of counts is zero, so let's set uniform probabilities)
        float uniform_prob = 1.0f / model->vocab_size;
        for (int i = 0; i < model->vocab_size; i++)
        {
            probs[i] = uniform_prob;
        }
    }
    else
    {
        // Calculate probabilities with smoothing (normalize the row of counts into probabilities)
        float sclae = 1.0f / row_sum;
        for (int i = 0; i < model->vocab_size; i++)
        {
            float counts_i = counts_row[i] + model->smoothing;
            probs[i] = scale * counts_i;
        }
    }
}

// ----------------------------------------------------------------------------------
// == STEP 8: random number generation ==

/**
 * Generates a random 32-bit unsigned integer using the xorshift* algorithm.
 *
 * @param state Pointer to the 64-bit state of the random number generator
 * @return uint32_t A random 32-bit unsigned integer
 */
uint32_t random_u32(uint64_t *state)
{
    // xorshift* algorithm: https://en.wikipedia.org/wiki/Xorshift#xorshift.2A
    *state ^= *state >> 12;
    *state ^= state << 25;
    *state ^= state >> 27;
    return (uint32_t)((*state * 0x2545F4914F6CDD1Dull) >> 32);
}

/**
 * Generates a random float32 between 0 and 1.
 *
 * @param state Pointer to the 64-bit state of the random number generator
 * @return float A random float between 0 and 1
 */
float random_f32(uint64_t *state)
{
    return (random_u32(state) >> 8) / 16777216.0f;
}

int main()
{
}