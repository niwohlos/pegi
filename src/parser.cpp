#include <cassert>
#include <cstdio>
#include <cstring>
#include <list>
#include <vector>

#include "parser.hpp"
#include "tokenize.hpp"


syntax_tree_node::syntax_tree_node(sv_type t, syntax_tree_node *p):
    parent(p), type(t)
{
    if (p)
        p->children.push_back(this);
}


syntax_tree_node::~syntax_tree_node(void)
{
    for (syntax_tree_node *n: children)
        delete n;
}


void syntax_tree_node::detach(void)
{
    if (parent)
        parent->children.remove(this);
}


struct keyword_entry
{
    const char *identifier;
    syntax_tree_node *declaration;
};


// XXX: Make this into a prefix tree or something
// new and delete are operators; false, nullptr and true are literals.
static std::list<keyword_entry> keywords({
    {"alignas", nullptr}, {"alignof", nullptr}, {"asm", nullptr},
    {"auto", nullptr}, {"bool", nullptr}, {"break", nullptr}, {"case", nullptr},
    {"catch", nullptr}, {"char", nullptr}, {"char16_t", nullptr},
    {"char32_t", nullptr}, {"class", nullptr}, {"const", nullptr},
    {"constexpr", nullptr}, {"const_cast", nullptr}, {"continue", nullptr},
    {"decltype", nullptr}, {"default", nullptr}, {"do", nullptr},
    {"double", nullptr}, {"dynamic_cast", nullptr}, {"else", nullptr},
    {"enum", nullptr}, {"explicit", nullptr}, {"export", nullptr},
    {"extern", nullptr}, {"float", nullptr}, {"for", nullptr},
    {"friend", nullptr}, {"goto", nullptr}, {"if", nullptr},
    {"inline", nullptr}, {"int", nullptr}, {"long", nullptr},
    {"mutable", nullptr}, {"namespace", nullptr}, {"noexcept", nullptr},
    {"operator", nullptr}, {"private", nullptr}, {"protected", nullptr},
    {"public", nullptr}, {"register", nullptr}, {"reinterpret_cast", nullptr},
    {"return", nullptr}, {"short", nullptr}, {"signed", nullptr},
    {"sizeof", nullptr}, {"static", nullptr}, {"static_assert", nullptr},
    {"static_cast", nullptr}, {"struct", nullptr}, {"switch", nullptr},
    {"template", nullptr}, {"this", nullptr}, {"thread_local", nullptr},
    {"throw", nullptr}, {"try", nullptr}, {"typedef", nullptr},
    {"typeid", nullptr}, {"typename", nullptr}, {"union", nullptr},
    {"unsigned", nullptr}, {"using", nullptr}, {"virtual", nullptr},
    {"void", nullptr}, {"volatile", nullptr}, {"wchar_t", nullptr},
    {"while", nullptr}
});


typedef std::vector<token *>::const_iterator range_t;


static bool is_keyword(syntax_tree_node *parent, token *tok, const char *name)
{
    (void)parent;

    identifier_token *it = reinterpret_cast<identifier_token *>(tok);
    if (name && strcmp(it->value, name))
        return false;

    for (const keyword_entry &kw: keywords)
        if (!strcmp(it->value, kw.identifier) && parent->sees(kw.declaration))
            return true;

    return false;
}


static bool is_identifier(syntax_tree_node *parent, token *tok, const char *name)
{
    (void)parent;

    identifier_token *it = reinterpret_cast<identifier_token *>(tok);
    if (name && strcmp(it->value, name))
        return false;

    for (const keyword_entry &kw: keywords)
        if (!strcmp(it->value, kw.identifier) && parent->sees(kw.declaration))
            return false;

    return true;
}


static range_t sv_trivially_balanced_token(syntax_tree_node *parent, range_t b, range_t e)
{
    (void)e;

    if ((*b)->type == token::OPERATOR)
    {
        operator_token *tok = reinterpret_cast<operator_token *>(*b);
        if (!strcmp(tok->value, "(") || !strcmp(tok->value, "[") || !strcmp(tok->value, "{") ||
            !strcmp(tok->value, ")") || !strcmp(tok->value, "]") || !strcmp(tok->value, "}"))
        {
            return b;
        }
    }

    syntax_tree_node *node = new syntax_tree_node(syntax_tree_node::TRIVIALLY_BALANCED_TOKEN, parent);
    (new syntax_tree_node(syntax_tree_node::TOKEN, node))->ass_token = *b;
    return ++b;
}


// excluding new, new[], delete, delete[], () and [].
static const char *const overloadable_operators[] = {
    "+", "-", "*", "/", "%", "^", "&", "|", "~", "!", "=", "<", ">", "+=", "-=",
    "*=", "/=", "%=", "^=", "&=", "|=", "<<", ">>", ">>=", "<<=", "==", "!=",
    "<=", ">=", "&&", "||", "++", "--", ",", "->*", "->"
};

static range_t sv_overloadable_operator(syntax_tree_node *parent, range_t b, range_t e)
{
    (void)e;

    if ((*b)->type != token::OPERATOR)
        return b;

    operator_token *tok = reinterpret_cast<operator_token *>(*b);

    if (!strcmp(tok->value, "new") || !strcmp(tok->value, "delete"))
    {
        range_t m = b;

        ++m;
        if (((*m)->type == token::OPERATOR) && !strcmp(reinterpret_cast<operator_token *>(*m)->value, "["))
        {
            ++m;
            if (((*m)->type == token::OPERATOR) && !strcmp(reinterpret_cast<operator_token *>(*m)->value, "]"))
            {
                syntax_tree_node *node = new syntax_tree_node(syntax_tree_node::OVERLOADABLE_OPERATOR, parent);
                (new syntax_tree_node(syntax_tree_node::TOKEN, node))->ass_token = *b;
                (new syntax_tree_node(syntax_tree_node::TOKEN, node))->ass_token = *++b;
                (new syntax_tree_node(syntax_tree_node::TOKEN, node))->ass_token = *++b;
                return ++b;
            }
        }
    }
    else if (!strcmp(tok->value, "(") || !strcmp(tok->value, "["))
    {
        range_t m = b;

        ++m;
        if (((*m)->type != token::OPERATOR) || strcmp(reinterpret_cast<operator_token *>(*m)->value, (*tok->value == '(') ? ")" : "]"))
            return b;
    }
    else
    {
        bool found = false;
        for (const char *op: overloadable_operators)
        {
            if (!strcmp(tok->value, op))
            {
                found = true;
                break;
            }
        }

        if (!found)
            return b;
    }


    syntax_tree_node *node = new syntax_tree_node(syntax_tree_node::OVERLOADABLE_OPERATOR, parent);
    (new syntax_tree_node(syntax_tree_node::TOKEN, node))->ass_token = *b;
    return ++b;
}


static range_t sv_typedef_name(syntax_tree_node *parent, range_t b, range_t e)
{
    (void)parent;
    (void)e;

    return b;
}


static range_t sv_original_namespace_name(syntax_tree_node *parent, range_t b, range_t e)
{
    (void)parent;
    (void)e;

    return b;
}


static range_t sv_namespace_alias(syntax_tree_node *parent, range_t b, range_t e)
{
    (void)parent;
    (void)e;

    return b;
}


static range_t sv_class_name(syntax_tree_node *parent, range_t b, range_t e)
{
    (void)parent;
    (void)e;

    return b;
}


static range_t sv_enum_name(syntax_tree_node *parent, range_t b, range_t e)
{
    (void)parent;
    (void)e;

    return b;
}


static range_t sv_template_name(syntax_tree_node *parent, range_t b, range_t e)
{
    (void)parent;
    (void)e;

    return b;
}


#include "parser-sv-prototypes.cxx"

#include "parser-sv-handlers.cxx"


syntax_tree_node *build_syntax_tree(const std::vector<token *> &token_list)
{
    return sv_translation_unit(token_list.begin(), token_list.end());
}
