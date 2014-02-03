#ifndef PARSER_HPP
#define PARSER_HPP

#include <list>
#include <vector>

#include "tokenize.hpp"


class syntax_tree_node
{
    public:
        enum sv_type
        {
#include "parser-enum-content.hpp"

            OVERLOADABLE_OPERATOR,
            TRIVIALLY_BALANCED_TOKEN,

            TYPEDEF_NAME,
            CLASS_NAME,
            TEMPLATE_NAME,
        };

        syntax_tree_node *parent;
        std::list<syntax_tree_node *> children;
        sv_type type;
        token *ass_token;
        bool intermediate;

        syntax_tree_node(sv_type type, syntax_tree_node *parent = nullptr, bool intermediate = false);
        ~syntax_tree_node(void);

        void detach(void);
        bool sees(const syntax_tree_node *other) const;
        syntax_tree_node *scope(void) const;
        syntax_tree_node *scope_below(void) const;
        token *first_token(void) const;
        void contract(void);
};


extern const char *const parser_type_names[];


syntax_tree_node *build_syntax_tree(const std::vector<token *> &token_list);

#endif
