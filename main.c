
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "analysis.h"
#include "entropy.h"
#include "compression.h"
#include "file.h"
#include "csv.h"

// Описание одного теста.
typedef struct
{
    const char *name;
    const char *source;

    const TokenType *expected_tokens;
    size_t expected_count;

    int should_succeed;
} TestCase;


// Освобождение памяти, выделенной для тестовой последовательности.
static void free_test_tokens(TokenSequence *sequence)
{
    if (sequence == NULL)
    {
        return;
    }

    for (size_t i = 0; i < sequence->length; i++)
    {
        free(sequence->tokens[i].symbols);
    }

    free(sequence->tokens);

    sequence->tokens = NULL;
    sequence->length = 0;
    sequence->capacity = 0;
}


// Преобразование типа токена в строку для вывода.
static const char *token_type_name(TokenType type)
{
    switch (type)
    {
        case TOKEN_NEWLINE:
            return "NEWLINE";

        case TOKEN_INDENT:
            return "INDENT";

        case TOKEN_DEDENT:
            return "DEDENT";

        case TOKEN_IDENTIFIER:
            return "IDENTIFIER";

        case TOKEN_NUMBER:
            return "NUMBER";

        case TOKEN_STRING:
            return "STRING";

        case TOKEN_OPERATOR:
            return "OPERATOR";

        case TOKEN_PUNCTUATION:
            return "PUNCTUATION";

        default:
            return "UNKNOWN";
    }
}


// Запуск одного теста.
static int run_test(const TestCase *test)
{
    printf("\n========================================\n");
    printf("TEST: %s\n", test->name);
    printf("========================================\n");

    // Декодированное представление входной программы.
    DecodedText decoded = {0};

    // Преобразуем исходную строку в последовательность Unicode-символов.
    if (decode_text(
            (const unsigned char *)test->source,
            strlen(test->source),
            &decoded) != 0)
    {
        printf("ERROR: cannot decode source text\n");
        return -1;
    }

    // Пустая последовательность для результата токенизации.
    TokenSequence actual = {0};

    // Выполняем структурную токенизацию.
    int status = tokenize_python_source(&decoded, &actual);

    // Проверяем, ожидалась ли успешная обработка.
    if (test->should_succeed && status != 0)
    {
        printf("FAIL: expected successful tokenization\n");

        free_test_tokens(&actual);
        free_decoded_text(&decoded);

        return -1;
    }

    // Проверяем, ожидалась ли ошибка.
    if (!test->should_succeed && status == 0)
    {
        printf("FAIL: expected tokenization error\n");

        free_test_tokens(&actual);
        free_decoded_text(&decoded);

        return -1;
    }

    // Если тест проверял ошибку, а функция её вернула,
    // считаем такой результат успешным.
    if (!test->should_succeed)
    {
        printf("PASS: expected error received\n");

        free_test_tokens(&actual);
        free_decoded_text(&decoded);

        return 0;
    }

    // Выводим фактически полученные структурные токены.
    printf("Actual tokens:\n");

    for (size_t i = 0; i < actual.length; i++)
    {
        printf("  %s\n", token_type_name(actual.tokens[i].type));
    }

    // Проверяем количество токенов.
    if (actual.length != test->expected_count)
    {
        printf(
            "FAIL: expected %zu tokens, got %zu\n",
            test->expected_count,
            actual.length
        );

        free_test_tokens(&actual);
        free_decoded_text(&decoded);

        return -1;
    }

    // Проверяем тип каждого токена.
    for (size_t i = 0; i < test->expected_count; i++)
    {
        if (actual.tokens[i].type != test->expected_tokens[i])
        {
            printf(
                "FAIL: token %zu: expected %s, got %s\n",
                i,
                token_type_name(test->expected_tokens[i]),
                token_type_name(actual.tokens[i].type)
            );

            free_test_tokens(&actual);
            free_decoded_text(&decoded);

            return -1;
        }
    }

    printf("PASS: all structural tokens match\n");

    // Освобождаем выделенную память.
    free_test_tokens(&actual);
    free_decoded_text(&decoded);

    return 0;
}


int main(void)
{
    // Ожидаемые последовательности структурных токенов.

    const TokenType expected_1[] =
    {
        TOKEN_NEWLINE
    };

    const TokenType expected_2[] =
    {
        TOKEN_NEWLINE,
        TOKEN_INDENT,
        TOKEN_NEWLINE,
        TOKEN_DEDENT,
        TOKEN_NEWLINE
    };

    const TokenType expected_3[] =
    {
        TOKEN_NEWLINE,
        TOKEN_INDENT,
        TOKEN_NEWLINE,
        TOKEN_INDENT,
        TOKEN_NEWLINE,
        TOKEN_DEDENT,
        TOKEN_NEWLINE,
        TOKEN_DEDENT,
        TOKEN_NEWLINE
    };

    const TokenType expected_4[] =
    {
        TOKEN_NEWLINE
    };

    const TokenType expected_5[] =
    {
        TOKEN_NEWLINE,
        TOKEN_NEWLINE
    };

    const TokenType expected_6[] =
    {
        TOKEN_NEWLINE
    };

    const TokenType expected_7[] =
    {
        TOKEN_NEWLINE
    };

    const TokenType expected_8[] =
    {
        TOKEN_NEWLINE,
        TOKEN_NEWLINE
    };

    const TokenType expected_9[] =
    {
        TOKEN_NEWLINE,
        TOKEN_NEWLINE
    };

    const TokenType expected_10[] =
    {
        TOKEN_NEWLINE
    };


    // Набор входных Python-программ.
    const TestCase tests[] =
    {
        {
            "Simple line",
            "x = 10\n",
            expected_1,
            sizeof(expected_1) / sizeof(expected_1[0]),
            1
        },

        {
            "One indentation level",
            "if x:\n"
            "    y = 1\n"
            "z = 2\n",
            expected_2,
            sizeof(expected_2) / sizeof(expected_2[0]),
            1
        },

        {
            "Nested indentation levels",
            "if x:\n"
            "    if y:\n"
            "        z = 1\n"
            "    q = 2\n"
            "r = 3\n",
            expected_3,
            sizeof(expected_3) / sizeof(expected_3[0]),
            1
        },

        {
            "Empty lines and comments",
            "# first comment\n"
            "\n"
            "x = 1  # comment after code\n"
            "\n"
            "# last comment\n",
            expected_4,
            sizeof(expected_4) / sizeof(expected_4[0]),
            1
        },

        {
            "Line continuation inside brackets",
            "result = (\n"
            "    10 +\n"
            "    20\n"
            ")\n"
            "x = 1\n",
            expected_5,
            sizeof(expected_5) / sizeof(expected_5[0]),
            1
        },

        {
            "Explicit line continuation",
            "result = 10 + \\\n"
            "    20\n",
            expected_6,
            sizeof(expected_6) / sizeof(expected_6[0]),
            1
        },

        {
            "No final newline",
            "x = 10",
            expected_7,
            sizeof(expected_7) / sizeof(expected_7[0]),
            1
        },

        {
            "CRLF line endings",
            "x = 1\r\n"
            "y = 2\r\n",
            expected_8,
            sizeof(expected_8) / sizeof(expected_8[0]),
            1
        },

        {
            "CR line endings",
            "x = 1\r"
            "y = 2\r",
            expected_9,
            sizeof(expected_9) / sizeof(expected_9[0]),
            1
        },

        {
            "Empty source",
            "",
            expected_10,
            0,
            1
        },

        {
            "Invalid indentation",
            "if x:\n"
            "    y = 1\n"
            "  z = 2\n",
            NULL,
            0,
            0
        },

        {
            "Unclosed bracket",
            "x = (\n"
            "    10 + 20\n",
            NULL,
            0,
            0
        }
    };

    // Вычисляем количество тестов.
    size_t test_count = sizeof(tests) / sizeof(tests[0]);

    size_t passed = 0;
    size_t failed = 0;

    printf("STRUCTURAL TOKENIZER TESTS\n");

    // Последовательно запускаем все тесты.
    for (size_t i = 0; i < test_count; i++)
    {
        if (run_test(&tests[i]) == 0)
        {
            passed++;
        }
        else
        {
            failed++;
        }
    }

    // Итоговая статистика.
    printf("\n========================================\n");
    printf("TEST RESULTS\n");
    printf("========================================\n");

    printf("Total:  %zu\n", test_count);
    printf("Passed: %zu\n", passed);
    printf("Failed: %zu\n", failed);

    if (failed == 0)
    {
        printf("\nALL TESTS PASSED\n");
        return 0;
    }

    printf("\nSOME TESTS FAILED\n");
    return 1;
}
