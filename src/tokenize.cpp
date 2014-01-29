#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctype.h>
#include <vector>

#include "tokenize.hpp"
#include "translation_limits.hpp"


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


// Strictly speaking, types are also keywords. But consider them just literals
// for now. Also, const/volatile.
// false/nullptr/true are technically keywords, but here considered literals.
// this is technically a keyword, but here considered an identifier.
static const char *keywords[] = {
    "alignas", "alignof", "asm", "break", "case", "catch", "class", "constexpr",
    "const_cast", "continue", "decltype", "default", "delete", "do",
    "dynamic_cast", "else", "enum", "explicit", "export", "extern", "for",
    "friend", "goto", "if", "inline", "mutable", "namespace", "new", "noexcept",
    "operator", "private", "protected", "public", "register",
    "reinterpret_cast", "return", "sizeof", "static", "static_assert",
    "static_cast", "struct", "switch", "template", "thread_local", "throw",
    "try", "typedef", "typeid", "typename", "union", "using", "virtual", "while"
};

static inline bool iskeyword(const char *str)
{
    for (const char *keyword: keywords)
        if (!strcmp(str, keyword))
            return true;

    return false;
}


// preprocessing-op-or-punc (sorted by reverse length)
static const char *poops[] = {
    "delete",

    "and_eq", "bitand", "not_eq", "xor_eq",

    "bitor", "compl", "or_eq",

    "%:%:",

    "...", "new", ">>=", "<<=", "->*", "and", "not", "xor",

    "##", "<:", ":>", "<%", "%>", "%:", "::", ".*", "+=", "-=", "*=", "/=",
    "%=", "^=", "&=", "|=", "<<", ">>", "==", "!=", "<=", ">=", "&&", "||",
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
                char next = (*c)[strlen(poop) - 1];
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

keyword_token::keyword_token(char *c):
    token(token::KEYWORD, c)
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
                throw 0;
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
                    throw 0;
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
        throw 0;
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
    {
        fprintf(stderr, "Expected an escape sequence\n");
        throw 0;
    }

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

    fprintf(stderr, "Invalid escape sequence starting with '%c'\n", **seq);
    throw 0;
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


static void throw_error(const char *input, const char *input_start, const char *msg)
{
    const char *line_start = input;
    while ((line_start >= input_start) && (*line_start != '\n'))
        line_start--;
    line_start++;

    const char *line_end = input;
    while (*line_end && (*line_end != '\n'))
        line_end++;

    char line[line_end - line_start + 1];
    memcpy(line, line_start, line_end - line_start);
    line[line_end - line_start] = 0;

    fprintf(stderr, "%s\n", line);
    fprintf(stderr, "%*s^\n", static_cast<int>(input - line_start), "");
    fprintf(stderr, "%s\n", msg);
}


std::vector<token *> tokenize(const char *str)
{
    const char *original_start = str;
    std::vector<token *> ret;

    try
    {

    while (*str)
    {
        while (*str && isspace(*str))
            str++;

        if (!*str)
            break;

        token *t = NULL;
        const char *start = str;


        if ((str[0] == '/') && (str[1] == '/'))
            while (*str && (*str != '\n'))
                str++;
        else if ((str[0] == '/') && (str[1] == '*'))
        {
            str += 2;
            while (*str && ((str[-2] != '*') || (str[-1] != '/')))
                str++;
        }
        else if (isidentifiernondigit(*str))
        {
            do
                str++;
            while (isidentifiernondigit(*str) || isdigit(*str));

            char *content = new char[str - start + 1];
            memcpy(content, start, str - start);
            content[str - start] = 0;

            if (iskeyword(content))
                t = new keyword_token(content);
            else if (!strcmp(content, "false") || !strcmp(content, "true"))
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
                do
                    str++;
                while (isdigit(*str));
            else if (!is_float && (tolower(*(++str)) != 'x'))
                while (isodigit(*str))
                    str++;
            else
            {
                is_hex = true;
                do
                    str++;
                while (isxdigit(*str));
            }

            if (*str == '.')
            {
                is_float = true;

                do
                    str++;
                while ((is_hex ? isxdigit : isdigit)(*str));
            }

            if (is_float &&
                ((is_hex && (tolower(*str) == 'p')) ||
                (!is_hex && (tolower(*str) == 'e'))))
            {
                str++;
                if ((*str == '-') || (*str == '+'))
                    str++;

                if (!isdigit(*str))
                    throw "Expected a decimal digit as floating point exponent";

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
                    catch (int __)
                    {
                        throw "Invalid escape sequence";
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
            catch (int __)
            {
                throw "Invalid escape sequence";
            }

            if (*str != '\'')
                throw "End of character sequence expected";

            str++;

            char *content = new char[str - start + 1];
            memcpy(content, start, str - start);
            content[str - start] = 0;

            t = new lit_char_token(content);
        }
        else
        {
            const char *tmp = str;
            if (ispoop(&tmp))
            {
                str = tmp;

                char *content = new char[str - start + 1];
                memcpy(content, start, str - start);
                content[str - start] = 0;

                t = new operator_token(content);
            }
            else
                throw "Could not parse character";
        }

        if (t)
            ret.push_back(t);

    }

    }
    catch (const char *msg)
    {
        throw_error(str, original_start, msg);
        for (token *_: ret) { delete _; }
        return std::vector<token *>();
    }

    return ret;
}
