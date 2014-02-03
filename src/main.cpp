#include <cerrno>
#include <cstdio>
#include <cstring>
#include <vector>

#include "error.hpp"
#include "parser.hpp"
#include "tokenize.hpp"


static void dump_token(token *tok, int spacing)
{
    switch (tok->type)
    {
        case token::IDENTIFIER:  printf("%-*s %s\n", spacing, "Identifier:", reinterpret_cast<identifier_token *>(tok)->value); break;
        case token::LIT_BOOL:    printf("%-*s %s\n", spacing, "Bool literal:", reinterpret_cast<lit_bool_token *>(tok)->value ? "true" : "false"); break;
        case token::LIT_FLOAT:   printf("%-*s %Lg\n", spacing, "Float literal:", reinterpret_cast<lit_float_token *>(tok)->value); break;
        case token::LIT_INTEGER:
            if (reinterpret_cast<lit_integer_token *>(tok)->type & lit_integer_token::UNSIGNED)
                                 printf("%-*s %llu\n", spacing, "Integer literal:", reinterpret_cast<lit_integer_token *>(tok)->value.u);
            else
                                 printf("%-*s %lli\n", spacing, "Integer literal:", reinterpret_cast<lit_integer_token *>(tok)->value.s);
            break;
        case token::LIT_POINTER: printf("%-*s %p\n", spacing, "Pointer literal:", reinterpret_cast<lit_pointer_token *>(tok)->value); break;
        case token::LIT_STRING:  printf("%-*s %s\n", spacing, "String literal:", tok->content); break;
        case token::LIT_CHAR:    printf("%-*s %s (%u)\n", spacing, "Char literal:", tok->content, reinterpret_cast<lit_char_token *>(tok)->value); break;
        case token::OPERATOR:    printf("%-*s %s\n", spacing, "Operator:", reinterpret_cast<operator_token *>(tok)->value); break;
        default:                 printf("Unknown token %2i: %s\n", tok->type, tok->content);
    }
}

static void dump_syntax_tree(syntax_tree_node *node, int indentation)
{
    printf("%*s%s", indentation, "", parser_type_names[node->type]);

    if (node->type != syntax_tree_node::TOKEN)
        putchar('\n');
    else
    {
        printf(": ");
        dump_token(node->ass_token, 0);
    }

    for (syntax_tree_node *c: node->children)
        dump_syntax_tree(c, indentation + 2);
}


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "%s: no input files\n", argv[0]);
        return 1;
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

        try
        {
            std::vector<token *> token_list = tokenize(buf);

            for (auto tok: token_list)
                dump_token(tok, 16);

            syntax_tree_node *root = build_syntax_tree(token_list);

            dump_syntax_tree(root, 0);

            for (auto tok: token_list)
                delete tok;

            delete[] buf;
        }
        catch (error *e)
        {
            e->emit(argv[0], argv[1], buf);
            return 1;
        }
    }

    return 0;
}
