#!/usr/bin/env ruby
# coding: utf-8

svs = Hash.new
current = nil

IO.readlines('src/syntax').each do |line|
    line.strip!
    next if line.empty? || line[0..1] == '--'

    if line[-1] == ':'
        if current
            current = line[0..-2]
        else
            current = '!' + line[0..-2]
        end

        if svs[current]
            $stderr.puts("Syntactic variable #{current} redefined")
            exit 1
        end
        svs[current] = Array.new
        next
    end

    if !current
        $stderr.puts('Cannot define a rule independent of a syntactic variable')
        exit 1
    end

    svs[current] << line.split
end


def const(name)
    name.upcase.gsub('-', '_').sub('!', '')
end

def var(name)
    name.gsub('-', '_').sub('!', '')
end


File.open('include/parser-enum-content.hpp', 'w') do |f|
    f.puts('TOKEN,')
    f.puts
    svs.each_key do |sv|
        f.puts("#{const sv},")
    end
end

File.open('src/parser-enum-names.cpp', 'w') do |f|
    f.puts('#include "parser.hpp"')
    f.puts
    f.puts('const char *const parser_type_names[] = {')
    f.puts('    "token",')
    svs.each_key do |sv|
        f.puts("    \"#{sv.sub('!', '')}\",")
    end
    f.puts('#include "parser-enum-names-supplement.cxx"')
    f.puts('};')
end

File.open('src/parser-sv-prototypes.cxx', 'w') do |f|
    svs.each_key do |sv|
        if sv[0] == '!'
            f.puts("static syntax_tree_node *sv_#{var sv}(range_t b, range_t e);")
        else
            f.puts("static range_t sv_#{var sv}(syntax_tree_node *parent, range_t b, range_t e);")
        end
    end
end

File.open('src/parser-sv-handlers.cxx', 'w') do |f|
    svs.each_key do |sv|
        if sv[0] == '!'
            f.puts("static syntax_tree_node *sv_#{var sv}(range_t b, range_t e)")
        else
            f.puts("static range_t sv_#{var sv}(syntax_tree_node *parent, range_t b, range_t e)")
        end
        f.puts('{')
        f.puts("    if (b == e) return #{sv[0] == '!' ? 'nullptr' : 'b'};")
        #f.puts("    printf(\"Visiting #{sv.sub('!', '')} for token %s\\n\", (*b)->content);")
        f.puts
        f.puts("    syntax_tree_node *node = new syntax_tree_node(syntax_tree_node::#{const sv}#{sv[0] == '!' ? '' : ', parent'});")

        f.puts
        f.puts("    // #{sv.sub('!', '')}:")

        svs[sv].each do |rule|
            f.puts("    //   #{rule * ' '}")
        end

        f.puts

        f.puts('    range_t m = b, n;')

        i = 0

        svs[sv].each do |rule|
            f.puts
            f.puts
            f.puts("sv_#{var sv}_part_#{i}:")
            if i > 0
                f.puts('    for (syntax_tree_node *c: node->children) delete c; node->children.clear();')
                f.puts('    m = b;')
                f.puts
            end

            had_non_optional = false
            in_loop = false

            rule.each do |part|
                if part == '{'
                    f.puts('    {')
                    f.puts('    range_t l;')
                    f.puts('    do')
                    f.puts('    {')
                    f.puts('    l = m;')
                    f.puts
                    in_loop = true
                    next
                elsif part == '}'
                    in_loop = false
                    f.puts('    } while (l != m);')
                    f.puts('    }')
                    next
                end


                if part[0] == '[' && part[-1] == ']'
                    part = part[1..-2]
                    optional = true
                else
                    optional = false
                    had_non_optional = true
                end

                if part.include?('(')
                    match = /^([\w-]+)\((.*)\)$/.match(part)
                    if !match
                        $stderr.puts("Cannot parse #{part}")
                        exit 1
                    end

                    kind = match[1]
                    condition = match[2]

                    if condition.empty?
                        f.puts("    if ((m != e) && ((*m)->type == token::#{const kind}))")
                    elsif condition[0] == '"'
                        f.puts("    if ((m != e) && ((*m)->type == token::#{const kind}) && !strcmp(reinterpret_cast<#{var kind}_token *>(*m)->value, #{condition}))")
                    else
                        # FIXME (@lit-integer)
                        f.puts("    if ((m != e) && ((*m)->type == token::#{const kind}) && (reinterpret_cast<#{var kind}_token *>(*m)->value#{kind == 'lit-integer' ? '.s' : ''} == #{condition}))")
                    end
                    f.puts('    {')
                    f.puts('        syntax_tree_node *tok_node = new syntax_tree_node(syntax_tree_node::TOKEN, node);')
                    f.puts('        tok_node->ass_token = *m;')
                    f.puts('        ++m;')
                    f.puts('    }')
                    f.puts('    else') unless optional
                else
                    f.puts("    m = sv_#{var part}(node, (n = m), e);")
                    f.puts('    if (m == n)') unless optional
                end

                if !optional
                    if in_loop
                        f.puts('    {')
                        f.puts('        m = l;')
                        f.puts('        break;')
                        f.puts('    }')
                    else
                        f.puts("        goto sv_#{var sv}_part_#{i + 1};")
                    end
                end

                f.puts
            end

            # If empty, go on (if everything fails, it will be empty anyway)
            if !had_non_optional
                f.puts('    if (m != b)')
                f.puts("        return #{sv[0] == '!' ? 'node' : 'm'};")
            else
                f.puts("    return #{sv[0] == '!' ? 'node' : 'm'};")
            end

            i += 1
        end

        f.puts
        f.puts
        f.puts("sv_#{var sv}_part_#{i}:")
        f.puts('    node->detach();')
        f.puts('    delete node;')
        f.puts("    return #{sv[0] == '!' ? 'node' : 'b'};")
        f.puts('}')

        f.puts
        f.puts
    end
end


# potential call chain
pcc = Hash.new
svs.each do |sv, rules|
    pcc[sv] = Array.new
    rules.each do |rule|
        leave_loop_nao = false
        rule.each do |part|
            if leave_loop_nao
                leave_loop_nao = false if part == '}'
                next
            end

            if part.include?('(')
                leave_loop_nao = true
                next
            end

            next if part.include?('{')

            if part[0] == '['
                pcc[sv] << part[1..-2]
            else
                pcc[sv] << part
                leave_loop_nao = true
            end
        end
    end
    pcc[sv].uniq!
end

pcc.each do |sv, subs|
    trans = subs
    addishornal = subs

    while !addishornal.empty?
        if trans.include?(sv)
            puts("Warning: Potential head recursion for #{sv}.")
            break
        end

        otrans = trans
        addishornal.each do |sub|
            trans += pcc[sub] if pcc[sub]
        end
        trans.uniq!

        addishornal = trans - otrans
    end
end
