"translation-unit\n"
"  declaration-seq\n"
"    declaration\n"
"      block-declaration\n"
"        simple-declaration\n"
"          decl-specifier-seq\n"
"            decl-specifier\n"
"              storage-class-specifier\n"
"                token: Identifier: ( 1: 1) extern\n"
"            decl-specifier\n"
"              type-specifier\n"
"                trailing-type-specifier\n"
"                  simple-type-specifier\n"
"                    token: Identifier: ( 1: 8) int\n"
"          init-declarator-list\n"
"            init-declarator\n"
"              declarator\n"
"                ptr-declarator\n"
"                  noptr-declarator\n"
"                    declarator-id\n"
"                      id-expression\n"
"                        unqualified-id\n"
"                          token: Identifier: ( 1:12) printf\n"
"                    parameters-and-qualifiers\n"
"                      token: Operator: ( 1:18) (\n"
"                      parameter-declaration-clause\n"
"                        parameter-declaration-list\n"
"                          parameter-declaration\n"
"                            decl-specifier-seq\n"
"                              decl-specifier\n"
"                                type-specifier\n"
"                                  trailing-type-specifier\n"
"                                    cv_qualifier\n"
"                                      token: Identifier: ( 1:19) const\n"
"                              decl-specifier\n"
"                                type-specifier\n"
"                                  trailing-type-specifier\n"
"                                    simple-type-specifier\n"
"                                      token: Identifier: ( 1:25) char\n"
"                            declarator\n"
"                              ptr-declarator\n"
"                                ptr-operator\n"
"                                  token: Operator: ( 1:30) *\n"
"                                noptr-declarator\n"
"                                  declarator-id\n"
"                                    id-expression\n"
"                                      unqualified-id\n"
"                                        token: Identifier: ( 1:31) format\n"
"                          token: Operator: ( 1:37) ,\n"
"                        token: Operator: ( 1:37) ,\n"
"                        token: Operator: ( 1:39) ...\n"
"                      token: Operator: ( 1:42) )\n"
"          token: Operator: ( 1:43) ;\n"
"    declaration\n"
"      block-declaration\n"
"        simple-declaration\n"
"          decl-specifier-seq\n"
"            decl-specifier\n"
"              type-specifier\n"
"                trailing-type-specifier\n"
"                  elaborated-type-specifier\n"
"                    class-key\n"
"                      token: Identifier: ( 2: 1) class\n"
"                    token: Identifier: ( 2: 7) b\n"
"          token: Operator: ( 2: 8) ;\n"
"    declaration\n"
"      block-declaration\n"
"        simple-declaration\n"
"          decl-specifier-seq\n"
"            decl-specifier\n"
"              type-specifier\n"
"                class-specifier\n"
"                  class-head\n"
"                    class-key\n"
"                      token: Identifier: ( 3: 1) class\n"
"                    class-head-name\n"
"                      token: Identifier: ( 3: 7) c\n"
"                  token: Operator: ( 4: 1) {\n"
"                  token: Operator: ( 5: 1) }\n"
"          token: Operator: ( 5: 2) ;\n"
"    declaration\n"
"      template-declaration\n"
"        token: Identifier: ( 6: 1) template\n"
"        token: Operator: ( 6: 9) <\n"
"        template-parameter-list\n"
"          template-parameter\n"
"            type-parameter\n"
"              token: Identifier: ( 6:10) typename\n"
"              token: Identifier: ( 6:19) T\n"
"        token: Operator: ( 6:20) >\n"
"        declaration\n"
"          block-declaration\n"
"            simple-declaration\n"
"              decl-specifier-seq\n"
"                decl-specifier\n"
"                  type-specifier\n"
"                    class-specifier\n"
"                      class-head\n"
"                        class-key\n"
"                          token: Identifier: ( 6:22) class\n"
"                        class-head-name\n"
"                          token: Identifier: ( 6:28) d\n"
"                      token: Operator: ( 7: 1) {\n"
"                      token: Operator: ( 8: 1) }\n"
"              token: Operator: ( 8: 2) ;\n"
"    declaration\n"
"      function-definition\n"
"        decl-specifier-seq\n"
"          decl-specifier\n"
"            type-specifier\n"
"              trailing-type-specifier\n"
"                simple-type-specifier\n"
"                  token: Identifier: ( 9: 1) int\n"
"        declarator\n"
"          ptr-declarator\n"
"            noptr-declarator\n"
"              declarator-id\n"
"                id-expression\n"
"                  unqualified-id\n"
"                    token: Identifier: ( 9: 5) main\n"
"              parameters-and-qualifiers\n"
"                token: Operator: ( 9: 9) (\n"
"                parameter-declaration-clause\n"
"                  parameter-declaration-list\n"
"                    parameter-declaration\n"
"                      decl-specifier-seq\n"
"                        decl-specifier\n"
"                          type-specifier\n"
"                            trailing-type-specifier\n"
"                              simple-type-specifier\n"
"                                token: Identifier: ( 9:10) int\n"
"                      declarator\n"
"                        ptr-declarator\n"
"                          noptr-declarator\n"
"                            declarator-id\n"
"                              id-expression\n"
"                                unqualified-id\n"
"                                  token: Identifier: ( 9:14) argc\n"
"                    token: Operator: ( 9:18) ,\n"
"                    parameter-declaration\n"
"                      decl-specifier-seq\n"
"                        decl-specifier\n"
"                          type-specifier\n"
"                            trailing-type-specifier\n"
"                              simple-type-specifier\n"
"                                token: Identifier: ( 9:20) char\n"
"                      declarator\n"
"                        ptr-declarator\n"
"                          ptr-operator\n"
"                            token: Operator: ( 9:25) *\n"
"                          noptr-declarator\n"
"                            declarator-id\n"
"                              id-expression\n"
"                                unqualified-id\n"
"                                  token: Identifier: ( 9:26) argv\n"
"                            token: Operator: ( 9:30) [\n"
"                            token: Operator: ( 9:31) ]\n"
"                token: Operator: ( 9:32) )\n"
"        function-body\n"
"          compound-statement\n"
"            token: Operator: (10: 1) {\n"
"            statement-seq\n"
"              statement\n"
"                declaration-statement\n"
"                  block-declaration\n"
"                    simple-declaration\n"
"                      decl-specifier-seq\n"
"                        decl-specifier\n"
"                          token: Identifier: (11: 5) typedef\n"
"                        decl-specifier\n"
"                          type-specifier\n"
"                            trailing-type-specifier\n"
"                              simple-type-specifier\n"
"                                token: Identifier: (11:13) int\n"
"                      init-declarator-list\n"
"                        init-declarator\n"
"                          declarator\n"
"                            ptr-declarator\n"
"                              noptr-declarator\n"
"                                declarator-id\n"
"                                  id-expression\n"
"                                    unqualified-id\n"
"                                      token: Identifier: (11:17) a\n"
"                      token: Operator: (11:18) ;\n"
"              statement\n"
"                expression-statement\n"
"                  expression\n"
"                    assignment-expression\n"
"                      conditional-expression\n"
"                        logical-or-expression\n"
"                          logical-and-expression\n"
"                            inclusive-or-expression\n"
"                              exclusive-or-expression\n"
"                                and-expression\n"
"                                  equality-expression\n"
"                                    relational-expression\n"
"                                      shift-expression\n"
"                                        additive-expression\n"
"                                          multiplicative-expression\n"
"                                            pm-expression\n"
"                                              cast-expression\n"
"                                                token: Operator: (12: 5) (\n"
"                                                type-id\n"
"                                                  type-specifier-seq\n"
"                                                    type-specifier\n"
"                                                      trailing-type-specifier\n"
"                                                        simple-type-specifier\n"
"                                                          token: Identifier: (12: 6) void\n"
"                                                token: Operator: (12:10) )\n"
"                                                unary-expression\n"
"                                                  postfix-expression\n"
"                                                    primary-expression\n"
"                                                      id-expression\n"
"                                                        unqualified-id\n"
"                                                          token: Identifier: (12:11) argc\n"
"                  token: Operator: (12:15) ;\n"
"              statement\n"
"                expression-statement\n"
"                  expression\n"
"                    assignment-expression\n"
"                      conditional-expression\n"
"                        logical-or-expression\n"
"                          logical-and-expression\n"
"                            inclusive-or-expression\n"
"                              exclusive-or-expression\n"
"                                and-expression\n"
"                                  equality-expression\n"
"                                    relational-expression\n"
"                                      shift-expression\n"
"                                        additive-expression\n"
"                                          multiplicative-expression\n"
"                                            pm-expression\n"
"                                              cast-expression\n"
"                                                token: Operator: (13: 5) (\n"
"                                                type-id\n"
"                                                  type-specifier-seq\n"
"                                                    type-specifier\n"
"                                                      trailing-type-specifier\n"
"                                                        simple-type-specifier\n"
"                                                          token: Identifier: (13: 6) void\n"
"                                                token: Operator: (13:10) )\n"
"                                                unary-expression\n"
"                                                  postfix-expression\n"
"                                                    primary-expression\n"
"                                                      id-expression\n"
"                                                        unqualified-id\n"
"                                                          token: Identifier: (13:11) argv\n"
"                  token: Operator: (13:15) ;\n"
"              statement\n"
"                declaration-statement\n"
"                  block-declaration\n"
"                    simple-declaration\n"
"                      decl-specifier-seq\n"
"                        decl-specifier\n"
"                          type-specifier\n"
"                            trailing-type-specifier\n"
"                              simple-type-specifier\n"
"                                type-name\n"
"                                  typedef-name\n"
"                                    token: Identifier: (14: 5) a\n"
"                      init-declarator-list\n"
"                        init-declarator\n"
"                          declarator\n"
"                            ptr-declarator\n"
"                              noptr-declarator\n"
"                                declarator-id\n"
"                                  id-expression\n"
"                                    unqualified-id\n"
"                                      token: Identifier: (14: 7) i\n"
"                      token: Operator: (14: 8) ;\n"
"              statement\n"
"                declaration-statement\n"
"                  block-declaration\n"
"                    simple-declaration\n"
"                      decl-specifier-seq\n"
"                        decl-specifier\n"
"                          type-specifier\n"
"                            trailing-type-specifier\n"
"                              simple-type-specifier\n"
"                                type-name\n"
"                                  class-name\n"
"                                    token: Identifier: (15: 5) b\n"
"                      init-declarator-list\n"
"                        init-declarator\n"
"                          declarator\n"
"                            ptr-declarator\n"
"                              ptr-operator\n"
"                                token: Operator: (15:10) *\n"
"                              noptr-declarator\n"
"                                declarator-id\n"
"                                  id-expression\n"
"                                    unqualified-id\n"
"                                      token: Identifier: (15:11) j\n"
"                          initializer\n"
"                            brace-or-equal-initializer\n"
"                              token: Operator: (15:13) =\n"
"                              initializer-clause\n"
"                                assignment-expression\n"
"                                  conditional-expression\n"
"                                    logical-or-expression\n"
"                                      logical-and-expression\n"
"                                        inclusive-or-expression\n"
"                                          exclusive-or-expression\n"
"                                            and-expression\n"
"                                              equality-expression\n"
"                                                relational-expression\n"
"                                                  shift-expression\n"
"                                                    additive-expression\n"
"                                                      multiplicative-expression\n"
"                                                        pm-expression\n"
"                                                          cast-expression\n"
"                                                            unary-expression\n"
"                                                              postfix-expression\n"
"                                                                primary-expression\n"
"                                                                  literal\n"
"                                                                    token: Pointer literal: (15:15) (nil)\n"
"                      token: Operator: (15:22) ;\n"
"              statement\n"
"                declaration-statement\n"
"                  block-declaration\n"
"                    simple-declaration\n"
"                      decl-specifier-seq\n"
"                        decl-specifier\n"
"                          type-specifier\n"
"                            trailing-type-specifier\n"
"                              simple-type-specifier\n"
"                                type-name\n"
"                                  class-name\n"
"                                    token: Identifier: (16: 5) c\n"
"                      init-declarator-list\n"
"                        init-declarator\n"
"                          declarator\n"
"                            ptr-declarator\n"
"                              noptr-declarator\n"
"                                declarator-id\n"
"                                  id-expression\n"
"                                    unqualified-id\n"
"                                      token: Identifier: (16: 7) k\n"
"                      token: Operator: (16: 8) ;\n"
"              statement\n"
"                declaration-statement\n"
"                  block-declaration\n"
"                    simple-declaration\n"
"                      decl-specifier-seq\n"
"                        decl-specifier\n"
"                          type-specifier\n"
"                            trailing-type-specifier\n"
"                              simple-type-specifier\n"
"                                type-name\n"
"                                  simple-template-id\n"
"                                    template-name\n"
"                                      token: Identifier: (19: 9) d\n"
"                                    token: Operator: (19:10) <\n"
"                                    template-argument-list\n"
"                                      template-argument\n"
"                                        type-id\n"
"                                          type-specifier-seq\n"
"                                            type-specifier\n"
"                                              trailing-type-specifier\n"
"                                                simple-type-specifier\n"
"                                                  type-name\n"
"                                                    class-name\n"
"                                                      token: Identifier: (19:11) c\n"
"                                    token: Operator: (19:12) >\n"
"                      init-declarator-list\n"
"                        init-declarator\n"
"                          declarator\n"
"                            ptr-declarator\n"
"                              noptr-declarator\n"
"                                declarator-id\n"
"                                  id-expression\n"
"                                    unqualified-id\n"
"                                      token: Identifier: (19:14) m\n"
"                      token: Operator: (19:15) ;\n"
"              statement\n"
"                expression-statement\n"
"                  expression\n"
"                    assignment-expression\n"
"                      conditional-expression\n"
"                        logical-or-expression\n"
"                          logical-and-expression\n"
"                            inclusive-or-expression\n"
"                              exclusive-or-expression\n"
"                                and-expression\n"
"                                  equality-expression\n"
"                                    relational-expression\n"
"                                      shift-expression\n"
"                                        additive-expression\n"
"                                          multiplicative-expression\n"
"                                            pm-expression\n"
"                                              cast-expression\n"
"                                                unary-expression\n"
"                                                  postfix-expression\n"
"                                                    primary-expression\n"
"                                                      id-expression\n"
"                                                        unqualified-id\n"
"                                                          token: Identifier: (20: 5) printf\n"
"                                                    token: Operator: (20:11) (\n"
"                                                    expression-list\n"
"                                                      initializer-list\n"
"                                                        initializer-clause\n"
"                                                          assignment-expression\n"
"                                                            conditional-expression\n"
"                                                              logical-or-expression\n"
"                                                                logical-and-expression\n"
"                                                                  inclusive-or-expression\n"
"                                                                    exclusive-or-expression\n"
"                                                                      and-expression\n"
"                                                                        equality-expression\n"
"                                                                          relational-expression\n"
"                                                                            shift-expression\n"
"                                                                              additive-expression\n"
"                                                                                multiplicative-expression\n"
"                                                                                  pm-expression\n"
"                                                                                    cast-expression\n"
"                                                                                      unary-expression\n"
"                                                                                        postfix-expression\n"
"                                                                                          primary-expression\n"
"                                                                                            literal\n"
"                                                                                              token: String literal: (20:12) \"ohai wurld %g %g %llu %llu %Lg\"\n"
"                                                        token: Operator: (20:44) ,\n"
"                                                        initializer-clause\n"
"                                                          assignment-expression\n"
"                                                            conditional-expression\n"
"                                                              logical-or-expression\n"
"                                                                logical-and-expression\n"
"                                                                  inclusive-or-expression\n"
"                                                                    exclusive-or-expression\n"
"                                                                      and-expression\n"
"                                                                        equality-expression\n"
"                                                                          relational-expression\n"
"                                                                            shift-expression\n"
"                                                                              additive-expression\n"
"                                                                                multiplicative-expression\n"
"                                                                                  pm-expression\n"
"                                                                                    cast-expression\n"
"                                                                                      unary-expression\n"
"                                                                                        postfix-expression\n"
"                                                                                          primary-expression\n"
"                                                                                            literal\n"
"                                                                                              token: Float literal: (20:46) 3.25\n"
"                                                        token: Operator: (20:51) ,\n"
"                                                        initializer-clause\n"
"                                                          assignment-expression\n"
"                                                            conditional-expression\n"
"                                                              logical-or-expression\n"
"                                                                logical-and-expression\n"
"                                                                  inclusive-or-expression\n"
"                                                                    exclusive-or-expression\n"
"                                                                      and-expression\n"
"                                                                        equality-expression\n"
"                                                                          relational-expression\n"
"                                                                            shift-expression\n"
"                                                                              additive-expression\n"
"                                                                                multiplicative-expression\n"
"                                                                                  pm-expression\n"
"                                                                                    cast-expression\n"
"                                                                                      unary-expression\n"
"                                                                                        postfix-expression\n"
"                                                                                          primary-expression\n"
"                                                                                            literal\n"
"                                                                                              token: Float literal: (20:53) 169.031\n"
"                                                        token: Operator: (20:62) ,\n"
"                                                        initializer-clause\n"
"                                                          assignment-expression\n"
"                                                            conditional-expression\n"
"                                                              logical-or-expression\n"
"                                                                logical-and-expression\n"
"                                                                  inclusive-or-expression\n"
"                                                                    exclusive-or-expression\n"
"                                                                      and-expression\n"
"                                                                        equality-expression\n"
"                                                                          relational-expression\n"
"                                                                            shift-expression\n"
"                                                                              additive-expression\n"
"                                                                                multiplicative-expression\n"
"                                                                                  pm-expression\n"
"                                                                                    cast-expression\n"
"                                                                                      unary-expression\n"
"                                                                                        postfix-expression\n"
"                                                                                          primary-expression\n"
"                                                                                            literal\n"
"                                                                                              token: Integer literal: (20:64) 42\n"
"                                                        token: Operator: (20:69) ,\n"
"                                                        initializer-clause\n"
"                                                          assignment-expression\n"
"                                                            conditional-expression\n"
"                                                              logical-or-expression\n"
"                                                                logical-and-expression\n"
"                                                                  inclusive-or-expression\n"
"                                                                    exclusive-or-expression\n"
"                                                                      and-expression\n"
"                                                                        equality-expression\n"
"                                                                          relational-expression\n"
"                                                                            shift-expression\n"
"                                                                              additive-expression\n"
"                                                                                multiplicative-expression\n"
"                                                                                  pm-expression\n"
"                                                                                    cast-expression\n"
"                                                                                      unary-expression\n"
"                                                                                        postfix-expression\n"
"                                                                                          primary-expression\n"
"                                                                                            literal\n"
"                                                                                              token: Integer literal: (20:71) 42\n"
"                                                        token: Operator: (20:76) ,\n"
"                                                        initializer-clause\n"
"                                                          assignment-expression\n"
"                                                            conditional-expression\n"
"                                                              logical-or-expression\n"
"                                                                logical-and-expression\n"
"                                                                  inclusive-or-expression\n"
"                                                                    exclusive-or-expression\n"
"                                                                      and-expression\n"
"                                                                        equality-expression\n"
"                                                                          relational-expression\n"
"                                                                            shift-expression\n"
"                                                                              additive-expression\n"
"                                                                                multiplicative-expression\n"
"                                                                                  pm-expression\n"
"                                                                                    cast-expression\n"
"                                                                                      unary-expression\n"
"                                                                                        unary-operator\n"
"                                                                                          token: Operator: (20:78) -\n"
"                                                                                        cast-expression\n"
"                                                                                          unary-expression\n"
"                                                                                            postfix-expression\n"
"                                                                                              primary-expression\n"
"                                                                                                literal\n"
"                                                                                                  token: Float literal: (20:79) 0\n"
"                                                    token: Operator: (20:85) )\n"
"                  token: Operator: (20:86) ;\n"
"              statement\n"
"                jump-statement\n"
"                  token: Identifier: (21: 5) return\n"
"                  expression\n"
"                    assignment-expression\n"
"                      conditional-expression\n"
"                        logical-or-expression\n"
"                          logical-and-expression\n"
"                            inclusive-or-expression\n"
"                              exclusive-or-expression\n"
"                                and-expression\n"
"                                  equality-expression\n"
"                                    relational-expression\n"
"                                      shift-expression\n"
"                                        additive-expression\n"
"                                          multiplicative-expression\n"
"                                            pm-expression\n"
"                                              cast-expression\n"
"                                                unary-expression\n"
"                                                  postfix-expression\n"
"                                                    primary-expression\n"
"                                                      literal\n"
"                                                        token: Integer literal: (21:12) 0\n"
"                  token: Operator: (21:13) ;\n"
"            token: Operator: (22: 1) }\n"
