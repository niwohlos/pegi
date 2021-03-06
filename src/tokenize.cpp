#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctype.h>
#include <vector>

#include "error.hpp"
#include "format.hpp"
#include "tokenize.hpp"
#include "translation_limits.hpp"

#ifdef __MACH__
#define exp10l(x) __exp10(x)
#endif

static inline bool isnondigit(char c)
{
    return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || (c == '_');
}


static inline bool isidentifiernondigit(char c)
{
    return isnondigit(c);
}


static inline bool isodigit(char c)
{
    return (c >= '0') && (c <= '7');
}


// preprocessing-op-or-punc (sorted by reverse length)
static const char *const poops[] = {
    "delete",

    "and_eq", "bitand", "not_eq", "xor_eq",

    "bitor", "compl", "or_eq",

    "%:%:",

    // Due to the template<foo<bar>> stuff, '>>' may not be part of other
    // operators
    "...", "new", /*">>=",*/ "<<=", "->*", "and", "not", "xor",

    "##", "<:", ":>", "<%", "%>", "%:", "::", ".*", "+=", "-=", "*=", "/=",
    "%=", "^=", "&=", "|=", "<<", /*">>",*/ "==", "!=", "<=", ">=", "&&", "||",
    "++", "--", "->", "or",

    "{", "}", "[", "]", "#", "(", ")", ";", ":", "?", ".", "+", "-", "*", "/",
    "%", "^", "&", "|", "~", "!", "=", "<", ">", ","
};

static inline bool ispoop(const char **c)
{
    for (const char *poop: poops)
    {
        if (!strncmp(*c, poop, strlen(poop)))
        {
            if (isidentifiernondigit(poop[strlen(poop) - 1]))
            {
                char next = (*c)[strlen(poop)];
                if (isidentifiernondigit(next) || isdigit(next))
                    continue;
            }

            *c += strlen(poop);

            return true;
        }
    }

    return false;
}


identifier_token::identifier_token(char *c):
    token(token::IDENTIFIER, c)
{
    value = c;
}

lit_integer_token::lit_integer_token(char *c):
    token(token::LIT_INTEGER, c)
{
    int base;

    if (c[0] != '0')
        base = 10;
    else if (tolower(c[1]) != 'x')
        base = 010;
    else
    {
        base = 0x10;
        c += 2;
    }

    char *suffix = c;
    while (*suffix && (tolower(*suffix) != 'u') && (tolower(*suffix) != 'l'))
        suffix++;

    bool is_unsigned = false;
    int l_counter = 0;

    for (int i = 0; suffix[i]; i++)
        if (tolower(suffix[i] == 'u'))
            is_unsigned = true;
        else
            l_counter++; // XXX: assert(tolower(suffix[i] == 'l'))

retry:
    if (is_unsigned)
    {
        value.u = 0;

        while (c != suffix)
        {
            unsigned long long new_value = value.u * base;
            if (new_value / base != value.u)
                throw format("Unsigned integer literal is too big");
            value.u = new_value;

            char d = tolower(*(c++));
            value.u += (d > '9') ? (d - 'a' + 10) : (d - '0');
        }

        subtype = UNSIGNED;
    }
    else
    {
        value.s = 0;
        char *restart_c = c;

        while (c != suffix)
        {
            long long new_value = value.s * base;
            if (new_value / base != value.s)
            {
                if (base == 10)
                {
                    // lol no unsigned for you (see below)
                    throw format("Signed decimal integer literal is too big");
                }

                is_unsigned = true;
                c = restart_c;
                // I'm so very sorry
                goto retry;
            }
            value.s = new_value;

            char d = tolower(*(c++));
            value.s += (d > '9') ? (d - 'a' + 10) : (d - '0');
        }

        subtype = SIGNED;
    }

    switch (l_counter)
    {
        case 0: subtype = static_cast<integer_type>(subtype | INT      ); break;
        case 1: subtype = static_cast<integer_type>(subtype | LONG     ); break;
        case 2: subtype = static_cast<integer_type>(subtype | LONG_LONG); break;
    }

    // promotions for everynyan! (lol this is crazy see table 6 in 2.14.2 for reference)
    if (base == 10)
    {
        // keep signedness for decimal constants (lol why)
        if ((subtype == SIGNED_INT) && ((value.s < TL_INT_MIN) || (value.s > TL_INT_MAX)))
            subtype = SIGNED_LONG;
        if ((subtype == UNSIGNED_INT) && (value.u > TL_UINT_MAX))
            subtype = UNSIGNED_LONG;
        if ((subtype == SIGNED_LONG) && ((value.s < TL_LONG_MIN) || (value.s < TL_LONG_MAX)))
            subtype = SIGNED_LONG_LONG;
        if ((subtype == UNSIGNED_LONG) && (value.u > TL_ULONG_MAX))
            subtype = UNSIGNED_LONG_LONG;
    }
    else
    {
        // this seems more logical (BUT WHY WOULD YOU DO DECIMAL DIFFERENTLY):
        // if u/U, always unsigned; if signed, make unsigned, if necessary
        if ((subtype == SIGNED_INT) && (value.s < TL_INT_MIN))
            subtype = SIGNED_LONG;
        if ((subtype == SIGNED_INT) && (value.s > TL_INT_MAX))
            subtype = UNSIGNED_INT;
        if ((subtype == UNSIGNED_INT) && (value.u > TL_UINT_MAX))
            subtype = is_unsigned ? UNSIGNED_LONG : SIGNED_LONG;
        if ((subtype == SIGNED_LONG) && (value.s < TL_LONG_MIN))
            subtype = SIGNED_LONG_LONG;
        if ((subtype == SIGNED_LONG) && (value.s > TL_LONG_MAX))
            subtype = UNSIGNED_LONG;
        if ((subtype == UNSIGNED_LONG) && (value.u > TL_ULONG_MAX))
            subtype = is_unsigned ? UNSIGNED_LONG_LONG : SIGNED_LONG_LONG;

        // and whether it's signed or unsigned long long, that's already been decided
    }
}

lit_float_token::lit_float_token(char *c):
    token(token::LIT_FLOAT, c)
{
    bool is_hex = (c[0] == '0') && (tolower(c[1] == 'x'));

    if (is_hex)
        c += 2;

    // The maximum mantissa (80 bit) is 63 bit, so we're fine with 64 bit.
    uint64_t int_part = 0;
    int leftover_exponent = 0;
    int base = is_hex ? 0x10 : 10;

    while (*c != '.')
    {
        uint64_t mult = int_part * base;
        if (mult / base != int_part)
        {
            // Just leave the rest, we can't store it in the float anyway, so who cares.
            leftover_exponent++;
            continue;
        }

        int_part = mult;
        int_part += (*c > '9') ? (tolower(*c) - 'a' + 10) : (*c - '0');
        c++;
    }

    c++;

    uint64_t frac_part = 0;
    int frac_exponent = 0;

    while ((is_hex ? isxdigit : isdigit)(*c))
    {
        uint64_t mult = frac_part * base;
        if (mult / base != frac_part)
            continue;

        frac_part = mult;
        frac_part += (*c > '9') ? (tolower(*c) - 'a' + 10) : (*c - '0');
        frac_exponent--;
        c++;
    }

    int number_exponent = 0;

    if ((is_hex && (tolower(*c) == 'p')) ||
       (!is_hex && (tolower(*c) == 'e')))
    {
        bool negative = (*++c == '-');
        if ((*c == '-') || (*c == '+'))
            c++;

        while (isdigit(*c))
        {
            number_exponent *= 10;
            number_exponent += *c - '0';
            c++;
        }

        if (negative)
            number_exponent *= -1;
    }

    // I'm frigging lazy (for hex, it'd probably be relatively easy, but for dec...)
    if (is_hex)
        value = (int_part * exp2l(leftover_exponent * 4) + frac_part * exp2l(frac_exponent * 4)) * exp2l(number_exponent /* this exponent is binary */);
    else
        value = (int_part * exp10l(leftover_exponent) + frac_part * exp10l(frac_exponent)) * exp10l(number_exponent);

    if (tolower(*c) == 'f')
        subtype = FLOAT;
    else if (tolower(*c) == 'l')
        subtype = LONG_DOUBLE;
    else if (!*c)
        subtype = DOUBLE;
    else
        throw format("Unknown floating point literal suffix %s", c);
}

lit_bool_token::lit_bool_token(char *c):
    token(token::LIT_BOOL, c)
{
    value = !strcmp(c, "true");
}

lit_pointer_token::lit_pointer_token(char *c):
    token(token::LIT_POINTER, c)
{
    assert(!strcmp(c, "nullptr"));
    value = NULL;
}

static inline char eseq(const char **seq)
{
    if (!**seq)
        throw format("Escape sequence is empty");

    if (**seq == 'x')
    {
        char val = 0;
        (*seq)++;
        while (**seq && isxdigit(**seq))
        {
            val *= 0x10;
            val += (**seq > '9') ? (tolower(**seq) - 'a' + 10) : (**seq - '0');
            (*seq)++;
        }
        return val;
    }
    else if (isodigit(**seq))
    {
        char val = 0;
        (*seq)++;
        while (**seq && isodigit(**seq))
        {
            val *= 010;
            val += **seq - '0';
            (*seq)++;
        }
        return val;
    }

    switch (*((*seq)++))
    {
        case '\'': return '\'';
        case '"':  return '\"';
        case '?':  return '\?';
        case '\\': return '\\';
        case 'a':  return '\a';
        case 'b':  return '\b';
        case 'f':  return '\f';
        case 'n':  return '\n';
        case 'r':  return '\r';
        case 't':  return '\t';
        case 'v':  return '\v';
    }

    throw format("Invalid escape sequence starting with '%c'", **seq);
}

lit_string_token::lit_string_token(char *c):
    token(token::LIT_STRING, c)
{
    char *tmp = ++c;
    length = 0;

    while (*tmp != '"')
    {
        if (*(tmp++) == '\\')
            eseq(const_cast<const char **>(&tmp));

        length++;
    }

    value = new char[length];
    char *vp = value;

    while (*c != '"')
    {
        if (*(c++) == '\\')
            *(vp++) = eseq(const_cast<const char **>(&c));
    }
}

lit_char_token::lit_char_token(char *c):
    token(token::LIT_CHAR, c)
{
    if (*++c != '\\')
        value = *c;
    else
    {
        c++;
        value = eseq(const_cast<const char **>(&c));
    }
}

operator_token::operator_token(char *c):
    token(token::OPERATOR, c)
{
    value = c;
}


std::vector<token *> tokenize(const char *str)
{
    std::vector<token *> ret;
    const char *line_start = str;
    int line = 1;

    try
    {
        while (*str)
        {
            while (*str && isspace(*str))
            {
                if (*(str++) == '\n')
                {
                    line++;
                    line_start = str;
                }
            }

            if (!*str)
                break;

            token *t = NULL;
            const char *start = str, *tmp = str;
            int column = str - line_start + 1;


            if ((str[0] == '/') && (str[1] == '/'))
                while (*str && (*str != '\n'))
                    str++;
            else if ((str[0] == '/') && (str[1] == '*'))
            {
                str += 2;
                while (*str && ((str[-2] != '*') || (str[-1] != '/')))
                {
                    if (*(str++) == '\n')
                    {
                        line++;
                        line_start = str;
                    }
                }
            }
            else if (ispoop(&tmp))
            {
                str = tmp;

                char *content = new char[str - start + 1];
                memcpy(content, start, str - start);
                content[str - start] = 0;

                t = new operator_token(content);
            }
            else if (isidentifiernondigit(*str))
            {
                do
                    str++;
                while (isidentifiernondigit(*str) || isdigit(*str));

                char *content = new char[str - start + 1];
                memcpy(content, start, str - start);
                content[str - start] = 0;

                if (!strcmp(content, "false") || !strcmp(content, "true"))
                    t = new lit_bool_token(content);
                else if (!strcmp(content, "nullptr"))
                    t = new lit_pointer_token(content);
                else
                    t = new identifier_token(content);
            }
            else if (isdigit(*str) || ((str[0] == '.') && isdigit(str[1])))
            {
                bool is_float = (*str == '.');
                bool is_hex = false;

                if (*str != '0')
                    while (isdigit(*++str));
                else if (!is_float && (tolower(*(++str)) != 'x'))
                    while (isodigit(*str))
                        str++;
                else
                {
                    is_hex = true;
                    while (isxdigit(*++str));
                }

                if (*str == '.')
                {
                    is_float = true;
                    while ((is_hex ? isxdigit : isdigit)(*++str));
                }

                if (is_float &&
                    ((is_hex && (tolower(*str) == 'p')) ||
                    (!is_hex && (tolower(*str) == 'e'))))
                {
                    str++;
                    if ((*str == '-') || (*str == '+'))
                        str++;

                    if (!isdigit(*str))
                        throw format("Expected a decimal digit as floating point exponent");

                    while (isdigit(*str))
                        str++;
                }

                if (is_float)
                {
                    if ((tolower(*str) == 'f') || (tolower(*str) == 'l'))
                        str++;
                }
                else
                {
                    bool had_unsigned = false;

                    if (tolower(*str) == 'u')
                    {
                        had_unsigned = true;
                        str++;
                    }

                    if (tolower(*str) == 'l')
                        if (tolower(*++str) == 'l')
                            str++;

                    if ((tolower(*str) == 'u') && !had_unsigned)
                        str++;
                }

                char *content = new char[str - start + 1];
                memcpy(content, start, str - start);
                content[str - start] = 0;

                if (is_float)
                    t = new lit_float_token(content);
                else
                    t = new lit_integer_token(content);
            }
            else if (*str == '"')
            {
                str++;
                while (*str && (*str != '"'))
                {
                    if ((*(str++) == '\\'))
                    {
                        try
                        {
                            eseq(&str);
                        }
                        catch (char *msg)
                        {
                            char *reformatted = format("Invalid escape sequence: %s", msg);
                            delete msg;
                            throw reformatted;
                        }
                    }
                }
                str++;

                char *content = new char[str - start + 1];
                memcpy(content, start, str - start);
                content[str - start] = 0;

                t = new lit_string_token(content);
            }
            else if (*str == '\'')
            {
                try
                {
                    str++;
                    if (*(str++) == '\\')
                        eseq(&str);
                }
                catch (char *msg)
                {
                    char *reformatted = format("Invalid escape sequence: %s", msg);
                    delete msg;
                    throw reformatted;
                }

                if (*str != '\'')
                    throw format("End of character sequence expected");

                str++;

                char *content = new char[str - start + 1];
                memcpy(content, start, str - start);
                content[str - start] = 0;

                t = new lit_char_token(content);
            }
            else
                throw format("Could not parse character");

            if (t)
            {
                t->line = line;
                t->column = column;
                ret.push_back(t);
            }
        }
    }
    catch (char *msg)
    {
        for (token *_: ret) { delete _; }
        throw new error(line, str - line_start + 1, msg);
    }

    return ret;
}
