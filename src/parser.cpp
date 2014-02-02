#include <cassert>
#include <cstdio>
#include <cstring>
#include <list>
#include <vector>

#include "parser.hpp"
#include "tokenize.hpp"


syntax_tree_node::syntax_tree_node(sv_type t, syntax_tree_node *p, bool i):
    parent(p), type(t), intermediate(i)
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


bool syntax_tree_node::sees(const syntax_tree_node *other) const
{
    if (!other)
        return true;

    const syntax_tree_node *other_compound = other->compound();
    if (!other_compound)
        return true;

    for (const syntax_tree_node *c = compound(); c; c = c->compound())
        if (c == other_compound)
            return true;

    return false;
}


syntax_tree_node *syntax_tree_node::compound(void) const
{
    for (syntax_tree_node *n = parent; n; n = n->parent)
        if (n->type == syntax_tree_node::COMPOUND_STATEMENT)
            return n;

    return nullptr;
}


void syntax_tree_node::contract(void)
{
    auto i = children.begin();

    while (i != children.end())
    {
        syntax_tree_node *child = *i;

        child->contract();

        if (!child->intermediate)
            ++i;
        else
        {
            i = children.erase(i);
            children.splice(i, child->children);
            // i now points to the element after the newly adopted grandchildren
            // (this is correct, as there may be no intermediate nodes among
            // those due to child->contract() before)

            delete child;
        }
    }
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

static range_t maximum_extent;


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
    if (b == e) return b;

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
    if (b == e) return b;

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


std::list<keyword_entry> typedef_names, class_names, template_names;


static void push_plain_qualified_ids(syntax_tree_node *node, syntax_tree_node *declaration, std::list<keyword_entry> *target)
{
    for (syntax_tree_node *c: node->children)
    {
        if ((c->type == syntax_tree_node::UNQUALIFIED_ID) &&
            (c->children.front()->type == syntax_tree_node::TOKEN) &&
            (c->children.front()->ass_token->type == token::IDENTIFIER))
        {
            target->push_back({strdup(reinterpret_cast<identifier_token *>(c->children.front()->ass_token)->value), declaration});
            keywords.push_back({strdup(reinterpret_cast<identifier_token *>(c->children.front()->ass_token)->value), declaration});
        }
        else
            push_plain_qualified_ids(c, declaration, target);
    }
}


static void simple_declaration_done(syntax_tree_node *node)
{
    syntax_tree_node *dss = nullptr, *idl = nullptr;
    for (syntax_tree_node *c: node->children)
    {
        if (c->type == syntax_tree_node::DECL_SPECIFIER_SEQ)
            dss = c;
        if (c->type == syntax_tree_node::INIT_DECLARATOR_LIST)
            idl = c;
    }

    if (dss && idl)
    {
        for (syntax_tree_node *c: dss->children)
        {
            if ((c->type == syntax_tree_node::DECL_SPECIFIER) &&
                (c->children.front()->type == syntax_tree_node::TOKEN) &&
                (c->children.front()->ass_token->type == token::IDENTIFIER) &&
                !strcmp(reinterpret_cast<identifier_token *>(c->children.front()->ass_token)->value, "typedef"))
            {
                // node: simple-declaration
                // node->parent: block-declaration
                // node->parent->parent: declaration
                push_plain_qualified_ids(idl, node->parent->parent, &typedef_names);
                break;
            }
        }
    }
    else if (dss)
    {
        for (syntax_tree_node *c: dss->children)
        {
            if (c->type != syntax_tree_node::DECL_SPECIFIER) continue;
            if ((c = c->children.front())->type != syntax_tree_node::TYPE_SPECIFIER) continue;
            c = c->children.front();

            identifier_token *tok = nullptr;

            if (c->type == syntax_tree_node::TRAILING_TYPE_SPECIFIER)
            {
                if ((c = c->children.front())->type != syntax_tree_node::ELABORATED_TYPE_SPECIFIER) continue;
                if (c->children.front()->type != syntax_tree_node::CLASS_KEY) continue;
                if ((c = c->children.back())->type != syntax_tree_node::TOKEN) continue;
                if (c->ass_token->type != token::IDENTIFIER) continue;
                tok = reinterpret_cast<identifier_token *>(c->ass_token);
            }
            else if (c->type == syntax_tree_node::CLASS_SPECIFIER)
            {
                if ((c = c->children.front())->type != syntax_tree_node::CLASS_HEAD) continue;
                for (syntax_tree_node *cc: c->children)
                {
                    if (cc->type == syntax_tree_node::CLASS_HEAD_NAME)
                    {
                        if ((cc = cc->children.back())->type != syntax_tree_node::TOKEN) continue;
                        if (cc->ass_token->type != token::IDENTIFIER) continue;
                        tok = reinterpret_cast<identifier_token *>(cc->ass_token);
                        break;
                    }
                }
            }

            if (tok)
            {
                class_names.push_back({strdup(tok->value), node->parent->parent});
                keywords.push_back({strdup(tok->value), node->parent->parent});
            }
        }
    }
}


static void template_declaration_done(syntax_tree_node *node)
{
    for (syntax_tree_node *c: node->children)
        if (c->type == syntax_tree_node::DECLARATION)
            for (const keyword_entry &kw: class_names)
                if (kw.declaration == c)
                    template_names.push_back({strdup(kw.identifier), node->parent});
}


static range_t sv_typedef_name(syntax_tree_node *parent, range_t b, range_t e)
{
    if (b == e) return b;

    if ((*b)->type == token::IDENTIFIER)
    {
        for (const keyword_entry &typedefd: typedef_names)
        {
            if (!strcmp(reinterpret_cast<identifier_token *>(*b)->value, typedefd.identifier) && parent->sees(typedefd.declaration))
            {
                syntax_tree_node *node = new syntax_tree_node(syntax_tree_node::TYPEDEF_NAME, parent);
                (new syntax_tree_node(syntax_tree_node::TOKEN, node))->ass_token = *b;
                return ++b;
            }
        }
    }

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
    if (b == e) return b;

    if ((*b)->type == token::IDENTIFIER)
    {
        for (const keyword_entry &cn: class_names)
        {
            if (!strcmp(reinterpret_cast<identifier_token *>(*b)->value, cn.identifier) && parent->sees(cn.declaration))
            {
                syntax_tree_node *node = new syntax_tree_node(syntax_tree_node::CLASS_NAME, parent);
                (new syntax_tree_node(syntax_tree_node::TOKEN, node))->ass_token = *b;
                return ++b;
            }
        }

        // FIXME: Only accept class typedefs here (i.e., resolve typedef)
        for (const keyword_entry &typedefd: typedef_names)
        {
            if (!strcmp(reinterpret_cast<identifier_token *>(*b)->value, typedefd.identifier) && parent->sees(typedefd.declaration))
            {
                syntax_tree_node *node = new syntax_tree_node(syntax_tree_node::CLASS_NAME, parent);
                (new syntax_tree_node(syntax_tree_node::TOKEN, node))->ass_token = *b;
                return ++b;
            }
        }
    }

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
    if (b == e) return b;

    if ((*b)->type == token::IDENTIFIER)
    {
        for (const keyword_entry &tn: template_names)
        {
            if (!strcmp(reinterpret_cast<identifier_token *>(*b)->value, tn.identifier) && parent->sees(tn.declaration))
            {
                syntax_tree_node *node = new syntax_tree_node(syntax_tree_node::TEMPLATE_NAME, parent);
                (new syntax_tree_node(syntax_tree_node::TOKEN, node))->ass_token = *b;
                return ++b;
            }
        }
    }

    return b;
}


#include "parser-sv-prototypes.cxx"

#include "parser-sv-handlers.cxx"


syntax_tree_node *build_syntax_tree(const std::vector<token *> &token_list)
{
    maximum_extent = token_list.begin();
    syntax_tree_node *root = sv_translation_unit(token_list.begin(), token_list.end());

    root->contract();

    if (maximum_extent != token_list.end())
        fprintf(stderr, "Parse error: %s (%i:%i)\n", (*maximum_extent)->content, (*maximum_extent)->line, (*maximum_extent)->column);

    return root;
}
