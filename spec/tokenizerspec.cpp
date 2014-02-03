#ifndef _TOKENIZERSPEC_H
#define _TOKENIZERSPEC_H

#include "error.hpp"
#include "tokenize.hpp"
#include "igloo/igloo.h"

#include <cstring>
#include <unistd.h>
#include <vector>


using namespace igloo;

static int dump_token(FILE *fp, token *tok, int spacing)
{
    switch (tok->type)
    {
        case token::IDENTIFIER:  return fprintf(fp, "%-*s (%2i:%2i) %s\n", spacing, "Identifier:", tok->line, tok->column, reinterpret_cast<identifier_token *>(tok)->value); break;
        case token::LIT_BOOL:    return fprintf(fp, "%-*s (%2i:%2i) %s\n", spacing, "Bool literal:", tok->line, tok->column, reinterpret_cast<lit_bool_token *>(tok)->value ? "true" : "false"); break;
        case token::LIT_FLOAT:   return fprintf(fp, "%-*s (%2i:%2i) %Lg\n", spacing, "Float literal:", tok->line, tok->column, reinterpret_cast<lit_float_token *>(tok)->value); break;
        case token::LIT_INTEGER:
            if (reinterpret_cast<lit_integer_token *>(tok)->type & lit_integer_token::UNSIGNED)
                                 return fprintf(fp, "%-*s (%2i:%2i) %llu\n", spacing, "Integer literal:", tok->line, tok->column, reinterpret_cast<lit_integer_token *>(tok)->value.u);
            else
                                 return fprintf(fp, "%-*s (%2i:%2i) %lli\n", spacing, "Integer literal:", tok->line, tok->column, reinterpret_cast<lit_integer_token *>(tok)->value.s);
            break;
        case token::LIT_POINTER: return fprintf(fp, "%-*s (%2i:%2i) %p\n", spacing, "Pointer literal:", tok->line, tok->column, reinterpret_cast<lit_pointer_token *>(tok)->value); break;
        case token::LIT_STRING:  return fprintf(fp, "%-*s (%2i:%2i) %s\n", spacing, "String literal:", tok->line, tok->column, tok->content); break;
        case token::LIT_CHAR:    return fprintf(fp, "%-*s (%2i:%2i) %s (%u)\n", spacing, "Char literal:", tok->line, tok->column, tok->content, reinterpret_cast<lit_char_token *>(tok)->value); break;
        case token::OPERATOR:    return fprintf(fp, "%-*s (%2i:%2i) %s\n", spacing, "Operator:", tok->line, tok->column, reinterpret_cast<operator_token *>(tok)->value); break;
        default:                 return fprintf(fp, "Unknown token %2i: (%2i:%2i) %s\n", tok->type, tok->line, tok->column, tok->content);
    }
}


Describe(tokenizer)
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

        int fds[2];
        pipe(fds);

        int sz = 0;

        FILE *ifp = fdopen(fds[1], "w");
        for (auto tok: token_list)
            sz += dump_token(ifp, tok, 16);
        fclose(ifp);

        char *output = new char[sz + 1];
        read(fds[0], output, sz);
        output[sz] = 0;

        close(fds[0]);
        close(fds[1]);


        Assert::That(output, Equals(
            "Identifier:      ( 1: 1) extern\n"
            "Identifier:      ( 1: 8) int\n"
            "Identifier:      ( 1:12) printf\n"
            "Operator:        ( 1:18) (\n"
            "Identifier:      ( 1:19) const\n"
            "Identifier:      ( 1:25) char\n"
            "Operator:        ( 1:30) *\n"
            "Identifier:      ( 1:31) format\n"
            "Operator:        ( 1:37) ,\n"
            "Operator:        ( 1:39) ...\n"
            "Operator:        ( 1:42) )\n"
            "Operator:        ( 1:43) ;\n"
            "Identifier:      ( 2: 1) class\n"
            "Identifier:      ( 2: 7) b\n"
            "Operator:        ( 2: 8) ;\n"
            "Identifier:      ( 3: 1) class\n"
            "Identifier:      ( 3: 7) c\n"
            "Operator:        ( 4: 1) {\n"
            "Operator:        ( 5: 1) }\n"
            "Operator:        ( 5: 2) ;\n"
            "Identifier:      ( 6: 1) template\n"
            "Operator:        ( 6: 9) <\n"
            "Identifier:      ( 6:10) typename\n"
            "Identifier:      ( 6:19) T\n"
            "Operator:        ( 6:20) >\n"
            "Identifier:      ( 6:22) class\n"
            "Identifier:      ( 6:28) d\n"
            "Operator:        ( 7: 1) {\n"
            "Operator:        ( 8: 1) }\n"
            "Operator:        ( 8: 2) ;\n"
            "Identifier:      ( 9: 1) int\n"
            "Identifier:      ( 9: 5) main\n"
            "Operator:        ( 9: 9) (\n"
            "Identifier:      ( 9:10) int\n"
            "Identifier:      ( 9:14) argc\n"
            "Operator:        ( 9:18) ,\n"
            "Identifier:      ( 9:20) char\n"
            "Operator:        ( 9:25) *\n"
            "Identifier:      ( 9:26) argv\n"
            "Operator:        ( 9:30) [\n"
            "Operator:        ( 9:31) ]\n"
            "Operator:        ( 9:32) )\n"
            "Operator:        (10: 1) {\n"
            "Identifier:      (11: 5) typedef\n"
            "Identifier:      (11:13) int\n"
            "Identifier:      (11:17) a\n"
            "Operator:        (11:18) ;\n"
            "Operator:        (12: 5) (\n"
            "Identifier:      (12: 6) void\n"
            "Operator:        (12:10) )\n"
            "Identifier:      (12:11) argc\n"
            "Operator:        (12:15) ;\n"
            "Operator:        (13: 5) (\n"
            "Identifier:      (13: 6) void\n"
            "Operator:        (13:10) )\n"
            "Identifier:      (13:11) argv\n"
            "Operator:        (13:15) ;\n"
            "Identifier:      (14: 5) a\n"
            "Identifier:      (14: 7) i\n"
            "Operator:        (14: 8) ;\n"
            "Identifier:      (15: 5) b\n"
            "Operator:        (15:10) *\n"
            "Identifier:      (15:11) j\n"
            "Operator:        (15:13) =\n"
            "Pointer literal: (15:15) (nil)\n"
            "Operator:        (15:22) ;\n"
            "Identifier:      (16: 5) c\n"
            "Identifier:      (16: 7) k\n"
            "Operator:        (16: 8) ;\n"
            "Identifier:      (19: 9) d\n"
            "Operator:        (19:10) <\n"
            "Identifier:      (19:11) c\n"
            "Operator:        (19:12) >\n"
            "Identifier:      (19:14) m\n"
            "Operator:        (19:15) ;\n"
            "Identifier:      (20: 5) printf\n"
            "Operator:        (20:11) (\n"
            "String literal:  (20:12) \"ohai wurld %g %g %llu %llu %Lg\"\n"
            "Operator:        (20:44) ,\n"
            "Float literal:   (20:46) 3.25\n"
            "Operator:        (20:51) ,\n"
            "Float literal:   (20:53) 169.031\n"
            "Operator:        (20:62) ,\n"
            "Integer literal: (20:64) 42\n"
            "Operator:        (20:69) ,\n"
            "Integer literal: (20:71) 42\n"
            "Operator:        (20:76) ,\n"
            "Operator:        (20:78) -\n"
            "Float literal:   (20:79) 0\n"
            "Operator:        (20:85) )\n"
            "Operator:        (20:86) ;\n"
            "Identifier:      (21: 5) return\n"
            "Integer literal: (21:12) 0\n"
            "Operator:        (21:13) ;\n"
            "Operator:        (22: 1) }\n"
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
