#include "../include/analysis.h"
#include <complex.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static const uint16_t cp1251_table[128] =
{
    0x0402, 0x0403, 0x201A, 0x0453, 0x201E, 0x2026, 0x2020, 0x2021,
    0x20AC, 0x2030, 0x0409, 0x2039, 0x040A, 0x040C, 0x040B, 0x040F,
    0x0452, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0xFFFD, 0x2122, 0x0459, 0x203A, 0x045A, 0x045C, 0x045B, 0x045F,
    0x00A0, 0x040E, 0x045E, 0x0408, 0x00A4, 0x0490, 0x00A6, 0x00A7,
    0x0401, 0x00A9, 0x0404, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x0407,
    0x00B0, 0x00B1, 0x0406, 0x0456, 0x0491, 0x00B5, 0x00B6, 0x00B7,
    0x0451, 0x2116, 0x0454, 0x00BB, 0x0458, 0x0405, 0x0455, 0x0457,
    0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417,
    0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F,
    0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427,
    0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F,
    0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437,
    0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F,
    0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447,
    0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F
};

static const uint16_t cp1252_table[128] =
{
    0x20AC, 0x0000, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
    0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x0000, 0x017D, 0x0000,
    0x0000, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x0000, 0x017E, 0x0178,
    0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
    0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
    0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
    0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
    0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
    0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
    0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
    0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
    0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
    0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
    0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF
};

static int is_valid_utf8(const unsigned char *data, size_t length)
{
    if (data == NULL && length > 0)
    {
        return 0;
    }

    size_t i = 0;

    while (i < length)
    {
        unsigned char byte = data[i];

        if (byte <= 0x7F)
        {
            i++;
        }
        else if (byte >= 0xC2 && byte <= 0xDF)
        {
            if (length - i < 2) return 0;

            if ((data[i + 1] & 0xC0) != 0x80) return 0;

            i += 2;
        }
        else if (byte >= 0xE0 && byte <= 0xEF)
        {
            if (length - i < 3) return 0;

            unsigned char byte2 = data[i + 1];
            unsigned char byte3 = data[i + 2];

            if ((byte2 & 0xC0) != 0x80) return 0;
            if ((byte3 & 0xC0) != 0x80) return 0;

            if (byte == 0xE0 && byte2 < 0xA0) return 0;
            if (byte == 0xED && byte2 >= 0xA0) return 0;

            i += 3;
        }
        else if (byte >= 0xF0 && byte <= 0xF4)
        {
            if (length - i < 4) return 0;

            unsigned char byte2 = data[i + 1];
            unsigned char byte3 = data[i + 2];
            unsigned char byte4 = data[i + 3];

            if ((byte2 & 0xC0) != 0x80) return 0;
            if ((byte3 & 0xC0) != 0x80) return 0;
            if ((byte4 & 0xC0) != 0x80) return 0;

            if (byte == 0xF0 && byte2 < 0x90) return 0;
            if (byte == 0xF4 && byte2 > 0x8F) return 0;

            i += 4;
        }
        else
        {
            return 0;
        }
    }

    return 1;
}


static int decode_utf8(const unsigned char *data, size_t length, uint32_t **symbols, size_t *symbol_count)
{
    if (data == NULL && length > 0) return -1;
    if (symbols == NULL) return -1;
    if (symbol_count == NULL) return -1;

    *symbols = NULL;
    *symbol_count = 0;

    if (!is_valid_utf8(data, length)) return -1;

    if (length == 0) return 0;

    if (length > SIZE_MAX / sizeof(uint32_t)) return -1;

    uint32_t *result = malloc(length * sizeof(uint32_t));

    if (result == NULL) return -1;

    size_t i = 0;
    size_t count = 0;

    while (i < length)
    {
        unsigned char byte = data[i];

        if (byte <= 0x7F)
        {
            result[count] = byte;
            i++;
        }
        else if (byte <= 0xDF)
        {
            uint32_t codepoint = ((uint32_t)(byte & 0x1F) << 6) | ((uint32_t)(data[i + 1] & 0x3F));

            result[count] = codepoint;
            i += 2;
        }
        else if (byte <= 0xEF)
        {
            uint32_t codepoint = ((uint32_t)(byte & 0x0F) << 12) | ((uint32_t)(data[i + 1] & 0x3F) << 6) | ((uint32_t)(data[i + 2] & 0x3F));

            result[count] = codepoint;
            i += 3;
        }
        else
        {
            uint32_t codepoint = ((uint32_t)(byte & 0x07) << 18) | ((uint32_t)(data[i + 1] & 0x3F) << 12) | ((uint32_t)(data[i + 2] & 0x3F) << 6) | ((uint32_t)(data[i + 3] & 0x3F));

            result[count] = codepoint;
            i += 4;
        }

        count++;
    }

    *symbols = result;
    *symbol_count = count;

    return 0;
}


static int decode_cp1251(
    const unsigned char *data,
    size_t length,
    uint32_t **symbols,
    size_t *symbol_count
)
{
    if (data == NULL && length > 0) return -1;
    if (symbols == NULL) return -1;
    if (symbol_count == NULL) return -1;

    *symbols = NULL;
    *symbol_count = 0;

    if (length == 0) return 0;

    if (length > SIZE_MAX / sizeof(uint32_t)) return -1;

    uint32_t *result = malloc(length * sizeof(uint32_t));

    if (result == NULL) return -1;

    for (size_t i = 0; i < length; i++)
    {
        unsigned char byte = data[i];

        if (byte == 0x98)
        {
            free(result);
            return -1;
        }

        if (byte < 0x80)
        {
            result[i] = byte;
        }
        else
        {
            result[i] = cp1251_table[byte - 0x80];
        }
    }

    *symbols = result;
    *symbol_count = length;

    return 0;
}

static int decode_cp1252(
    const unsigned char *data,
    size_t length,
    uint32_t **symbols,
    size_t *symbol_count
)
{
    if (data == NULL && length > 0) return -1;
    if (symbols == NULL) return -1;
    if (symbol_count == NULL) return -1;

    *symbols = NULL;
    *symbol_count = 0;

    if (length == 0) return 0;

    if (length > SIZE_MAX / sizeof(uint32_t)) return -1;

    uint32_t *result = malloc(length * sizeof(uint32_t));

    if (result == NULL) return -1;

    for (size_t i = 0; i < length; i++)
    {
        unsigned char byte = data[i];

        if (byte < 0x80)
        {
            result[i] = byte;
        }
        else
        {
            uint16_t codepoint = cp1252_table[byte - 0x80];

            if (codepoint == 0x0000)
            {
                free(result);
                return -1;
            }

            result[i] = codepoint;
        }
    }

    *symbols = result;
    *symbol_count = length;

    return 0;
}

static int decode_latin1(
    const unsigned char *data,
    size_t length,
    uint32_t **symbols,
    size_t *symbol_count
)
{
    if (data == NULL && length > 0) return -1;
    if (symbols == NULL) return -1;
    if (symbol_count == NULL) return -1;

    *symbols = NULL;
    *symbol_count = 0;

    if (length == 0) return 0;

    if (length > SIZE_MAX / sizeof(uint32_t)) return -1;

    uint32_t *result = malloc(length * sizeof(uint32_t));

    if (result == NULL) return -1;

    for (size_t i = 0; i < length; i++)
    {
        result[i] = data[i];
    }

    *symbols = result;
    *symbol_count = length;

    return 0;
}

int decode_text(
    const unsigned char *data,
    size_t length,
    DecodedText *result
)
{
    if (data == NULL && length > 0) return -1;
    if (result == NULL) return -1;

    result->symbols = NULL;
    result->length = 0;
    result->encoding = ENCODING_UTF8;

    if (length >= 3 &&
        data[0] == 0xEF &&
        data[1] == 0xBB &&
        data[2] == 0xBF)
    {
        if (decode_utf8(data + 3, length - 3, &result->symbols, &result->length) != 0)
        {
            return -1;
        }

        result->encoding = ENCODING_UTF8_BOM;
        return 0;
    }

    if (decode_utf8(data, length, &result->symbols, &result->length) == 0)
    {
        result->encoding = ENCODING_UTF8;
        return 0;
    }

    if (decode_cp1251(data, length, &result->symbols, &result->length) == 0)
    {
        result->encoding = ENCODING_CP1251;
        return 0;
    }

    if (decode_cp1252(data, length, &result->symbols, &result->length) == 0)
    {
        result->encoding = ENCODING_CP1252;
        return 0;
    }

    if (decode_latin1(data, length, &result->symbols, &result->length) == 0)
    {
        result->encoding = ENCODING_LATIN1;
        return 0;
    }

    return -1;
}
void free_decoded_text(DecodedText *text)
{
    if (text == NULL) return;

    free(text->symbols);

    text->symbols = NULL;
    text->length = 0;
    text->encoding = ENCODING_UTF8;
}


// Лексический и структурный анализ исходного кода Python.
// Поддерживаемое подмножество описано в комментариях к тестам.

static int is_identifier_start(uint32_t symbol)
{
    return symbol == '_' ||
           (symbol >= 'A' && symbol <= 'Z') ||
           (symbol >= 'a' && symbol <= 'z');
}

static int is_identifier_char(uint32_t symbol)
{
    return is_identifier_start(symbol) ||
           (symbol >= '0' && symbol <= '9');
}

static int is_ascii_digit(uint32_t symbol)
{
    return symbol >= '0' && symbol <= '9';
}

static int is_hex_digit(uint32_t symbol)
{
    return is_ascii_digit(symbol) ||
           (symbol >= 'a' && symbol <= 'f') ||
           (symbol >= 'A' && symbol <= 'F');
}

static int is_binary_digit(uint32_t symbol)
{
    return symbol == '0' || symbol == '1';
}

static int is_octal_digit(uint32_t symbol)
{
    return symbol >= '0' && symbol <= '7';
}

static int append_token(
    TokenSequence *sequence,
    const uint32_t *symbols,
    size_t length,
    TokenType type
)
{
    if (sequence == NULL || (symbols == NULL && length > 0))
    {
        return -1;
    }

    if (sequence->length > sequence->capacity)
    {
        return -1;
    }

    if (sequence->length == sequence->capacity)
    {
        size_t new_capacity;

        if (sequence->capacity == 0)
        {
            new_capacity = 8;
        }
        else
        {
            if (sequence->capacity > SIZE_MAX / 2)
            {
                return -1;
            }

            new_capacity = sequence->capacity * 2;
        }

        if (new_capacity > SIZE_MAX / sizeof(Token))
        {
            return -1;
        }

        Token *temp = realloc(
            sequence->tokens,
            new_capacity * sizeof(Token)
        );

        if (temp == NULL)
        {
            return -1;
        }

        sequence->tokens = temp;
        sequence->capacity = new_capacity;
    }

    uint32_t *token_symbols = NULL;

    if (length > 0)
    {
        if (length > SIZE_MAX / sizeof(uint32_t))
        {
            return -1;
        }

        token_symbols = malloc(length * sizeof(uint32_t));

        if (token_symbols == NULL)
        {
            return -1;
        }

        memcpy(
            token_symbols,
            symbols,
            length * sizeof(uint32_t)
        );
    }

    sequence->tokens[sequence->length].symbols = token_symbols;
    sequence->tokens[sequence->length].length = length;
    sequence->tokens[sequence->length].type = type;
    sequence->length++;

    return 0;
}

static void free_token_sequence(TokenSequence *sequence)
{
    if (sequence == NULL)
    {
        return;
    }

    if (sequence->tokens != NULL)
    {
        for (size_t i = 0; i < sequence->length; i++)
        {
            free(sequence->tokens[i].symbols);
        }
    }

    free(sequence->tokens);

    sequence->tokens = NULL;
    sequence->length = 0;
    sequence->capacity = 0;
}

static int append_special_token(TokenSequence *sequence, TokenType type)
{
    return append_token(sequence, NULL, 0, type);
}

static size_t newline_length(
    const uint32_t *symbols,
    size_t index,
    size_t length
)
{
    if (symbols == NULL || index >= length)
    {
        return 0;
    }

    if (symbols[index] == '\r')
    {
        if (index + 1 < length && symbols[index + 1] == '\n')
        {
            return 2;
        }

        return 1;
    }

    return symbols[index] == '\n' ? 1 : 0;
}

static int is_string_prefix_char(uint32_t symbol)
{
    return symbol == 'r' || symbol == 'R' ||
           symbol == 'u' || symbol == 'U' ||
           symbol == 'b' || symbol == 'B' ||
           symbol == 'f' || symbol == 'F';
}

static int is_valid_string_prefix(
    const uint32_t *symbols,
    size_t start,
    size_t length
)
{
    if (symbols == NULL || length == 0 || length > 2)
    {
        return 0;
    }

    char prefix[3] = {0};

    for (size_t i = 0; i < length; i++)
    {
        if (symbols[start + i] > 0x7F)
        {
            return 0;
        }

        char c = (char)symbols[start + i];

        if (c >= 'A' && c <= 'Z')
        {
            c = (char)(c - 'A' + 'a');
        }

        prefix[i] = c;
    }

    return strcmp(prefix, "r") == 0 ||
           strcmp(prefix, "u") == 0 ||
           strcmp(prefix, "b") == 0 ||
           strcmp(prefix, "f") == 0 ||
           strcmp(prefix, "br") == 0 ||
           strcmp(prefix, "rb") == 0 ||
           strcmp(prefix, "fr") == 0 ||
           strcmp(prefix, "rf") == 0;
}

/*
 * Распознаёт строку, в том числе с префиксом r/b/f/u/br/rb/fr/rf.
 * start указывает на первый символ префикса либо на кавычку.
 * При успехе end получает позицию сразу после закрывающей кавычки.
 */
static int scan_python_string(
    const uint32_t *symbols,
    size_t length,
    size_t start,
    size_t quote_index,
    size_t *end
)
{
    if (symbols == NULL || end == NULL ||
        start >= length || quote_index >= length)
    {
        return -1;
    }

    uint32_t quote = symbols[quote_index];

    if (quote != '\'' && quote != '"')
    {
        return -1;
    }

    int triple =
        quote_index + 2 < length &&
        symbols[quote_index + 1] == quote &&
        symbols[quote_index + 2] == quote;

    size_t i = quote_index + (triple ? 3 : 1);

    while (i < length)
    {
        if (symbols[i] == '\\')
        {
            if (i + 1 >= length)
            {
                return -1;
            }

            size_t escaped_newline =
                newline_length(symbols, i + 1, length);

            if (escaped_newline > 0)
            {
                i += 1 + escaped_newline;
            }
            else
            {
                i += 2;
            }

            continue;
        }

        if (triple)
        {
            if (i + 2 < length &&
                symbols[i] == quote &&
                symbols[i + 1] == quote &&
                symbols[i + 2] == quote)
            {
                *end = i + 3;
                return 0;
            }

            i++;
            continue;
        }

        if (newline_length(symbols, i, length) > 0)
        {
            return -1;
        }

        if (symbols[i] == quote)
        {
            *end = i + 1;
            return 0;
        }

        i++;
    }

    (void)start;
    return -1;
}

/*
 * Проверяет, начинается ли строковый литерал в позиции i.
 * Например: "text", r"text", f"text", br"bytes".
 */
static int find_string_start(
    const uint32_t *symbols,
    size_t length,
    size_t i,
    size_t *quote_index
)
{
    if (symbols == NULL || quote_index == NULL || i >= length)
    {
        return 0;
    }

    if (symbols[i] == '\'' || symbols[i] == '"')
    {
        *quote_index = i;
        return 1;
    }

    if (!is_string_prefix_char(symbols[i]))
    {
        return 0;
    }

    size_t j = i;

    while (j < length &&
           j - i < 2 &&
           is_string_prefix_char(symbols[j]))
    {
        j++;
    }

    if (j >= length ||
        (symbols[j] != '\'' && symbols[j] != '"'))
    {
        return 0;
    }

    if (!is_valid_string_prefix(symbols, i, j - i))
    {
        return 0;
    }

    *quote_index = j;
    return 1;
}

/* Считывает одну группу цифр, разрешая подчёркивания между цифрами. */
static int scan_digit_part(
    const uint32_t *symbols,
    size_t length,
    size_t *index,
    int (*is_digit)(uint32_t)
)
{
    size_t i = *index;
    int has_digit = 0;
    int previous_was_digit = 0;

    while (i < length)
    {
        if (is_digit(symbols[i]))
        {
            has_digit = 1;
            previous_was_digit = 1;
            i++;
        }
        else if (symbols[i] == '_')
        {
            if (!previous_was_digit ||
                i + 1 >= length ||
                !is_digit(symbols[i + 1]))
            {
                return -1;
            }

            previous_was_digit = 0;
            i++;
        }
        else
        {
            break;
        }
    }

    *index = i;
    return has_digit ? 0 : -1;
}

static int scan_number(
    const uint32_t *symbols,
    size_t length,
    size_t start,
    size_t *end
)
{
    if (symbols == NULL || end == NULL || start >= length)
    {
        return -1;
    }

    size_t i = start;

    if (symbols[i] == '0' && i + 1 < length)
    {
        uint32_t prefix = symbols[i + 1];

        if (prefix == 'x' || prefix == 'X' ||
            prefix == 'b' || prefix == 'B' ||
            prefix == 'o' || prefix == 'O')
        {
            i += 2;

            int (*digit_test)(uint32_t) = NULL;

            if (prefix == 'x' || prefix == 'X')
            {
                digit_test = is_hex_digit;
            }
            else if (prefix == 'b' || prefix == 'B')
            {
                digit_test = is_binary_digit;
            }
            else
            {
                digit_test = is_octal_digit;
            }

            if (scan_digit_part(symbols, length, &i, digit_test) != 0)
            {
                return -1;
            }

            *end = i;
            return 0;
        }
    }

    if (symbols[i] == '.')
    {
        i++;

        if (scan_digit_part(
                symbols, length, &i, is_ascii_digit) != 0)
        {
            return -1;
        }
    }
    else
    {
        if (scan_digit_part(
                symbols, length, &i, is_ascii_digit) != 0)
        {
            return -1;
        }

        if (i < length && symbols[i] == '.' &&
            !(i + 1 < length && symbols[i + 1] == '.'))
        {
            i++;

            if (i < length && is_ascii_digit(symbols[i]))
            {
                if (scan_digit_part(
                        symbols, length, &i, is_ascii_digit) != 0)
                {
                    return -1;
                }
            }
        }
    }

    if (i < length && (symbols[i] == 'e' || symbols[i] == 'E'))
    {
        i++;

        if (i < length &&
            (symbols[i] == '+' || symbols[i] == '-'))
        {
            i++;
        }

        if (scan_digit_part(
                symbols, length, &i, is_ascii_digit) != 0)
        {
            return -1;
        }
    }

    if (i < length && (symbols[i] == 'j' || symbols[i] == 'J'))
    {
        i++;
    }

    *end = i;
    return 0;
}

static int is_punctuation(uint32_t symbol)
{
    return symbol == '(' || symbol == ')' ||
           symbol == '[' || symbol == ']' ||
           symbol == '{' || symbol == '}' ||
           symbol == ',' || symbol == ':' ||
           symbol == ';' || symbol == '.';
}

static int match_operator(
    const uint32_t *symbols,
    size_t length,
    size_t index,
    size_t *operator_length
)
{
    /*
     * Сначала проверяются самые длинные операторы.
     * Это обеспечивает правило maximal munch:
     * **= распознаётся раньше ** и =.
     */
    static const char *operators[] =
    {
        "**=", "//=", ">>=", "<<=",
        "...", ":=", "->",
        "+=", "-=", "*=", "/=", "%=",
        "&=", "|=", "^=", "@=",
        "==", "!=", "<=", ">=",
        "**", "//", "<<", ">>",
        "+", "-", "*", "/", "%",
        "@", "&", "|", "^", "~",
        "<", ">", "="
    };

    if (symbols == NULL || operator_length == NULL)
    {
        return 0;
    }

    for (size_t op = 0;
         op < sizeof(operators) / sizeof(operators[0]);
         op++)
    {
        size_t op_length = strlen(operators[op]);

        if (op_length > length - index)
        {
            continue;
        }

        size_t j = 0;

        while (j < op_length &&
               symbols[index + j] == (unsigned char)operators[op][j])
        {
            j++;
        }

        if (j == op_length)
        {
            *operator_length = op_length;
            return 1;
        }
    }

    return 0;
}

static int append_source_token(
    TokenSequence *result,
    const uint32_t *symbols,
    size_t start,
    size_t end,
    TokenType type
)
{
    if (end < start)
    {
        return -1;
    }

    return append_token(
        result,
        symbols + start,
        end - start,
        type
    );
}

int tokenize_python_source(
    const DecodedText *text,
    TokenSequence *result
)
{
    if (text == NULL || result == NULL)
    {
        return -1;
    }

    if (text->length > 0 && text->symbols == NULL)
    {
        return -1;
    }

    if (result->tokens != NULL ||
        result->length != 0 ||
        result->capacity != 0)
    {
        return -1;
    }

    size_t indentation_stack[256] = {0};
    size_t indentation_level = 1;

    uint32_t bracket_stack[256] = {0};
    size_t bracket_level = 0;

    size_t i = 0;
    size_t indentation = 0;

    int at_line_start = 1;
    int logical_line_has_code = 0;
    int explicit_continuation = 0;

    while (i < text->length)
    {
        uint32_t symbol = text->symbols[i];

        /*
         * Обработка начала физической строки.
         * Отступы учитываются только вне скобок и вне
         * явного продолжения строки.
         */
        if (at_line_start)
        {
            indentation = 0;

            if (bracket_level == 0 && !explicit_continuation)
            {
                while (i < text->length &&
                       (text->symbols[i] == ' ' ||
                        text->symbols[i] == '\t'))
                {
                    if (text->symbols[i] == ' ')
                    {
                        if (indentation == SIZE_MAX)
                        {
                            goto cleanup;
                        }

                        indentation++;
                    }
                    else
                    {
                        if (indentation > SIZE_MAX - 8)
                        {
                            goto cleanup;
                        }

                        indentation =
                            ((indentation / 8) + 1) * 8;
                    }

                    i++;
                }
            }
            else
            {
                while (i < text->length &&
                       (text->symbols[i] == ' ' ||
                        text->symbols[i] == '\t'))
                {
                    i++;
                }
            }

            if (i >= text->length)
            {
                break;
            }

            symbol = text->symbols[i];

            /* Пустая физическая строка. */
            if (newline_length(
                    text->symbols, i, text->length) > 0)
            {
                at_line_start = 0;
                continue;
            }

            /* Комментарий в начале физической строки. */
            if (symbol == '#')
            {
                while (i < text->length &&
                       newline_length(
                           text->symbols, i, text->length) == 0)
                {
                    i++;
                }

                at_line_start = 0;
                continue;
            }

            if (bracket_level == 0 && !explicit_continuation)
            {
                size_t current_indent =
                    indentation_stack[indentation_level - 1];

                if (indentation > current_indent)
                {
                    if (indentation_level >= 256)
                    {
                        goto cleanup;
                    }

                    indentation_stack[indentation_level] =
                        indentation;
                    indentation_level++;

                    if (append_special_token(
                            result, TOKEN_INDENT) != 0)
                    {
                        goto cleanup;
                    }
                }
                else if (indentation < current_indent)
                {
                    while (indentation_level > 1 &&
                           indentation <
                           indentation_stack[indentation_level - 1])
                    {
                        indentation_level--;

                        if (append_special_token(
                                result, TOKEN_DEDENT) != 0)
                        {
                            goto cleanup;
                        }
                    }
                }

                if (indentation !=
                    indentation_stack[indentation_level - 1])
                {
                    goto cleanup;
                }
            }

            explicit_continuation = 0;
            at_line_start = 0;
            continue;
        }

        /* Перевод физической строки. */
        size_t nl = newline_length(
            text->symbols, i, text->length);

        if (nl > 0)
        {
            if (bracket_level == 0 &&
                !explicit_continuation &&
                logical_line_has_code)
            {
                if (append_special_token(
                        result, TOKEN_NEWLINE) != 0)
                {
                    goto cleanup;
                }

                logical_line_has_code = 0;
            }

            i += nl;
            at_line_start = 1;
            continue;
        }

        /* Пробельные символы внутри строки. */
        if (symbol == ' ' || symbol == '\t' || symbol == '\f')
        {
            i++;
            continue;
        }

        /* Комментарий до конца физической строки. */
        if (symbol == '#')
        {
            while (i < text->length &&
                   newline_length(
                       text->symbols, i, text->length) == 0)
            {
                i++;
            }

            continue;
        }

        /* Строковый литерал с возможным префиксом. */
        size_t quote_index = 0;

        if (find_string_start(
                text->symbols,
                text->length,
                i,
                &quote_index))
        {
            size_t string_end = 0;

            if (scan_python_string(
                    text->symbols,
                    text->length,
                    i,
                    quote_index,
                    &string_end) != 0)
            {
                goto cleanup;
            }

            if (append_source_token(
                    result,
                    text->symbols,
                    i,
                    string_end,
                    TOKEN_STRING) != 0)
            {
                goto cleanup;
            }

            i = string_end;
            logical_line_has_code = 1;
            continue;
        }

        /* Явное продолжение строки: backslash + newline. */
        if (symbol == '\\')
        {
            size_t next_nl = newline_length(
                text->symbols, i + 1, text->length);

            if (next_nl == 0)
            {
                goto cleanup;
            }

            i += 1 + next_nl;
            at_line_start = 1;
            explicit_continuation = 1;
            continue;
        }

        /* Идентификатор или ключевое слово Python. */
        if (is_identifier_start(symbol))
        {
            size_t start = i;

            while (i < text->length &&
                   is_identifier_char(text->symbols[i]))
            {
                i++;
            }

            if (append_source_token(
                    result,
                    text->symbols,
                    start,
                    i,
                    TOKEN_IDENTIFIER) != 0)
            {
                goto cleanup;
            }

            logical_line_has_code = 1;
            continue;
        }

        /* Числовой литерал, включая .5, 1.0, 1e-3, 0xFF, 2j. */
        if (is_ascii_digit(symbol) ||
            (symbol == '.' &&
             i + 1 < text->length &&
             is_ascii_digit(text->symbols[i + 1])))
        {
            size_t start = i;
            size_t number_end = 0;

            if (scan_number(
                    text->symbols,
                    text->length,
                    start,
                    &number_end) != 0)
            {
                goto cleanup;
            }

            if (append_source_token(
                    result,
                    text->symbols,
                    start,
                    number_end,
                    TOKEN_NUMBER) != 0)
            {
                goto cleanup;
            }

            i = number_end;
            logical_line_has_code = 1;
            continue;
        }

        /* Скобки: сохраняем тип, а не только глубину. */
        if (symbol == '(' || symbol == '[' || symbol == '{')
        {
            if (bracket_level >=
                sizeof(bracket_stack) / sizeof(bracket_stack[0]))
            {
                goto cleanup;
            }

            bracket_stack[bracket_level++] = symbol;

            if (append_source_token(
                    result, text->symbols, i, i + 1,
                    TOKEN_PUNCTUATION) != 0)
            {
                goto cleanup;
            }

            i++;
            logical_line_has_code = 1;
            continue;
        }

        if (symbol == ')' || symbol == ']' || symbol == '}')
        {
            if (bracket_level == 0)
            {
                goto cleanup;
            }

            uint32_t expected_open =
                symbol == ')' ? '(' :
                symbol == ']' ? '[' : '{';

            if (bracket_stack[bracket_level - 1] != expected_open)
            {
                goto cleanup;
            }

            bracket_level--;

            if (append_source_token(
                    result, text->symbols, i, i + 1,
                    TOKEN_PUNCTUATION) != 0)
            {
                goto cleanup;
            }

            i++;
            logical_line_has_code = 1;
            continue;
        }

        /* Запятая, двоеточие, точка, точка с запятой и т. п. */
        if (is_punctuation(symbol))
        {
            size_t punctuation_length = 1;

            if (symbol == '.' &&
                i + 2 < text->length &&
                text->symbols[i + 1] == '.' &&
                text->symbols[i + 2] == '.')
            {
                punctuation_length = 3;
            }

            if (append_source_token(
                    result,
                    text->symbols,
                    i,
                    i + punctuation_length,
                    TOKEN_PUNCTUATION) != 0)
            {
                goto cleanup;
            }

            i += punctuation_length;
            logical_line_has_code = 1;
            continue;
        }

        /* Операторы: выбираем самое длинное совпадение. */
        size_t op_length = 0;

        if (match_operator(
                text->symbols,
                text->length,
                i,
                &op_length))
        {
            if (append_source_token(
                    result,
                    text->symbols,
                    i,
                    i + op_length,
                    TOKEN_OPERATOR) != 0)
            {
                goto cleanup;
            }

            i += op_length;
            logical_line_has_code = 1;
            continue;
        }

        /* Неизвестный символ в поддерживаемом подмножестве. */
        goto cleanup;
    }

    if (bracket_level != 0 || explicit_continuation)
    {
        goto cleanup;
    }

    if (logical_line_has_code)
    {
        if (append_special_token(result, TOKEN_NEWLINE) != 0)
        {
            goto cleanup;
        }
    }

    while (indentation_level > 1)
    {
        indentation_level--;

        if (append_special_token(result, TOKEN_DEDENT) != 0)
        {
            goto cleanup;
        }
    }

    return 0;

cleanup:
    free_token_sequence(result);
    return -1;
}