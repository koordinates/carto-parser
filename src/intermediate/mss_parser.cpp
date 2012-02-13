#include <intermediate/mss_parser.hpp>

#include <expression_eval.hpp>
#include <parse/carto_grammar.hpp>

#include <fstream>

namespace carto { namespace intermediate {

mss_parser::mss_parser(parse_tree const& pt, bool strict_, std::string const& path_)
  : tree(pt),
    strict(strict_),
    path(path_) { }
  
mss_parser::mss_parser(std::string const& in, bool strict_, std::string const& path_)
  : strict(strict_),
    path(path_) 
{
    typedef position_iterator<std::string::const_iterator> iter;
    tree = build_parse_tree<carto_parser<iter> >(in, path);
}

inline int mss_parser::get_node_type(utree const& ut) {
    return tree.annotations(ut.tag()).second;
}

inline source_location mss_parser::get_location(utree const& ut) {
    return tree.annotations()[ut.tag()].first;
}

void mss_parser::parse_stylesheet(stylesheet &styl,
                                               style_env &env) {
    using spirit::utree_type;

    utree const& root_node = tree.ast();

    for (utree::const_iterator it = root_node.begin();
         it != root_node.end();
         ++it) {
        switch((carto_node_type) get_node_type(*it)) {
            case carto_variable:
                parse_variable(*it,env);
                break;
            case carto_map_style:
                parse_map_style(styl, *it, env);
                break;
            case carto_style:
                parse_style(styl, *it, env);
                break;
            case carto_mixin:
            case carto_comment:
                break;
            default:
            {
                std::stringstream out;
                out << "Invalid stylesheet node type: " << get_node_type(*it)
                    << " at " << get_location(*it).get_string();
                throw parser_error(out.str());
            }
        }
     }
}

void mss_parser::parse_style(
    stylesheet &styl, utree const& node, style_env const& parent_env,
    rule const& parent_rule
) {
    BOOST_ASSERT(node.size()==2);

    for (utree::const_iterator style_it  = node.front().begin();
         style_it != node.front().end();
         ++style_it) {

        style_env env(parent_env);
        rule rule(parent_rule);

        BOOST_ASSERT(*style_it.size() == 3);
        utree::const_iterator name_it  = (*style_it).begin(),
                              name_end = (*style_it).end();

        utree const& uname   = *name_it; name_it++;
        utree const& uattach = *name_it; name_it++;
        utree const& ufilter = *name_it;

        // parse the name
        if (uname.size() != 0) {
            std::string name = as<std::string>(uname);
            switch (name[0])
            {
                case '#':
                    rule.name_selector = id_selector(as<std::string>(uattach));
                    break;

                case '.':
                    rule.name_selector = class_selector(as<std::string>(uattach));
                    break;

                default:
                    std::stringstream out;
                    out << "Unknown name: " << name
                        << " at " << get_location(uname).get_string();
                    throw parser_error(out.str());
            }
        }

        // parse attachments
        if (uattach.size() != 0) {
            rule.attachment_selector = attachment_selector(as<std::string>(uattach));
        }

        // parse filters
        if (ufilter.size() != 0) {
            BOOST_ASSERT(get_node_type(ufilter) == carto_filter);
            parse_filter(styl, ufilter, env, rule);
        }

        // parse the rest
        for (utree::const_iterator it = node.back().begin();
             it != node.back().end(); ++it) {
            switch(get_node_type(*it)) {
                case carto_variable:
                    parse_variable(*it,env);
                    break;
                case carto_style:
                {
                    parse_style(styl, *it, env, rule);
                    break;
                }
                case carto_attribute:
                    parse_attribute(styl, *it, env, rule);                    
                    break;
                case carto_mixin:
                case carto_comment:
                    break;
                default:
                    std::stringstream out;
                    out << "Invalid style node type: " << get_node_type(*it)
                        << " at " << get_location(*it).get_string();
                    throw parser_error(out.str());
            }
        }

        styl.rules[rule.specificity()] = rule;
    }
}

void mss_parser::parse_filter(
    stylesheet &style, utree const& node, style_env const& env, rule &rule
) {
    if (node.size() == 0) return;

    utree::const_iterator it  = node.begin(),
                          end = node.end();
                          
    for (; it != end; ++it)
    {
        utree::const_iterator fit = it->begin(),
                              fend = it->end();

        utree key_utree = *fit; ++fit;
        utree value = *fit; ++fit;

        std::string key = as<std::string>(key_utree);

        filter_selector::predicate pred;

        switch(get_node_type(node))
        {
            case filter_eq:
                pred = filter_selector::pred_eq;
                break;
            case filter_lt:
                pred = filter_selector::pred_lt;
                break;
            case filter_le:
                pred = filter_selector::pred_le;
                break;
            case filter_gt:
                pred = filter_selector::pred_gt;
                break;
            case filter_ge:
                pred = filter_selector::pred_ge;
                break;
            case filter_neq:
                pred = filter_selector::pred_neq;
                break;
            default:
                std::stringstream out;
                out << "Unknown predicate at "
                    << get_location(node).get_string();
                throw parser_error(out.str());
        }

        filter_selector filt(key, pred, value);
        rule.filters.push_back(filt);
    }
}

utree mss_parser::eval_var(utree const& node,
                                        style_env const& env) {
    std::string key = as<std::string>(node.front());
    
    utree value = env.vars.lookup(key);
    
    if (value == utree::nil_type()) {
        std::stringstream err;
        err << "Unknown variable: @" << key
            << " at " << get_location(node).get_string(); 
        throw config_error(err.str());
    }
    
    return (get_node_type(value) == carto_variable) ? eval_var(value,env) : value;
}

utree mss_parser::parse_value(utree const& node,
                                           style_env const& env) {
    if (get_node_type(node) == carto_variable) {
        return eval_var(node, env); // vars can point at other vars
    } else if (get_node_type(node) == carto_expression) {
        //BOOST_ASSERT(node.size()==1);
        expression exp(node.front().front(), tree.annotations(), env);
        return exp.eval();
    } else {
        if (node.size() == 1)
            return node.front();
        else 
            return node;
    }
}

void mss_parser::parse_attribute(stylesheet &map,
                                              utree const& node,
                                              style_env const& env,
                                              rule &rule) {
    BOOST_ASSERT(node.size()==2);
    
    std::string key = as<std::string>(node.front());
    utree value = parse_value(node.back(), env);

    attribute attr(key, value);
    rule.attrs.push_back(attr);
}

void mss_parser::parse_variable(utree const& node,
                                             style_env& env) {
    std::string name = as<std::string>(node.front());
    utree val = parse_value(node.back(), env);
    env.vars.define(name, val);
}

void mss_parser::parse_map_style(stylesheet &styl,
                                              utree const& node,
                                              style_env& env) {
    throw parser_error("map style currently unsupported");
}

mss_parser mss_parser::load(std::string filename, bool strict) {
    std::ifstream file(filename.c_str(), std::ios_base::in);

    if (!file)
        throw config_error(std::string("Cannot open input file: ")+filename);

    std::string in;
    file.unsetf(std::ios::skipws);
    copy(std::istream_iterator<char>(file),
         std::istream_iterator<char>(),
         std::back_inserter(in));

    return mss_parser(in, strict, filename);
}

} }
