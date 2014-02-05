#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <list>
#include <stack>
#include <vector>

#include "error.hpp"
#include "format.hpp"
#include "parser.hpp"
#include "tokenize.hpp"


struct keyword_entry
{
    const char *identifier;
    syntax_tree_node *declaration, *complete_declaration;
};


// XXX: Make this into a prefix tree or something
static std::list<keyword_entry> keywords, typedef_names, class_names, template_names, original_namespace_names;


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

    bool registered = false;
    for (auto it = keywords.begin(); it != keywords.end();)
    {
        auto it_next = it;
        ++it_next;

        if ((this == (*it).declaration) || (this == (*it).complete_declaration))
        {
            free(const_cast<char *>((*it).identifier));
            keywords.erase(it);
            registered = true;
        }

        it = it_next;
    }

    if (registered)
    {
        for (std::list<keyword_entry> *kwl: {&typedef_names, &class_names, &template_names, &original_namespace_names})
        {
            for (auto it = kwl->begin(); it != kwl->end();)
            {
                auto it_next = it;
                ++it_next;

                if ((this == (*it).declaration) || (this == (*it).complete_declaration))
                {
                    free(const_cast<char *>((*it).identifier));
                    kwl->erase(it);
                }

                it = it_next;
            }
        }
    }
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
        return false;

    for (const syntax_tree_node *s = scope(); s; s = s->scope_above())
        if (s == other_scope)
            return true;

    return false;
}


bool syntax_tree_node::sees_in_ns(const syntax_tree_node *other, const syntax_tree_node *ns) const
{
    // FIXME: Oh god does this even work
    return ns ? (other->scope_above() == ns) : sees(other);
}


/**
 * Finds the associated scope block, which is one of the following:
 *   - a compound statement
 *   - a class specifier
 *   - a declaration sequence
 */
syntax_tree_node *syntax_tree_node::scope(void) const
{
    for (syntax_tree_node *n = parent; n; n = n->parent)
    {
        if ((n->type == syntax_tree_node::COMPOUND_STATEMENT) || (n->type == syntax_tree_node::CLASS_SPECIFIER) || (n->type == syntax_tree_node::DECLARATION_SEQ))
            return n;
        else if (n->type == syntax_tree_node::TEMPLATE_DECLARATION)
        {
            // If above this there is first a template declaration before any
            // scope block appears, the appropriate scope is probably the scope
            // enclosed by the template declaration.
            return n->scope_below();
        }
    }

    token *tok = first_token();
    if (tok)
        throw format("%p Could not resolve scope of a %s node (%i:%i)", parent, parser_type_names[type], tok->line, tok->column);
    else
        throw format("Could not resolve scope of a %s node", parser_type_names[type]);
}


/**
 * Finds the first scope block above this node.
 */
syntax_tree_node *syntax_tree_node::scope_above(void) const
{
    for (syntax_tree_node *n = parent; n; n = n->parent)
        if ((n->type == syntax_tree_node::COMPOUND_STATEMENT) || (n->type == syntax_tree_node::CLASS_SPECIFIER) || (n->type == syntax_tree_node::DECLARATION_SEQ))
            return n;

    return nullptr;
}


/**
 * Finds the first scope block below this node.
 */
syntax_tree_node *syntax_tree_node::scope_below(void) const
{
    for (syntax_tree_node *c: children)
    {
        if ((c->type == syntax_tree_node::COMPOUND_STATEMENT) || (c->type == syntax_tree_node::CLASS_SPECIFIER) || (c->type == syntax_tree_node::DECLARATION_SEQ))
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


typedef std::vector<token *>::const_iterator range_t;

static range_t maximum_extent;


#include "parser-sv-prototypes.cxx"


// Funny thing about C++: Only use the keyword/identifier separation for the
// current namespace.
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
    if (++b > maximum_extent) maximum_extent = b;
    *success = true;
    return b;
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
                if (++b > maximum_extent) maximum_extent = b;
                *success = true;
                return b;
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
    if (++b > maximum_extent) maximum_extent = b;
    *success = true;
    return b;
}


struct namespace_scope_entry
{
    syntax_tree_node *scope;
    syntax_tree_node *related;
};

std::stack<namespace_scope_entry> namespace_scope_stack;

static syntax_tree_node *namespace_scope;


static void push_plain_qualified_ids(syntax_tree_node *node, syntax_tree_node *declaration, std::list<keyword_entry> *target)
{
    for (syntax_tree_node *c: node->children)
    {
        if ((c->type == syntax_tree_node::UNQUALIFIED_ID) &&
            (c->children.front()->type == syntax_tree_node::TOKEN) &&
            (c->children.front()->ass_token->type == token::IDENTIFIER))
        {
            // FIXME: Use complete_declaration for type reference
            target->push_back({strdup(reinterpret_cast<identifier_token *>(c->children.front()->ass_token)->value), declaration, nullptr});
            keywords.push_back({strdup(reinterpret_cast<identifier_token *>(c->children.front()->ass_token)->value), declaration, nullptr});
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

                class_names.push_back({strdup(tok->value), node->parent->parent, nullptr});
                keywords.push_back({strdup(tok->value), node->parent->parent, nullptr});
            }
        }
    }
}


static void class_specifier_done(syntax_tree_node *node)
{
    syntax_tree_node *c = node->children.front();

    if (c->type != syntax_tree_node::CLASS_HEAD) return;
    for (syntax_tree_node *cc: c->children)
    {
        if (cc->type == syntax_tree_node::CLASS_HEAD_NAME)
        {
            if ((cc = cc->children.back())->type != syntax_tree_node::CLASS_NAME) continue;
            if ((cc = cc->children.back())->type != syntax_tree_node::TOKEN) continue;
            if (cc->ass_token->type != token::IDENTIFIER) continue;

            // TODO: Overwrite old entry, if it exists

            syntax_tree_node *decl;
            for (decl = node; decl && (decl->type != syntax_tree_node::DECLARATION) && (decl->type != syntax_tree_node::MEMBER_DECLARATION); decl = decl->parent);
            decl = decl ? decl : node;

            class_names.push_back({strdup(reinterpret_cast<identifier_token *>(cc->ass_token)->value), decl, node});
            keywords.push_back({strdup(reinterpret_cast<identifier_token *>(cc->ass_token)->value), decl, node});

            return;
        }
    }
}


static void template_declaration_done(syntax_tree_node *node)
{
    for (syntax_tree_node *c: node->children)
        if ((c->type == syntax_tree_node::DECLARATION) || (c->type == syntax_tree_node::MEMBER_DECLARATION))
            for (const keyword_entry &kw: class_names)
                if (kw.declaration == c)
                    template_names.push_back({strdup(kw.identifier), node->parent, kw.complete_declaration});
}


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
                template_names.push_back({strdup(identifier), declaration, nullptr});
            else if (!strcmp(reinterpret_cast<identifier_token *>(node->children.front()->ass_token)->value, "typename"))
                typedef_names.push_back({strdup(identifier), declaration, nullptr});
            else if (!strcmp(reinterpret_cast<identifier_token *>(node->children.front()->ass_token)->value, "class"))
                class_names.push_back({strdup(identifier), declaration, nullptr});
            else
                throw format("A type parameter must be precedented by template, typename or class. Check the syntax definition file.");

            keywords.push_back({strdup(identifier), declaration, nullptr});
        }
    }
    // Nothing to do for parameter-declaration, since this only introduces a
    // "variable" (a paramter) rather than a new type name
    else if (node->type != syntax_tree_node::PARAMETER_DECLARATION)
        throw format("Unknown template parameter type %s", parser_type_names[node->type]);
}


static void original_namespace_definition_done(syntax_tree_node *node)
{
    auto i = node->children.begin();
    ++i;

    if ((*i)->type != syntax_tree_node::TOKEN)
        throw format("original-namespace-definition must start with at least two tokens. Check the syntax definition file.");

    if (!strcmp(reinterpret_cast<identifier_token *>((*i)->ass_token)->value, "namespace")) // First was "inline", then
        ++i;

    if ((*i)->type != syntax_tree_node::TOKEN)
        throw format("Identifier missing in original-namespace-definition.");

    original_namespace_names.push_back({strdup(reinterpret_cast<identifier_token *>((*i)->ass_token)->value), node, node});
    keywords.push_back({strdup(reinterpret_cast<identifier_token *>((*i)->ass_token)->value), node, node});
}


static range_t sv_typedef_name(syntax_tree_node *parent, range_t b, range_t e, bool *success)
{
    if (b == e) { *success = false; return b; }

    if ((*b)->type == token::IDENTIFIER)
    {
        for (const keyword_entry &typedefd: typedef_names)
        {
            if (!strcmp(reinterpret_cast<identifier_token *>(*b)->value, typedefd.identifier) &&
                parent->sees_in_ns(typedefd.declaration, namespace_scope))
            {
                syntax_tree_node *node = new syntax_tree_node(syntax_tree_node::TYPEDEF_NAME, parent);
                node->supplemental.declaration = typedefd.complete_declaration;
                (new syntax_tree_node(syntax_tree_node::TOKEN, node))->ass_token = *b;
                if (++b > maximum_extent) maximum_extent = b;
                *success = true;
                return b;
            }
        }
    }

    *success = false;
    return b;
}


static range_t sv_original_namespace_name(syntax_tree_node *parent, range_t b, range_t e, bool *success)
{
    if (b == e) { *success = false; return b; }

    if ((*b)->type == token::IDENTIFIER)
    {
        for (const keyword_entry &ns: original_namespace_names)
        {
            if (!strcmp(reinterpret_cast<identifier_token *>(*b)->value, ns.identifier) &&
                parent->sees_in_ns(ns.declaration, namespace_scope))
            {
                syntax_tree_node *node = new syntax_tree_node(syntax_tree_node::ORIGINAL_NAMESPACE_NAME, parent);
                node->supplemental.declaration = ns.complete_declaration;
                (new syntax_tree_node(syntax_tree_node::TOKEN, node))->ass_token = *b;
                if (++b > maximum_extent) maximum_extent = b;
                *success = true;
                return b;
            }
        }
    }

    *success = false;
    return b;
}


static range_t sv_namespace_alias(syntax_tree_node *parent, range_t b, range_t e, bool *success)
{
    (void)parent;

    if (b == e) { *success = false; return b; }

    *success = false;
    return b;
}


static range_t sv_class_name(syntax_tree_node *parent, range_t b, range_t e, bool *success)
{
    if (b == e) { *success = false; return b; }

    syntax_tree_node *node = new syntax_tree_node(syntax_tree_node::CLASS_NAME, parent);

    // FIXME: God please this is shit (read: only accept templates resolving to classes here)
    bool could_parse;
    range_t m = sv_simple_template_id(node, b, e, &could_parse);
    if (could_parse)
    {
        //                             class-name   simple-template-id  template-name
        node->supplemental.declaration = node->children.front()->children.front()->supplemental.declaration;
        if (m > maximum_extent) maximum_extent = m;
        *success = true;
        return m;
    }

    if ((*b)->type == token::IDENTIFIER)
    {
        if (parent->type == syntax_tree_node::CLASS_HEAD_NAME)
        {
            // In this very special case we are in fact introducing a new
            // class-name. Therefore, accept both simple-template-ids (for
            // partial specialization etc.) and any identifier in general.
            // However, they have to be true identifiers (no keywords).

            if (is_identifier(parent, *b, nullptr))
            {
                // class-head-name -> class-head -> class-specifier
                node->supplemental.declaration = parent->parent->parent;
                (new syntax_tree_node(syntax_tree_node::TOKEN, node))->ass_token = *b;
                if (++b > maximum_extent) maximum_extent = b;
                *success = true;
                return b;
            }
        }

        for (const keyword_entry &cn: class_names)
        {
            if (!strcmp(reinterpret_cast<identifier_token *>(*b)->value, cn.identifier) &&
                parent->sees_in_ns(cn.declaration, namespace_scope))
            {
                node->supplemental.declaration = cn.complete_declaration;
                (new syntax_tree_node(syntax_tree_node::TOKEN, node))->ass_token = *b;
                if (++b > maximum_extent) maximum_extent = b;
                *success = true;
                return b;
            }
        }

        // FIXME: Only accept class typedefs here (i.e., resolve typedef)
        for (const keyword_entry &typedefd: typedef_names)
        {
            if (!strcmp(reinterpret_cast<identifier_token *>(*b)->value, typedefd.identifier) &&
                parent->sees_in_ns(typedefd.declaration, namespace_scope))
            {
                // FIXME: RESOLVE NAO
                node->supplemental.declaration = typedefd.complete_declaration;
                (new syntax_tree_node(syntax_tree_node::TOKEN, node))->ass_token = *b;
                if (++b > maximum_extent) maximum_extent = b;
                *success = true;
                return b;
            }
        }
    }

    node->detach();
    delete node;

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
            if (!strcmp(reinterpret_cast<identifier_token *>(*b)->value, tn.identifier) &&
                parent->sees_in_ns(tn.declaration, namespace_scope))
            {
                syntax_tree_node *node = new syntax_tree_node(syntax_tree_node::TEMPLATE_NAME, parent);
                node->supplemental.declaration = tn.complete_declaration;
                (new syntax_tree_node(syntax_tree_node::TOKEN, node))->ass_token = *b;
                if (++b > maximum_extent) maximum_extent = b;
                *success = true;
                return b;
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
            if (++m > maximum_extent) maximum_extent = m;
            *success = true;
            return m;
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
            if (++m > maximum_extent) maximum_extent = m;
            *success = true;
            return m;
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


static void nested_name_specifier_start_done(syntax_tree_node *node)
{
    auto ci = node->children.begin();

    if ((*ci)->type == syntax_tree_node::TOKEN) // operator("::")
        ++ci;

    if ((*ci)->type == syntax_tree_node::DECLTYPE_SPECIFIER)
        throw format("Please implement decltype-specifier for nested-name-specifier"); // どうしよう〜

    syntax_tree_node *n = (*ci)->children.front();
    if (n->type == syntax_tree_node::SIMPLE_TEMPLATE_ID)
        n = n->children.front(); // template-name

    syntax_tree_node *ns_scope = n->supplemental.declaration ? n->supplemental.declaration->scope_below() : nullptr;
    namespace_scope_stack.push({ns_scope, node->parent->parent});
    namespace_scope = ns_scope;
}


static void nested_name_specifier_repeatable_done(syntax_tree_node *node)
{
    auto ci = node->children.begin();

    if (((*ci)->type == syntax_tree_node::TOKEN) && !strcmp((*ci)->ass_token->content, "template"))
        ++ci;

    syntax_tree_node *n = (*ci)->children.front();
    if (n->type == syntax_tree_node::SIMPLE_TEMPLATE_ID) // this shouldn't even happen
        n = n->children.front(); // template-name
    syntax_tree_node *ns_scope = n->supplemental.declaration;

    if (ns_scope && (ns_scope->type != syntax_tree_node::CLASS_SPECIFIER))
        ns_scope = ns_scope->scope_below();

    namespace_scope_stack.push({ns_scope, node->parent->parent});
    namespace_scope = ns_scope;
}


static void clear_nested_name_specifier(syntax_tree_node *node)
{
    if (namespace_scope_stack.empty())
        return;

    if (node == namespace_scope_stack.top().related)
    {
        namespace_scope_stack.pop();

        if (!namespace_scope_stack.empty())
            namespace_scope = namespace_scope_stack.top().scope;
        else
            namespace_scope = nullptr;
    }
}


static void push_null_namespace(syntax_tree_node *node)
{
    if ((node->parent->type == syntax_tree_node::QUALIFIED_ID) || !namespace_scope)
        return;

    namespace_scope_stack.push({nullptr, node});
    namespace_scope = nullptr;
}


#include "parser-sv-handlers.cxx"


syntax_tree_node *build_syntax_tree(const std::vector<token *> &token_list)
{
    maximum_extent = token_list.begin();

    for (std::list<keyword_entry> *kwl: {&keywords, &typedef_names, &class_names, &template_names, &original_namespace_names})
    {
        for (const keyword_entry &kw: *kwl)
            free(const_cast<char *>(kw.identifier));

        kwl->clear();
    }

    // new and delete are operators; false, nullptr and true are literals.
    for (auto kw: { "alignas", "alignof", "asm", "auto", "bool", "break",
                    "case", "catch", "char", "char16_t", "char32_t", "class",
                    "const", "constexpr", "const_cast", "continue", "decltype",
                    "default", "do", "double", "dynamic_cast", "else", "enum",
                    "explicit", "export", "extern", "float", "for", "friend",
                    "goto", "if", "inline", "int", "long", "mutable",
                    "namespace", "noexcept", "operator", "private", "protected",
                    "public", "register", "reinterpret_cast", "return", "short",
                    "signed", "sizeof", "static", "static_assert",
                    "static_cast", "struct", "switch", "template", "this",
                    "thread_local", "throw", "try", "typedef", "typeid",
                    "typename", "union", "unsigned", "using", "virtual", "void",
                    "volatile", "wchar_t", "while" })
    {
        keywords.push_back({strdup(kw), nullptr, nullptr});
    }

    while (!namespace_scope_stack.empty())
        namespace_scope_stack.pop();

    namespace_scope = nullptr;

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
