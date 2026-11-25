#include <stddef.h>

typedef struct
{
    size_t symbol;
    size_t count;

} SymbolFrequency;

typedef struct
{
    size_t *context;
    size_t context_count;

    SymbolFrequency *next_symbols;
    size_t next_symbol_count;
    size_t next_symbol_capacity;

} ContextFrequency;

typedef struct
{
    size_t *block;
    size_t count;
} BlockFrequency;

int entropy_from_counts(const size_t *counts, size_t alphabet_size, double *entropy);
int max_entropy(size_t alphabet_size, double *result);

int calculate_redundancy(double entropy, double max_entropy, double *absolute, double *relative);

int conditional_entropy(const size_t *sequence, size_t length, size_t order, double *result);
int block_entropy(const size_t *sequence, size_t length, size_t order, double *result);