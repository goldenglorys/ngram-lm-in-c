# N-gram Language Model: A Journey into the Basics of Machine Learning

Here's the wonderful world of n-gram Language Models! 🎉 This project help me understand the foundations of machine learning (training, evaluation, data splits, hyperparameters, overfitting) and  basis of autoregressive language modeling tokenization, next token prediction, perplexity, sampling. It's like learning to cook, but instead of creating culinary masterpieces, it's cooking up some language magic! 🧙‍♂️✨

## Table of Contents

1. [Introduction](#introduction)
2. [Getting Started](#getting-started)
3. [Implementation Steps](#implementation-steps)
4. [For the Beginners](#for-the-beginners)
5. [Technical Deep Dive](#technical-deep-dive)
6. [References](#references)

## Introduction

This n-gram Language Model is like a crystal ball for text prediction. It learns the patterns in a given text and then tries to predict what comes next. It's not quite as advanced as your smartphone's autocomplete, but it's a great start! GPT is "just" a very large n-gram model, too. The only difference is that GPT uses a neural network to calculate the probability of the next token, while n-gram uses a simple count-based approach.

The model will learn from a dataset of 32,032 names from [ssa.gov](https://www.ssa.gov/oact/babynames/) for the year 2018, which were split into 1,000 names in the test split, 1,000 in val split, and the rest in the training split, all of them inside the data/ folder. Therefore, the n-gram model will essentially try to learn the statistics of the characters in these names, and then generate new names by sampling from the model. Think of it as teaching a robot to come up with baby names. Who knows, maybe it'll invent the next "Khaleesi" or "X Æ A-12"! 👶

## Getting Started

### Prerequisites

- A C compiler (`clang`, but `gcc` works too)
- A sense of adventure and a tolerance for occasionally nonsensical computer-generated names

### Setup and Run

1. Clone this repository (or download the `ngram.c` file)
2. Open your terminal and navigate to the project directory
3. Run the following command:

```bash
clang -O3 -Wall -Wextra -Wpedantic -fsanitize=address -fsanitize=undefined -o ngram ngram.c && ./ngram
```

"But wait," you say, "what's all that gibberish?" Let's break it down:

- `clang`: This is our C compiler. It's like a translator that turns our human-readable code into computer instructions.
- `-O3`: This tells the compiler to optimize our code. It's like telling the translator to make our instructions as efficient as possible.
- `-Wall -Wextra -Wpedantic`: These are warning flags. They're like proofreaders that point out potential issues in our code.
- `-fsanitize=address -fsanitize=undefined`: These are like safety nets that catch certain types of programming errors.
- `-o ngram`: This names our output program "ngram".
- `ngram.c`: This is our source code file.
- `&& ./ngram`: This runs our program right after compiling it.

Running this command will compile the code and immediately run the program. You'll see some output showing the model's training process and its attempts at generating names.

## Implementation Steps

Here's a high-level overview of how n-gram model code works:

![flow diagram](./data/Screenshot%202024-07-17%20at%2019.17.14.png)

## For the Beginners

Imagine building a magical word prediction machine for a storytelling game. This machine will learn from stories it reads and then try to predict what word comes next in a new story. It's like having a friend who's read lots of books and can guess what you're going to say next!
The program is divided into several parts:

1. **Helper Functions**: These are like the machine's toolkit. They help it open books (files), remember words (allocate memory), and do quick calculations.
2. **Random Number Generators**: These are like dice rollers for the machine. When it needs to make a random choice, it uses these.
3. **The N-gram Model**: This is the heart of the machine. It learns patterns from the text it reads and uses these patterns to make predictions.
4. **The Main Function**: This is the control center. It sets up the machine, trains it, asks it to write a short story, and then tests how well it can predict words in a new story.

As you go through the code, think of each function as a different part of the machine, all working together to learn from stories and create new ones!

## Technical Deep Dive
Getting into the nitty-gritty, here's a more technical explanation of the key components:

1. **Tokenizer**: Converts characters to integer tokens and vice versa. It's the model's way of understanding individual characters.
2. **Tape Structure**: A fixed-size buffer that stores a sequence of tokens. It's like the model's short-term memory.
3. **N-gram Model**: The core of the program. It maintains counts of n-grams (sequences of n tokens) and uses these counts to calculate probabilities for next-token prediction.
4. **DataLoader**: Handles reading data from files, converting text to tokens, and feeding these tokens to the model for training or evaluation.
5. **Sampling**: Uses the trained model to generate new sequences of tokens, which we then convert back to text.

The model uses a simple count-based approach to calculate probabilities, with Laplace smoothing to handle unseen n-grams. The seq_len parameter determines the length of the n-grams used (e.g., seq_len = 4 means we're using 4-grams).

## References

This project was inspired by Andrej Karpathy's [makemore](https://github.com/karpathy/makemore) series. Check out his YouTube [video](https://www.youtube.com/watch?v=PaCmpygFfXo) for more insights!
For a deep dive into n-gram models and other NLP concepts, "Speech and Language Processing" by Jurafsky and Martin, is highly recommended especially [Chapter 3](https://web.stanford.edu/~jurafsky/slp3/3.pdf).
