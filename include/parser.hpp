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
        };

        syntax_tree_node *parent;
        std::list<syntax_tree_node *> children;
        sv_type type;
        token *ass_token;

        syntax_tree_node(sv_type type, syntax_tree_node *parent = nullptr);
        ~syntax_tree_node(void);

        void detach(void);
};


extern const char *const parser_type_names[];


syntax_tree_node *build_syntax_tree(const std::vector<token *> &token_list);

#endif
