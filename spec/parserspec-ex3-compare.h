"translation-unit\n"
"  declaration-seq\n"
"    declaration\n"
"      template-declaration\n"
"        token: Identifier: ( 1: 1) template\n"
"        token: Operator: ( 1: 9) <\n"
"        template-parameter-list\n"
"          template-parameter\n"
"            type-parameter\n"
"              token: Identifier: ( 1:10) typename\n"
"              token: Identifier: ( 1:19) T\n"
"        token: Operator: ( 1:20) >\n"
"        declaration\n"
"          block-declaration\n"
"            simple-declaration\n"
"              decl-specifier-seq\n"
"                decl-specifier\n"
"                  type-specifier\n"
"                    class-specifier\n"
"                      class-head\n"
"                        class-key\n"
"                          token: Identifier: ( 1:22) class\n"
"                        class-head-name\n"
"                          token: Identifier: ( 1:28) a\n"
"                      token: Operator: ( 2: 1) {\n"
"                      token: Operator: ( 3: 1) }\n"
"              token: Operator: ( 3: 2) ;\n"
"    declaration\n"
"      template-declaration\n"
"        token: Identifier: ( 4: 1) template\n"
"        token: Operator: ( 4: 9) <\n"
"        template-parameter-list\n"
"          template-parameter\n"
"            type-parameter\n"
"              token: Identifier: ( 4:10) typename\n"
"              token: Identifier: ( 4:19) T\n"
"        token: Operator: ( 4:20) >\n"
"        declaration\n"
"          block-declaration\n"
"            simple-declaration\n"
"              decl-specifier-seq\n"
"                decl-specifier\n"
"                  type-specifier\n"
"                    class-specifier\n"
"                      class-head\n"
"                        class-key\n"
"                          token: Identifier: ( 4:22) class\n"
"                        class-head-name\n"
"                          token: Identifier: ( 4:28) b\n"
"                      token: Operator: ( 5: 1) {\n"
"                      token: Operator: ( 6: 1) }\n"
"              token: Operator: ( 6: 2) ;\n"
"    declaration\n"
"      function-definition\n"
"        decl-specifier-seq\n"
"          decl-specifier\n"
"            type-specifier\n"
"              trailing-type-specifier\n"
"                simple-type-specifier\n"
"                  token: Identifier: ( 7: 1) auto\n"
"        declarator\n"
"          noptr-declarator\n"
"            declarator-id\n"
"              id-expression\n"
"                unqualified-id\n"
"                  token: Identifier: ( 7: 6) foo\n"
"          parameters-and-qualifiers\n"
"            token: Operator: ( 7: 9) (\n"
"            parameter-declaration-clause\n"
"            token: Operator: ( 7:10) )\n"
"          trailing-return-type\n"
"            token: Operator: ( 7:12) ->\n"
"            trailing-type-specifier-seq\n"
"              trailing-type-specifier\n"
"                simple-type-specifier\n"
"                  decltype-specifier\n"
"                    token: Identifier: ( 7:15) decltype\n"
"                    token: Operator: ( 7:23) (\n"
"                    expression\n"
"                      assignment-expression\n"
"                        conditional-expression\n"
"                          logical-or-expression\n"
"                            logical-and-expression\n"
"                              inclusive-or-expression\n"
"                                exclusive-or-expression\n"
"                                  and-expression\n"
"                                    equality-expression\n"
"                                      relational-expression\n"
"                                        shift-expression\n"
"                                          additive-expression\n"
"                                            multiplicative-expression\n"
"                                              pm-expression\n"
"                                                cast-expression\n"
"                                                  unary-expression\n"
"                                                    new-expression\n"
"                                                      token: Operator: ( 7:24) new\n"
"                                                      new-type-id\n"
"                                                        type-specifier-seq\n"
"                                                          type-specifier\n"
"                                                            trailing-type-specifier\n"
"                                                              simple-type-specifier\n"
"                                                                type-name\n"
"                                                                  simple-template-id\n"
"                                                                    template-name\n"
"                                                                      token: Identifier: ( 7:28) a\n"
"                                                                    token: Operator: ( 7:29) <\n"
"                                                                    template-argument-list\n"
"                                                                      template-argument\n"
"                                                                        id-expression\n"
"                                                                          unqualified-id\n"
"                                                                            template-id\n"
"                                                                              simple-template-id\n"
"                                                                                template-name\n"
"                                                                                  token: Identifier: ( 7:30) b\n"
"                                                                                token: Operator: ( 7:31) <\n"
"                                                                                template-argument-list\n"
"                                                                                  template-argument\n"
"                                                                                    type-id\n"
"                                                                                      type-specifier-seq\n"
"                                                                                        type-specifier\n"
"                                                                                          trailing-type-specifier\n"
"                                                                                            simple-type-specifier\n"
"                                                                                              token: Identifier: ( 7:32) int\n"
"                                                                                token: Operator: ( 7:35) >\n"
"                                                                    token: Operator: ( 7:36) >\n"
"                                                        new-declarator\n"
"                                                          noptr-new-declarator\n"
"                                                            token: Operator: ( 7:37) [\n"
"                                                            expression\n"
"                                                              assignment-expression\n"
"                                                                conditional-expression\n"
"                                                                  logical-or-expression\n"
"                                                                    logical-and-expression\n"
"                                                                      inclusive-or-expression\n"
"                                                                        exclusive-or-expression\n"
"                                                                          and-expression\n"
"                                                                            equality-expression\n"
"                                                                              relational-expression\n"
"                                                                                shift-expression\n"
"                                                                                  additive-expression\n"
"                                                                                    multiplicative-expression\n"
"                                                                                      pm-expression\n"
"                                                                                        cast-expression\n"
"                                                                                          unary-expression\n"
"                                                                                            postfix-expression\n"
"                                                                                              primary-expression\n"
"                                                                                                literal\n"
"                                                                                                  token: Integer literal: ( 7:38) 42\n"
"                                                            token: Operator: ( 7:40) ]\n"
"                    token: Operator: ( 7:41) )\n"
"        function-body\n"
"          compound-statement\n"
"            token: Operator: ( 8: 1) {\n"
"            statement-seq\n"
"              statement\n"
"                declaration-statement\n"
"                  block-declaration\n"
"                    simple-declaration\n"
"                      decl-specifier-seq\n"
"                        decl-specifier\n"
"                          type-specifier\n"
"                            trailing-type-specifier\n"
"                              simple-type-specifier\n"
"                                token: Identifier: ( 9: 5) int\n"
"                      init-declarator-list\n"
"                        init-declarator\n"
"                          declarator\n"
"                            ptr-declarator\n"
"                              noptr-declarator\n"
"                                declarator-id\n"
"                                  id-expression\n"
"                                    unqualified-id\n"
"                                      token: Identifier: ( 9: 9) x\n"
"                          initializer\n"
"                            token: Operator: ( 9:10) (\n"
"                            expression-list\n"
"                              initializer-list\n"
"                                initializer-clause\n"
"                                  assignment-expression\n"
"                                    conditional-expression\n"
"                                      logical-or-expression\n"
"                                        logical-and-expression\n"
"                                          inclusive-or-expression\n"
"                                            exclusive-or-expression\n"
"                                              and-expression\n"
"                                                equality-expression\n"
"                                                  relational-expression\n"
"                                                    shift-expression\n"
"                                                      additive-expression\n"
"                                                        multiplicative-expression\n"
"                                                          pm-expression\n"
"                                                            cast-expression\n"
"                                                              unary-expression\n"
"                                                                postfix-expression\n"
"                                                                  primary-expression\n"
"                                                                    token: Operator: ( 9:11) (\n"
"                                                                    expression\n"
"                                                                      assignment-expression\n"
"                                                                        conditional-expression\n"
"                                                                          logical-or-expression\n"
"                                                                            logical-and-expression\n"
"                                                                              inclusive-or-expression\n"
"                                                                                exclusive-or-expression\n"
"                                                                                  and-expression\n"
"                                                                                    equality-expression\n"
"                                                                                      relational-expression\n"
"                                                                                        shift-expression\n"
"                                                                                          additive-expression\n"
"                                                                                            multiplicative-expression\n"
"                                                                                              pm-expression\n"
"                                                                                                cast-expression\n"
"                                                                                                  unary-expression\n"
"                                                                                                    postfix-expression\n"
"                                                                                                      primary-expression\n"
"                                                                                                        literal\n"
"                                                                                                          token: Integer literal: ( 9:12) 4\n"
"                                                                                          shift-operator\n"
"                                                                                            token: Operator: ( 9:14) <<\n"
"                                                                                          additive-expression\n"
"                                                                                            multiplicative-expression\n"
"                                                                                              pm-expression\n"
"                                                                                                cast-expression\n"
"                                                                                                  unary-expression\n"
"                                                                                                    postfix-expression\n"
"                                                                                                      primary-expression\n"
"                                                                                                        literal\n"
"                                                                                                          token: Integer literal: ( 9:17) 2\n"
"                                                                    token: Operator: ( 9:18) )\n"
"                                                      shift-operator\n"
"                                                        token: Operator: ( 9:20) >>\n"
"                                                      additive-expression\n"
"                                                        multiplicative-expression\n"
"                                                          pm-expression\n"
"                                                            cast-expression\n"
"                                                              unary-expression\n"
"                                                                postfix-expression\n"
"                                                                  primary-expression\n"
"                                                                    literal\n"
"                                                                      token: Integer literal: ( 9:23) 3\n"
"                            token: Operator: ( 9:24) )\n"
"                      token: Operator: ( 9:25) ;\n"
"              statement\n"
"                expression-statement\n"
"                  expression\n"
"                    assignment-expression\n"
"                      logical-or-expression\n"
"                        logical-and-expression\n"
"                          inclusive-or-expression\n"
"                            exclusive-or-expression\n"
"                              and-expression\n"
"                                equality-expression\n"
"                                  relational-expression\n"
"                                    shift-expression\n"
"                                      additive-expression\n"
"                                        multiplicative-expression\n"
"                                          pm-expression\n"
"                                            cast-expression\n"
"                                              unary-expression\n"
"                                                postfix-expression\n"
"                                                  primary-expression\n"
"                                                    id-expression\n"
"                                                      unqualified-id\n"
"                                                        token: Identifier: (10: 5) x\n"
"                      assignment-operator\n"
"                        token: Operator: (10: 7) >>=\n"
"                      initializer-clause\n"
"                        assignment-expression\n"
"                          conditional-expression\n"
"                            logical-or-expression\n"
"                              logical-and-expression\n"
"                                inclusive-or-expression\n"
"                                  exclusive-or-expression\n"
"                                    and-expression\n"
"                                      equality-expression\n"
"                                        relational-expression\n"
"                                          shift-expression\n"
"                                            additive-expression\n"
"                                              multiplicative-expression\n"
"                                                pm-expression\n"
"                                                  cast-expression\n"
"                                                    unary-expression\n"
"                                                      postfix-expression\n"
"                                                        primary-expression\n"
"                                                          literal\n"
"                                                            token: Integer literal: (10:11) 1\n"
"                  token: Operator: (10:12) ;\n"
"              statement\n"
"                jump-statement\n"
"                  token: Identifier: (11: 5) return\n"
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
"                                                  new-expression\n"
"                                                    token: Operator: (11:12) new\n"
"                                                    new-type-id\n"
"                                                      type-specifier-seq\n"
"                                                        type-specifier\n"
"                                                          trailing-type-specifier\n"
"                                                            simple-type-specifier\n"
"                                                              type-name\n"
"                                                                simple-template-id\n"
"                                                                  template-name\n"
"                                                                    token: Identifier: (11:16) a\n"
"                                                                  token: Operator: (11:17) <\n"
"                                                                  template-argument-list\n"
"                                                                    template-argument\n"
"                                                                      id-expression\n"
"                                                                        unqualified-id\n"
"                                                                          template-id\n"
"                                                                            simple-template-id\n"
"                                                                              template-name\n"
"                                                                                token: Identifier: (11:18) b\n"
"                                                                              token: Operator: (11:19) <\n"
"                                                                              template-argument-list\n"
"                                                                                template-argument\n"
"                                                                                  type-id\n"
"                                                                                    type-specifier-seq\n"
"                                                                                      type-specifier\n"
"                                                                                        trailing-type-specifier\n"
"                                                                                          simple-type-specifier\n"
"                                                                                            token: Identifier: (11:20) int\n"
"                                                                              token: Operator: (11:23) >\n"
"                                                                  token: Operator: (11:24) >\n"
"                                                      new-declarator\n"
"                                                        noptr-new-declarator\n"
"                                                          token: Operator: (11:25) [\n"
"                                                          expression\n"
"                                                            assignment-expression\n"
"                                                              conditional-expression\n"
"                                                                logical-or-expression\n"
"                                                                  logical-and-expression\n"
"                                                                    inclusive-or-expression\n"
"                                                                      exclusive-or-expression\n"
"                                                                        and-expression\n"
"                                                                          equality-expression\n"
"                                                                            relational-expression\n"
"                                                                              shift-expression\n"
"                                                                                additive-expression\n"
"                                                                                  multiplicative-expression\n"
"                                                                                    pm-expression\n"
"                                                                                      cast-expression\n"
"                                                                                        unary-expression\n"
"                                                                                          postfix-expression\n"
"                                                                                            primary-expression\n"
"                                                                                              id-expression\n"
"                                                                                                unqualified-id\n"
"                                                                                                  token: Identifier: (11:26) x\n"
"                                                          token: Operator: (11:27) ]\n"
"                  token: Operator: (11:28) ;\n"
"            token: Operator: (12: 1) }\n"