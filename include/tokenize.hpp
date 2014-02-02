#ifndef TOKENIZE_HPP
#define TOKENIZE_HPP

#include <cstddef>

#include <type_traits>
#include <vector>


class token
{
    public:
        enum token_type
        {
            IDENTIFIER,
            LIT_INTEGER,
            LIT_FLOAT,
            LIT_BOOL,
            LIT_POINTER,
            LIT_STRING,
            LIT_CHAR,
            OPERATOR
        };


        token_type type;
        char *content;


        token(token_type t, char *c): type(t), content(c) {}
        ~token(void) { delete[] content; }
};


class identifier_token:
    public token
{
    public:
        char *value;
        identifier_token(char *c);
};


class lit_integer_token:
    public token
{
    public:
        enum integer_type
        {
            SIGNED    = (0 << 0),
            UNSIGNED  = (1 << 0),
            INT       = (0 << 1),
            LONG      = (1 << 1),
            LONG_LONG = (2 << 1),

            SIGNED_INT         =   SIGNED | INT,
            UNSIGNED_INT       = UNSIGNED | INT,
            SIGNED_LONG        =   SIGNED | LONG,
            UNSIGNED_LONG      = UNSIGNED | LONG,
            SIGNED_LONG_LONG   =   SIGNED | LONG_LONG,
            UNSIGNED_LONG_LONG = UNSIGNED | LONG_LONG
        };

        integer_type subtype;
        union
        {
              signed long long s;
            unsigned long long u;
        } value;

        lit_integer_token(char *c);
};


class lit_float_token:
    public token
{
    public:
        enum float_type
        {
            FLOAT,
            DOUBLE,
            LONG_DOUBLE
        };

        float_type subtype;
        long double value;

        lit_float_token(char *c);
};


class lit_bool_token:
    public token
{
    public:
        bool value;
        lit_bool_token(char *c);
};


class lit_pointer_token:
    public token
{
    public:
        void *value;
        lit_pointer_token(char *c);
};


class lit_string_token:
    public token
{
    public:
        char *value;
        size_t length;
        lit_string_token(char *c);
        ~lit_string_token(void) { delete[] value; }
};


class lit_char_token:
    public token
{
    public:
        unsigned value;
        lit_char_token(char *c);
};


class operator_token:
    public token
{
    public:
        char *value;
        operator_token(char *c);
};


std::vector<token *> tokenize(const char *str);

#endif
