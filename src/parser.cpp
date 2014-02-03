#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <list>
#include <vector>

#include "error.hpp"
#include "format.hpp"
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


/* FIXME: We have to check whether the other node appeared before this one
 * beside checking the scope (e.g. through adding line/column information to
 * every node). Currently, this works without, as this is only used for
 * declarations as "other" and they are only added when they have been
 * completely matched - furthermore, the current state of the code is that it
 * doesn't even consider declarations being wrongly matched and discarding
 * them later, soooooooo... */
bool syntax_tree_node::sees(const syntax_tree_node *other) const
{
    if (!other)
        return true;

    const syntax_tree_node *other_scope = other->scope();
    if (!other_scope)
        return true;

    for (const syntax_tree_node *s = scope(); s; s = s->scope())
        if (s == other_scope)
            return true;

    return false;
}


/**
 * Finds the associated scope block, which is one of the following:
 *   - a compound statement
 *   - a class specifier
 */
syntax_tree_node *syntax_tree_node::scope(void) const
{
    for (syntax_tree_node *n = parent; n; n = n->parent)
    {
        if ((n->type == syntax_tree_node::COMPOUND_STATEMENT) || (n->type == syntax_tree_node::CLASS_SPECIFIER))
            return n;
        else if (n->type == syntax_tree_node::TEMPLATE_DECLARATION)
        {
            // If above this there is first a template declaration before any
            // scope block appears, the appropriate scope is probably the scope
            // enclosed by the template declaration.
            return n->scope_below();
        }
    }

    return nullptr;
}


/**
 * Finds the first scope block below this node.
 */
syntax_tree_node *syntax_tree_node::scope_below(void) const
{
    for (syntax_tree_node *c: children)
    {
        if ((c->type == syntax_tree_node::COMPOUND_STATEMENT) || (c->type == syntax_tree_node::CLASS_SPECIFIER))
            return c;

        syntax_tree_node *sb = c->scope_below();
        if (sb)
            return sb;
    }

    return nullptr;
}


/**
 * This function moves children of intermediate nodes and nodes which are of the
 * same type as their parent (i.e., loop through recursion) to their parent and
 * removes them from the syntax tree.
 */
void syntax_tree_node::contract(void)
{
    auto i = children.begin();

    while (i != children.end())
    {
        syntax_tree_node *child = *i;

        child->contract();

        if (!child->intermediate && (child->type != type))
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


/**
 * Returns the first token for this node.
 */
token *syntax_tree_node::first_token(void) const
{
    if (type == syntax_tree_node::TOKEN)
        return ass_token;

    for (syntax_tree_node *c: children)
    {
        token *t = c->first_token();
        if (t)
            return t;
    }

    return nullptr;
}


/**
 * Fixes >> and >>= from > > and > >= (after we're sure it's not > > or > >=).
 */
void syntax_tree_node::fix_right_shifts(void)
{
    if (type == syntax_tree_node::ASSIGNMENT_OPERATOR)
    {
        if (!strcmp(reinterpret_cast<operator_token *>(children.front()->ass_token)->value, ">") &&
            !strcmp(reinterpret_cast<operator_token *>(children.back ()->ass_token)->value, ">="))
        {
            children.pop_back();
            operator_token *tok = reinterpret_cast<operator_token *>(children.front()->ass_token);
            delete tok->content;

            strcpy(tok->value = tok->content = new char[4], ">>=");
        }
    }
    else if (type == syntax_tree_node::SHIFT_OPERATOR)
    {
        if (!strcmp(reinterpret_cast<operator_token *>(children.front()->ass_token)->value, ">") &&
            !strcmp(reinterpret_cast<operator_token *>(children.back ()->ass_token)->value, ">"))
        {
            children.pop_back();
            operator_token *tok = reinterpret_cast<operator_token *>(children.front()->ass_token);
            delete tok->content;

            strcpy(tok->value = tok->content = new char[3], ">>");
        }
    }
    else
        for (syntax_tree_node *c: children)
            c->fix_right_shifts();
}


struct keyword_entry
{
    const char *identifier;
    syntax_tree_node *declaration;
    bool builtin;
};


// XXX: Make this into a prefix tree or something
static std::list<keyword_entry> keywords;


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


static range_t sv_trivially_balanced_token(syntax_tree_node *parent, range_t b, range_t e, bool *success)
{
    if (b == e) { *success = false; return b; }

    if ((*b)->type == token::OPERATOR)
    {
        operator_token *tok = reinterpret_cast<operator_token *>(*b);
        if (!strcmp(tok->value, "(") || !strcmp(tok->value, "[") || !strcmp(tok->value, "{") ||
            !strcmp(tok->value, ")") || !strcmp(tok->value, "]") || !strcmp(tok->value, "}"))
        {
            *success = false;
            return b;
        }
    }

    syntax_tree_node *node = new syntax_tree_node(syntax_tree_node::TRIVIALLY_BALANCED_TOKEN, parent);
    (new syntax_tree_node(syntax_tree_node::TOKEN, node))->ass_token = *b;
    *success = true;
    return ++b;
}


// excluding new, new[], delete, delete[], () and [].
static const char *const overloadable_operators[] = {
    "+", "-", "*", "/", "%", "^", "&", "|", "~", "!", "=", "<", ">", "+=", "-=",
    "*=", "/=", "%=", "^=", "&=", "|=", "<<", ">>", ">>=", "<<=", "==", "!=",
    "<=", ">=", "&&", "||", "++", "--", ",", "->*", "->"
};

static range_t sv_overloadable_operator(syntax_tree_node *parent, range_t b, range_t e, bool *success)
{
    if (b == e) { *success = false; return b; }

    if ((*b)->type != token::OPERATOR)
    {
        *success = false;
        return b;
    }

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
                *success = true;
                return ++b;
            }
        }
    }
    else if (!strcmp(tok->value, "(") || !strcmp(tok->value, "["))
    {
        range_t m = b;

        ++m;
        if (((*m)->type != token::OPERATOR) || strcmp(reinterpret_cast<operator_token *>(*m)->value, (*tok->value == '(') ? ")" : "]"))
        {
            *success = false;
            return b;
        }
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
        {
            *success = false;
            return b;
        }
    }


    syntax_tree_node *node = new syntax_tree_node(syntax_tree_node::OVERLOADABLE_OPERATOR, parent);
    (new syntax_tree_node(syntax_tree_node::TOKEN, node))->ass_token = *b;
    *success = true;
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
            target->push_back({strdup(reinterpret_cast<identifier_token *>(c->children.front()->ass_token)->value), declaration, false});
            keywords.push_back({strdup(reinterpret_cast<identifier_token *>(c->children.front()->ass_token)->value), declaration, false});
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
                class_names.push_back({strdup(tok->value), node->parent->parent, false});
                keywords.push_back({strdup(tok->value), node->parent->parent, false});
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
                    template_names.push_back({strdup(kw.identifier), node->parent, false});
}


// FIXME: Having a match for this doesn't mean anything. We should really be
// able to revoke such a declaration if necessary.
static void template_parameter_done(syntax_tree_node *node)
{
    // Not really a declaration but lol idc yours clici

    syntax_tree_node *declaration = node;
    node = node->children.front();

    if (node->type == syntax_tree_node::TYPE_PARAMETER)
    {
        const char *identifier = nullptr;
        // am i doin it rite
        for (syntax_tree_node *c: node->children)
        {
            if ((c->type == syntax_tree_node::TOKEN) &&
                (c->ass_token->type == token::IDENTIFIER) &&
                is_identifier(c, c->ass_token, nullptr))
            {
                identifier = reinterpret_cast<identifier_token *>(c->ass_token)->value;
                break;
            }
        }

        if (identifier)
        {
            if (!strcmp(reinterpret_cast<identifier_token *>(node->children.front()->ass_token)->value, "template"))
                template_names.push_back({strdup(identifier), declaration, false});
            else if (!strcmp(reinterpret_cast<identifier_token *>(node->children.front()->ass_token)->value, "typename"))
                typedef_names.push_back({strdup(identifier), declaration, false});
            else if (!strcmp(reinterpret_cast<identifier_token *>(node->children.front()->ass_token)->value, "class"))
                class_names.push_back({strdup(identifier), declaration, false});
            else
                throw format("A type parameter must be precedented by template, typename or class. Check the syntax definition file.");

            keywords.push_back({strdup(identifier), declaration, false});
        }
    }
    else
        throw format("Unknown template parameter type %s", parser_type_names[node->type]);
}


static range_t sv_typedef_name(syntax_tree_node *parent, range_t b, range_t e, bool *success)
{
    if (b == e) { *success = false; return b; }

    if ((*b)->type == token::IDENTIFIER)
    {
        for (const keyword_entry &typedefd: typedef_names)
        {
            if (!strcmp(reinterpret_cast<identifier_token *>(*b)->value, typedefd.identifier) && parent->sees(typedefd.declaration))
            {
                syntax_tree_node *node = new syntax_tree_node(syntax_tree_node::TYPEDEF_NAME, parent);
                (new syntax_tree_node(syntax_tree_node::TOKEN, node))->ass_token = *b;
                *success = true;
                return ++b;
            }
        }
    }

    *success = false;
    return b;
}


static range_t sv_original_namespace_name(syntax_tree_node *parent, range_t b, range_t e, bool *success)
{
    (void)parent;
    (void)e;

    *success = false;
    return b;
}


static range_t sv_namespace_alias(syntax_tree_node *parent, range_t b, range_t e, bool *success)
{
    (void)parent;
    (void)e;

    *success = false;
    return b;
}


static range_t sv_class_name(syntax_tree_node *parent, range_t b, range_t e, bool *success)
{
    if (b == e) { *success = false; return b; }

    if ((*b)->type == token::IDENTIFIER)
    {
        for (const keyword_entry &cn: class_names)
        {
            if (!strcmp(reinterpret_cast<identifier_token *>(*b)->value, cn.identifier) && parent->sees(cn.declaration))
            {
                syntax_tree_node *node = new syntax_tree_node(syntax_tree_node::CLASS_NAME, parent);
                (new syntax_tree_node(syntax_tree_node::TOKEN, node))->ass_token = *b;
                *success = true;
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
                *success = true;
                return ++b;
            }
        }
    }

    *success = false;
    return b;
}


static range_t sv_enum_name(syntax_tree_node *parent, range_t b, range_t e, bool *success)
{
    (void)parent;
    (void)e;

    *success = false;
    return b;
}


static range_t sv_template_name(syntax_tree_node *parent, range_t b, range_t e, bool *success)
{
    if (b == e) { *success = false; return b; }

    if ((*b)->type == token::IDENTIFIER)
    {
        for (const keyword_entry &tn: template_names)
        {
            if (!strcmp(reinterpret_cast<identifier_token *>(*b)->value, tn.identifier) && parent->sees(tn.declaration))
            {
                syntax_tree_node *node = new syntax_tree_node(syntax_tree_node::TEMPLATE_NAME, parent);
                (new syntax_tree_node(syntax_tree_node::TOKEN, node))->ass_token = *b;
                *success = true;
                return ++b;
            }
        }
    }

    *success = false;
    return b;
}


static range_t sv_right_shift(syntax_tree_node *parent, range_t b, range_t e, bool *success)
{
    range_t m = b;

    if ((m != e) && ((*m)->type == token::OPERATOR) && !strcmp(reinterpret_cast<operator_token *>(*m)->value, ">"))
    {
        ++m;
        if ((m != e) && ((*m)->type == token::OPERATOR) &&
            ((*m)->line == (*b)->line) && ((*m)->column == (*b)->column + 1) &&
            !strcmp(reinterpret_cast<operator_token *>(*m)->value, ">"))
        {
            // XXX: This is evil. All code normally assumes that every SV
            // matching function only adds a single child node to the parent.
            // This is important for removing the correct number of incompletely
            // matched children after a loop. However, this SV's parent
            // (shift-operator) is never part of a loop. Therefore, this is
            // safe.
            (new syntax_tree_node(syntax_tree_node::TOKEN, parent))->ass_token = *b;
            (new syntax_tree_node(syntax_tree_node::TOKEN, parent))->ass_token = *m;
            *success = true;
            return ++m;
        }
    }

    *success = false;
    return b;
}


static range_t sv_right_shift_assignment(syntax_tree_node *parent, range_t b, range_t e, bool *success)
{
    range_t m = b;

    if ((m != e) && ((*m)->type == token::OPERATOR) && !strcmp(reinterpret_cast<operator_token *>(*m)->value, ">"))
    {
        ++m;
        if ((m != e) && ((*m)->type == token::OPERATOR) &&
            ((*m)->line == (*b)->line) && ((*m)->column == (*b)->column + 1) &&
            !strcmp(reinterpret_cast<operator_token *>(*m)->value, ">="))
        {
            // XXX: See above.
            (new syntax_tree_node(syntax_tree_node::TOKEN, parent))->ass_token = *b;
            (new syntax_tree_node(syntax_tree_node::TOKEN, parent))->ass_token = *m;
            *success = true;
            return ++m;
        }
    }

    *success = false;
    return b;
}


// God I hate this fucking syntax
static range_t repair_noptr_declarator(syntax_tree_node *node, range_t b, range_t e, bool *success)
{
    (void)e;
    (void)success;

    if (node->parent->type != syntax_tree_node::DECLARATOR)
        return b;

    syntax_tree_node *c = node->children.back();
    if (c->type != syntax_tree_node::NOPTR_DECLARATOR_REPEATABLE)
        return b;

    if (c->children.front()->type != syntax_tree_node::PARAMETERS_AND_QUALIFIERS)
        return b;

    // I HATE IT

    token *tok = c->first_token();

    delete c;
    node->children.pop_back();

    while (*b != tok)
        --b;

    return b;

    /* Explanation: If noptr-declarator appears directly inside of declarator,
     * it is followed by a mandatory parameters-and-qualifiers. However,
     * noptr-declarator itself ends with a loop of parameters-and-qualifiers of
     * indeterminate length. Therefore, we have to stuff the final instance back
     * if noptr-declarator appears directly inside of declarator and ends with
     * parameters-and-qualifiers. */
}


#include "parser-sv-prototypes.cxx"

#include "parser-sv-handlers.cxx"


syntax_tree_node *build_syntax_tree(const std::vector<token *> &token_list)
{
    maximum_extent = token_list.begin();

    for (std::list<keyword_entry> *kwl: {&keywords, &typedef_names, &class_names, &template_names})
    {
        for (const keyword_entry &kw: *kwl)
            if (!kw.builtin)
                free(const_cast<char *>(kw.identifier));

        kwl->clear();
    }

    // new and delete are operators; false, nullptr and true are literals.
    keywords = {
        {"alignas", nullptr, true}, {"alignof", nullptr, true},
        {"asm", nullptr, true}, {"auto", nullptr, true},
        {"bool", nullptr, true}, {"break", nullptr, true},
        {"case", nullptr, true}, {"catch", nullptr, true},
        {"char", nullptr, true}, {"char16_t", nullptr, true},
        {"char32_t", nullptr, true}, {"class", nullptr, true},
        {"const", nullptr, true}, {"constexpr", nullptr, true},
        {"const_cast", nullptr, true}, {"continue", nullptr, true},
        {"decltype", nullptr, true}, {"default", nullptr, true},
        {"do", nullptr, true}, {"double", nullptr, true},
        {"dynamic_cast", nullptr, true}, {"else", nullptr, true},
        {"enum", nullptr, true}, {"explicit", nullptr, true},
        {"export", nullptr, true}, {"extern", nullptr, true},
        {"float", nullptr, true}, {"for", nullptr, true},
        {"friend", nullptr, true}, {"goto", nullptr, true},
        {"if", nullptr, true}, {"inline", nullptr, true},
        {"int", nullptr, true}, {"long", nullptr, true},
        {"mutable", nullptr, true}, {"namespace", nullptr, true},
        {"noexcept", nullptr, true}, {"operator", nullptr, true},
        {"private", nullptr, true}, {"protected", nullptr, true},
        {"public", nullptr, true}, {"register", nullptr, true},
        {"reinterpret_cast", nullptr, true}, {"return", nullptr, true},
        {"short", nullptr, true}, {"signed", nullptr, true},
        {"sizeof", nullptr, true}, {"static", nullptr, true},
        {"static_assert", nullptr, true}, {"static_cast", nullptr, true},
        {"struct", nullptr, true}, {"switch", nullptr, true},
        {"template", nullptr, true}, {"this", nullptr, true},
        {"thread_local", nullptr, true}, {"throw", nullptr, true},
        {"try", nullptr, true}, {"typedef", nullptr, true},
        {"typeid", nullptr, true}, {"typename", nullptr, true},
        {"union", nullptr, true}, {"unsigned", nullptr, true},
        {"using", nullptr, true}, {"virtual", nullptr, true},
        {"void", nullptr, true}, {"volatile", nullptr, true},
        {"wchar_t", nullptr, true}, {"while", nullptr, true}
    };

    syntax_tree_node *root = nullptr;
    try
    {
        bool success;
        root = sv_translation_unit(token_list.begin(), token_list.end(), &success);
        root->contract();
        root->fix_right_shifts();

        if (!success || (maximum_extent != token_list.end()))
            throw format("Could not match token %s", (*maximum_extent)->content);
    }
    catch (char *msg)
    {
        if (maximum_extent == token_list.end())
            throw new error(msg);
        else
            throw new error((*maximum_extent)->line, (*maximum_extent)->column, msg);
    }

    return root;
}
