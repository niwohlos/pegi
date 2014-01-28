#include <cerrno>
#include <cstdio>
#include <cstring>
#include <vector>

#include "tokenize.hpp"


int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "%s: no input files\n", argv[0]);
        return -1;
    }

    for (int i = 1; i < argc; i++)
    {
        FILE *fp = fopen(argv[i], "r");
        if (!fp)
        {
            fprintf(stderr, "%s: Could not open %s: %s\n", argv[0], argv[i], strerror(errno));
            return 1;
        }

        fseek(fp, 0, SEEK_END);
        off_t len = ftell(fp);
        rewind(fp);

        char *buf = new char[len + 1];
        fread(buf, 1, len, fp);
        buf[len] = 0;
        fclose(fp);

        std::vector<token *> token_list = tokenize(buf);
        if (token_list.empty())
        {
            fprintf(stderr, "%s: Could not tokenize %s\n", argv[0], argv[i]);
            return 1;
        }

        for (auto tok: token_list)
        {
            switch (tok->type)
            {
                case token::IDENTIFIER:  printf("Identifier:       %s\n", reinterpret_cast<identifier_token *>(tok)->value); break;
                case token::KEYWORD:     printf("Keyword:          %s\n", reinterpret_cast<keyword_token *>(tok)->value); break;
                case token::LIT_BOOL:    printf("Bool literal:     %s\n", reinterpret_cast<lit_bool_token *>(tok)->value ? "true" : "false"); break;
                case token::LIT_FLOAT:   printf("Float literal:    %Lg\n", reinterpret_cast<lit_float_token *>(tok)->value); break;
                case token::LIT_INTEGER:
                    if (reinterpret_cast<lit_integer_token *>(tok)->type & lit_integer_token::UNSIGNED)
                                         printf("Integer literal:  %llu\n", reinterpret_cast<lit_integer_token *>(tok)->value.u);
                    else
                                         printf("Integer literal:  %lli\n", reinterpret_cast<lit_integer_token *>(tok)->value.s);
                    break;
                case token::LIT_POINTER: printf("Pointer literal:  %p\n", reinterpret_cast<lit_pointer_token *>(tok)->value); break;
                case token::LIT_STRING:  printf("String literal:   %s\n", tok->content); break;
                case token::LIT_CHAR:    printf("Char literal:     %s (%u)\n", tok->content, reinterpret_cast<lit_char_token *>(tok)->value); break;
                case token::OPERATOR:    printf("Operator:         %s\n", reinterpret_cast<operator_token *>(tok)->value); break;
                default:                 printf("Unknown token %2i: %s\n", tok->type, tok->content);
            }

            delete tok;
        }

        delete[] buf;
    }

    return 0;
}
