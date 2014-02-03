#!/usr/bin/env ruby
# coding: utf-8

svs = Hash.new
post_hooks = Hash.new
post_modify = Hash.new
is_intermediate = Hash.new
current = nil

def missing_parameter_for(attribute)
    $stderr.puts("Attribute #{attribute} requires a parameter")
    exit 1
end

IO.readlines('src/syntax').each do |line|
    line.strip!
    next if line.empty? || line[0..1] == '--'

    spl = line.split

    if spl[0][-1] == ':'
        line = spl[0]
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
        spl[1..-1].each do |moar|
            match = /^([\w-]+)\((\w*)\)$/.match(moar)
            if match
                attribute = match[1]
                parameter = match[2].empty? ? nil : match[2]
            else
                attribute = moar
                parameter = nil
            end
            case attribute
            when 'post-hook'
                missing_parameter_for 'post-hook' unless parameter
                post_hooks[current] = parameter
            when 'intermediate'
                is_intermediate[current] = true
            when 'post-modify'
                missing_parameter_for 'post-modify' unless parameter
                post_modify[current] = parameter
            else
                $stderr.puts("Unknown attribute #{match[1]}")
                exit 1
            end
        end

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
            f.puts("static syntax_tree_node *sv_#{var sv}(range_t b, range_t e, bool *success);")
        else
            f.puts("static range_t sv_#{var sv}(syntax_tree_node *parent, range_t b, range_t e, bool *success);")
        end
    end
end

File.open('src/parser-sv-handlers.cxx', 'w') do |f|
    svs.each_key do |sv|
        if sv[0] == '!'
            f.puts("static syntax_tree_node *sv_#{var sv}(range_t b, range_t e, bool *success)")
        else
            f.puts("static range_t sv_#{var sv}(syntax_tree_node *parent, range_t b, range_t e, bool *success)")
        end
        f.puts('{')
        f.puts("    bool could_parse;")
        #f.puts("    printf(\"Visiting #{sv.sub('!', '')} for token %s (from %s)\\n\", (*b)->content, parent ? parser_type_names[parent->type] : \"(nil)\");") unless sv[0] == '!'
        f.puts
        f.puts("    syntax_tree_node *node = new syntax_tree_node(syntax_tree_node::#{const sv}, #{sv[0] == '!' ? 'nullptr' : 'parent'}#{is_intermediate[sv] ? ', true' : ''});")

        f.puts
        f.puts("    // #{sv.sub('!', '')}:")

        svs[sv].each do |rule|
            f.puts("    //   #{rule * ' '}")
        end

        f.puts

        f.puts('    range_t m = b;')

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
                end

                if part.include?('(')
                    match = /^([\w-]+)\((.*)\)$/.match(part)
                    if !match
                        $stderr.puts("Cannot parse #{part}")
                        exit 1
                    end

                    kind = match[1]
                    condition = match[2]

                    if kind == 'identifier' || kind == 'keyword'
                        f.puts("    if ((m != e) && ((*m)->type == token::#{const 'identifier'}) && is_#{var kind}(node, *m, #{condition.empty? ? 'nullptr' : condition}))")
                    elsif condition.empty?
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
                    f.puts("    m = sv_#{var part}(node, m, e, &could_parse);")
                    f.puts('    if (!could_parse)') unless optional
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

            f.puts('    if (m > maximum_extent) maximum_extent = m;')
            f.puts('    *success = true;')
            f.puts("    m = #{post_modify[sv]}(node, m, e, success);") if post_modify[sv]
            f.puts("    #{post_hooks[sv]}(node);") if post_hooks[sv]
            f.puts("    return #{sv[0] == '!' ? 'node' : 'm'};")

            i += 1
        end

        f.puts
        f.puts
        f.puts("sv_#{var sv}_part_#{i}:")
        f.puts('    node->detach();')
        f.puts('    delete node;')
        f.puts('    *success = false;')
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
