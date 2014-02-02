#ifndef _PARSERSPEC_H
#define _PARSERSPEC_H

#include "parser.hpp"
#include "tokenize.hpp"
#include "igloo/igloo.h"

#include <cstring>
#include <unistd.h>
#include <vector>


using namespace igloo;

static int dump_syntax_tree(FILE *fp, syntax_tree_node *node, int indentation)
{
    int ret = fprintf(fp, "%*s%s", indentation, "", parser_type_names[node->type]);

    if (node->type != syntax_tree_node::TOKEN)
    {
        fputc('\n', fp);
        ret++;
    }
    else
    {
        ret += fprintf(fp, ": ");
        ret += dump_token(fp, node->ass_token, 0);
    }

    for (syntax_tree_node *c: node->children)
        ret += dump_syntax_tree(fp, c, indentation + 2);

    return ret;
}


Describe(parser)
{
    Spec(ex1)
    {
        std::vector<token *> token_list = tokenize(
            "extern int printf(const char *format, ...);\n"
            "class b;\n"
            "class c\n"
            "{\n"
            "};\n"
            "template<typename T> class d\n"
            "{\n"
            "};\n"
            "int main(int argc, char *argv[])\n"
            "{\n"
            "    typedef int a;\n"
            "    (void)argc;\n"
            "    (void)argv;\n"
            "    a i;\n"
            "    b *j = nullptr;\n"
            "    c k;\n"
            "    d<c> m;\n"
            "    printf(\"ohai wurld %g %g %llu %llu %Lg\", 3.25f, 0x2a.42p2, 42LLU, 42ull, -0.e-3l);\n"
            "    return 0;\n"
            "}\n"
        );

        syntax_tree_node *root = build_syntax_tree(token_list);

        int fds[2];
        pipe(fds);

        FILE *ifp = fdopen(fds[1], "w");
        int sz = dump_syntax_tree(ifp, root, 0);
        fclose(ifp);

        char *output = new char[sz + 1];
        read(fds[0], output, sz);
        output[sz] = 0;

        close(fds[0]);
        close(fds[1]);


        Assert::That(output, Equals(
#include "parserspec-ex1-compare.h"
        ));

        delete[] output;
    }
};

#endif
