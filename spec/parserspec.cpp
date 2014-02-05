#ifndef _PARSERSPEC_H
#define _PARSERSPEC_H

#include "error.hpp"
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

static char *dump_syntax_tree_to_buffer(syntax_tree_node *root)
{
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

    return output;
}


Describe(parser)
{
    Spec(ex1)
    {
        try
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
                "    (void)argv; // some wild comment & other 1337 stuff\n"
                "    a i;\n"
                "    b/**/*j = nullptr;\n"
                "    c k;\n"
                "    /*\n"
                "     * a multi-line comment\n"
                "     */ d<c> m;\n"
                "    printf(\"ohai wurld %g %g %llu %llu %Lg\", 3.25f, 0x2a.42p2, 42LLU, 42ull, -0.e-3l);\n"
                "    return 0;\n"
                "}\n"
            );

            syntax_tree_node *root = build_syntax_tree(token_list);
            char *output = dump_syntax_tree_to_buffer(root);
            delete root;
            for (token *t: token_list)
                delete t;

            Assert::That(output, Equals(
#include "parserspec-ex1-compare.h"
            ));

            delete[] output;
        }
        catch (error *err)
        {
            err->emit();
            Assert::Failure(err->msg);
            delete err;
        }
    }


    Spec(ex2)
    {
        try
        {
            std::vector<token *> token_list = tokenize(
                "template<typename T> struct foo\n"
                "{\n"
                "    private:\n"
                "        T bar;\n"
                "};\n"
                "foo<int> baz;\n"
            );

            syntax_tree_node *root = build_syntax_tree(token_list);
            char *output = dump_syntax_tree_to_buffer(root);
            delete root;
            for (token *t: token_list)
                delete t;

            Assert::That(output, Equals(
#include "parserspec-ex2-compare.h"
            ));

            delete[] output;
        }
        catch (error *err)
        {
            err->emit();
            Assert::Failure(err->msg);
            delete err;
        }
    }


    Spec(ex3)
    {
        try
        {
            std::vector<token *> token_list = tokenize(
                "template<typename T> class a\n"
                "{\n"
                "};\n"
                "template<typename T> class b\n"
                "{\n"
                "};\n"
                "auto foo() -> decltype(new a<b<int>>[42])\n"
                "{\n"
                "    int x((4 << 2) >> 3);\n"
                "    x >>= 1;\n"
                "    return new a<b<int>>[x];\n"
                "}\n"
            );

            syntax_tree_node *root = build_syntax_tree(token_list);
            char *output = dump_syntax_tree_to_buffer(root);
            delete root;
            for (token *t: token_list)
                delete t;

            Assert::That(output, Equals(
#include "parserspec-ex3-compare.h"
            ));

            delete[] output;
        }
        catch (error *err)
        {
            err->emit();
            Assert::Failure(err->msg);
            delete err;
        }
    }


    Spec(ex4)
    {
        try
        {
            std::vector<token *> token_list = tokenize(
                "template<bool B, class T = void>\n"
                "struct enable_if {};\n"
                "template<class T>\n"
                "struct enable_if<true, T> { typedef T type; };\n"
            );

            syntax_tree_node *root = build_syntax_tree(token_list);
            char *output = dump_syntax_tree_to_buffer(root);
            delete root;
            for (token *t: token_list)
                delete t;

            Assert::That(output, Equals(
#include "parserspec-ex4-compare.h"
            ));

            delete[] output;
        }
        catch (error *err)
        {
            err->emit();
            Assert::Failure(err->msg);
            delete err;
        }
    }


    Spec(ex5)
    {
        try
        {
            std::vector<token *> token_list = tokenize(
                "namespace foo\n"
                "{\n"
                "    class bar\n"
                "    {\n"
                "        public:\n"
                "            class baz\n"
                "            {\n"
                "            };\n"
                "    };\n"
                "}\n"
                "\n"
                "foo::bar::baz *x;\n"
            );

            syntax_tree_node *root = build_syntax_tree(token_list);
            char *output = dump_syntax_tree_to_buffer(root);
            delete root;
            for (token *t: token_list)
                delete t;

            Assert::That(output, Equals(
#include "parserspec-ex5-compare.h"
            ));

            delete[] output;
        }
        catch (error *err)
        {
            err->emit();
            Assert::Failure(err->msg);
            delete err;
        }
    }
};

#endif
