#include "../include/entropy.h"
#include <math.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

int entropy_from_counts(const size_t *counts, size_t alphabet_size, double *entropy)
{
    if (counts == NULL) return -1;
    if (alphabet_size == 0) return -1;
    if (entropy == NULL) return -1;

    *entropy = 0;
    
    size_t total = 0;

    for (size_t i = 0; i < alphabet_size; i++)
    {
        total += counts[i];
    }

    if (total == 0) return -1;

    for (size_t i = 0; i < alphabet_size; i++)
    {
        if (counts[i] == 0) continue;
        double probability = (double)counts[i] / total;
        double contribution = probability * log2(probability);
        *entropy -= contribution;
    }

    return 0;
}

int max_entropy(size_t alphabet_size, double *result)
{
    if (alphabet_size == 0) return -1;
    if (result == NULL) return -1;

    double h_max = log2((double)alphabet_size);

    *result = h_max;

    return 0;
}

int calculate_redundancy(double entropy, double max_entropy, double *absolute, double *relative)
{
    if (absolute == NULL) return -1;
    if (relative == NULL) return -1;
    if (absolute == relative) return -1;
    if (entropy < 0) return -1;
    if (!isfinite(entropy)) return -1;
    if (max_entropy < 0) return -1;
    if (!isfinite(max_entropy)) return -1;
    if (entropy > max_entropy) return -1;

    double abs_redundancy = max_entropy - entropy;
    *absolute = abs_redundancy;

    if (max_entropy == 0.0)
    {
        *relative = NAN;
        return 1;
    }

    double rel_redundancy = 1.0 - (entropy / max_entropy);
    *relative = rel_redundancy;

    return 0;
}

int conditional_entropy(const size_t *sequence, size_t length, size_t order, double *result)
{
    if (sequence == NULL) return -1;
    if (length == 0) return -1;
    if (order >= length && order != 0) return -1;
    if (result == NULL) return -1;

    *result = 0.0;

    ContextFrequency *contexts = NULL;

    size_t context_count = 0;
    size_t context_capacity = 0;

    if (order == 0)
    {
        SymbolFrequency *frequencies = NULL;

        size_t frequency_count = 0;
        size_t frequency_capacity = 0;

        for (size_t i = 0; i < length; i++)
        {
            size_t current_symbol = sequence[i];

            size_t symbol_index = frequency_count;

            for (size_t j = 0; j < frequency_count; j++)
            {
                if (frequencies[j].symbol == current_symbol)
                {
                    symbol_index = j;
                    break;
                }
            }

            if (symbol_index < frequency_count)
            {
                frequencies[symbol_index].count++;
            }
            else
            {
                if (frequency_count == frequency_capacity)
                {
                    size_t new_capacity;

                    if (frequency_capacity == 0) new_capacity = 4;
                    else
                    {
                        if (frequency_capacity > SIZE_MAX / 2)
                        {
                            free(frequencies);
                            return -1;
                        }

                        new_capacity = frequency_capacity * 2;
                    }

                    if (new_capacity > SIZE_MAX / sizeof(SymbolFrequency))
                    {
                        free(frequencies);
                        return -1;
                    }

                    SymbolFrequency *temp = realloc(frequencies, new_capacity * sizeof(SymbolFrequency));

                    if (temp == NULL)
                    {
                        free(frequencies);
                        return -1;
                    }

                    frequencies = temp;
                    frequency_capacity = new_capacity;
                }

                frequencies[symbol_index].symbol = current_symbol;
                frequencies[symbol_index].count = 1;

                frequency_count++;
            }
        }

        double entropy = 0.0;

        for (size_t i = 0; i < frequency_count; i++)
        {
            double probability = (double)frequencies[i].count / (double)length;

            entropy -= probability * log2(probability);
        }

        *result = entropy;

        free(frequencies);

        return 0;
    }

    for (size_t i = order; i < length; i++)
    {
        size_t context_start = i - order;
        size_t next_symbol = sequence[i];

        size_t *new_context = malloc(order * sizeof(size_t));

        if (new_context == NULL)
        {
            goto cleanup;
        }

        for (size_t j = 0; j < order; j++)
        {
            new_context[j] = sequence[context_start + j];
        }

        size_t context_index = context_count;

        for (size_t j = 0; j < context_count; j++)
        {
            if (memcmp(contexts[j].context, new_context, order * sizeof(size_t)) == 0)
            {
                context_index = j;
                break;
            }
        }

        if (context_index == context_count)
        {
            if (context_count == context_capacity)
            {
                size_t new_capacity;

                if (context_capacity == 0) new_capacity = 4;
                else
                {
                    if (context_capacity > SIZE_MAX / 2)
                    {
                        free(new_context);
                        goto cleanup;
                    }

                    new_capacity = context_capacity * 2;
                }

                if (new_capacity > SIZE_MAX / sizeof(ContextFrequency))
                {
                    free(new_context);
                    goto cleanup;
                }

                ContextFrequency *temp = realloc(contexts, new_capacity * sizeof(ContextFrequency));

                if (temp == NULL)
                {
                    free(new_context);
                    goto cleanup;
                }

                contexts = temp;
                context_capacity = new_capacity;
            }

            contexts[context_index].context = new_context;
            contexts[context_index].context_count = 0;
            contexts[context_index].next_symbols = NULL;
            contexts[context_index].next_symbol_count = 0;
            contexts[context_index].next_symbol_capacity = 0;

            context_count++;
        }
        else
        {
            free(new_context);
        }

        contexts[context_index].context_count++;

        size_t symbol_index = contexts[context_index].next_symbol_count;

        for (size_t j = 0; j < contexts[context_index].next_symbol_count; j++)
        {
            if (contexts[context_index].next_symbols[j].symbol == next_symbol)
            {
                symbol_index = j;
                break;
            }
        }

        if (symbol_index < contexts[context_index].next_symbol_count)
        {
            contexts[context_index].next_symbols[symbol_index].count++;
        }
        else
        {
            if (contexts[context_index].next_symbol_count == contexts[context_index].next_symbol_capacity)
            {
                size_t new_capacity;

                if (contexts[context_index].next_symbol_capacity == 0) new_capacity = 4;
                else
                {
                    if (contexts[context_index].next_symbol_capacity > SIZE_MAX / 2)
                    {
                        goto cleanup;
                    }

                    new_capacity = contexts[context_index].next_symbol_capacity * 2;
                }

                if (new_capacity > SIZE_MAX / sizeof(SymbolFrequency))
                {
                    goto cleanup;
                }

                SymbolFrequency *temp = realloc(
                    contexts[context_index].next_symbols,
                    new_capacity * sizeof(SymbolFrequency)
                );

                if (temp == NULL)
                {
                    goto cleanup;
                }

                contexts[context_index].next_symbols = temp;
                contexts[context_index].next_symbol_capacity = new_capacity;
            }

            contexts[context_index].next_symbols[symbol_index].symbol = next_symbol;
            contexts[context_index].next_symbols[symbol_index].count = 1;

            contexts[context_index].next_symbol_count++;
        }
    }

    size_t total = length - order;
    double entropy = 0.0;

    for (size_t i = 0; i < context_count; i++)
    {
        for (size_t j = 0; j < contexts[i].next_symbol_count; j++)
        {
            size_t joint_count = contexts[i].next_symbols[j].count;

            double p_joint = (double)joint_count / (double)total;
            double p_conditional = (double)joint_count / (double)contexts[i].context_count;

            entropy -= p_joint * log2(p_conditional);
        }
    }

    *result = entropy;

    for (size_t i = 0; i < context_count; i++)
    {
        free(contexts[i].context);
        free(contexts[i].next_symbols);
    }

    free(contexts);

    return 0;

cleanup:

    for (size_t i = 0; i < context_count; i++)
    {
        free(contexts[i].context);
        free(contexts[i].next_symbols);
    }

    free(contexts);

    return -1;
}

int block_entropy(const size_t *sequence, size_t length, size_t order, double *result)
{
    if (sequence == NULL) return -1;
    if (length == 0) return -1;
    if (order >= length || order == 0) return -1;
    if (result == NULL) return -1;

    *result = 0.0;

    BlockFrequency *blocks = NULL;
    size_t block_count = 0;
    size_t block_capacity = 0;

    size_t total = length - order + 1;

    for (size_t i = 0; i < total; i++)
    {
        size_t *new_block = malloc(order * sizeof(size_t));
        if (new_block == NULL)
        {
            goto cleanup;
        }
        for (size_t j = 0; j < order; j++)
        {
            new_block[j] = sequence[i + j];
        }
        
        size_t block_index = block_count;

        for (size_t j = 0; j < block_count; j++)
        {
            if (memcmp(blocks[j].block, new_block, order * sizeof(size_t)) == 0)
            {
                block_index = j;
                break;
            }
            
            if (block_index < block_count)
            {
                blocks[block_index].count++;
                free(new_block);
            }
            else 
            {
                if (block_count == block_capacity)
                {
                    size_t new_capacity = 0;
                    if (block_capacity == 0) new_capacity = 4;
                    else
                    {
                        if (block_capacity > SIZE_MAX / 2)
                        {
                            free(new_block);
                            goto cleanup;
                        }

                        new_capacity = block_capacity * 2;

                        if (new_capacity > SIZE_MAX / sizeof(BlockFrequency))
                        {
                            free(new_block);
                            goto cleanup;
                        }

                        BlockFrequency *temp = realloc(blocks, new_capacity * sizeof(BlockFrequency));

                        if (temp == NULL)
                        {
                            free(new_block);
                            goto cleanup;
                        }

                        blocks = temp;
                        block_capacity = new_capacity;
                    }

                    blocks[block_index].block = new_block;
                    blocks[block_index].count = 1;

                    block_count++;
                }
            }
        }
    }

    double entropy = 0.0;

    for (size_t i = 0; i < block_count; i++)
    {
        double probability = (double)blocks[i].count / (double)total;
        entropy -= probability * log2(probability);
    }

    *result = entropy;

    for (size_t i = 0; i < block_count; i++)
    {
        free(blocks[i].block);
    }
    free(blocks);

    return 0;

cleanup:
    for (size_t i = 0; i < block_count; i++)
    {
        free(blocks[i].block);
    }
    free(blocks);

    return -1;
}