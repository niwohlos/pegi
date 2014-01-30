#include <cstdio>
#include <cstring>

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


typedef std::vector<token *>::const_iterator range_t;


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
