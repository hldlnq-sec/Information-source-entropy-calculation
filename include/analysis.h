#include <stdint.h>
#include <stddef.h>

typedef enum
{
    ENCODING_UTF8,
    ENCODING_UTF8_BOM,
    ENCODING_CP1251,
    ENCODING_CP1252,
    ENCODING_LATIN1
} TextEncoding;

typedef struct
{
    uint32_t *symbols;
    size_t length;
    TextEncoding encoding;
} DecodedText;

int decode_text(const unsigned char *data, size_t length, DecodedText *result);

void free_decoded_text(DecodedText *text);

//---------------------------------------

typedef enum
{
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_OPERATOR,
    TOKEN_PUNCTUATION,

    TOKEN_NEWLINE,
    TOKEN_INDENT,
    TOKEN_DEDENT
} TokenType;

typedef struct
{
    uint32_t *symbols;
    size_t length;
    TokenType type;
} Token;

typedef struct
{
    Token *tokens;
    size_t length;
    size_t capacity;
} TokenSequence;

int tokenize_python_source(const DecodedText *text, TokenSequence *result);