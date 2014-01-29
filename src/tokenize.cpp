#include <cassert>
#include <cstdio>
#include <cstring>
#include <ctype.h>
#include <vector>

#include "tokenize.hpp"


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
        base = 0x10;

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

    if (is_unsigned)
    {
        value.u = 0;

        while (c != suffix)
        {
            char d = tolower(*(c++));
            value.u *= base;
            value.u += (d > '9') ? (d - 'a' + 10) : (d - '0');
        }

        subtype = UNSIGNED;
    }
    else
    {
        value.s = 0;

        while (c != suffix)
        {
            char d = tolower(*(c++));
            value.s *= base;
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
        else if (isdigit(*str))
        {
            if (*str != '0')
                do
                    str++;
                while (isdigit(*str));
            else if (tolower(*(++str)) != 'x')
                while (isodigit(*str))
                    str++;
            else
                do
                    str++;
                while (isxdigit(*str));

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

            char *content = new char[str - start + 1];
            memcpy(content, start, str - start);
            content[str - start] = 0;

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
                        throw_error(str, original_start, "Invalid escape sequence");
                        for (token *_: ret) { delete _; }
                        return std::vector<token *>();
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
                throw_error(str, original_start, "Invalid escape sequence");
                for (token *_: ret) { delete _; }
                return std::vector<token *>();
            }

            if (*str != '\'')
            {
                throw_error(str, original_start, "End of character sequence expected");
                for (token *_: ret) { delete _; }
                return std::vector<token *>();
            }

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
            {
                throw_error(str, original_start, "Could not parse character");
                for (token *_: ret) { delete _; }
                return std::vector<token *>();
            }
        }

        if (t)
            ret.push_back(t);

    }

    return ret;
}
