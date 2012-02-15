#ifndef INTERMDIATE_MSS_PARSER_H
#define INTERMDIATE_MSS_PARSER_H

#include <intermediate/types.hpp>

#include <string>
#include <exception>

#include <parse/parse_tree.hpp>

#include <utility/environment.hpp>

namespace carto { namespace intermediate {

class parser_error : public std::runtime_error {
public:
    parser_error(std::string const& msg) : std::runtime_error(msg) { }
    virtual ~parser_error() throw() { }
};

struct mss_parser {
    parse_tree tree;
    bool strict;
    std::string path;
    
    mss_parser(parse_tree const& pt, bool strict_ = false,
               std::string const& path_ = "./");
      
    mss_parser(std::string const& in, bool strict_ = false,
               std::string const& path_ = "./");
    
    template<class T>
    T as(utree const& ut)
    {
        return detail::as<T>(ut);
    }
    
    inline parse_tree get_parse_tree() {
        return tree;
    }
    
    inline std::string get_path() {
        return path;
    }
    
    int get_node_type(utree const& ut);

    source_location get_location(utree const& ut);

    void parse_stylesheet(stylesheet &styl, style_env& env);

    void cascade(stylesheet &styl);

    void parse_style(stylesheet &styl, utree const& node,
                     style_env const& parent_env,
                     rule const& parent_rule = rule());

    void parse_filter(stylesheet &style, utree const& node,
                      style_env const& env, rule& rule);

    utree eval_var(utree const& node, style_env const& env);

    utree parse_value(utree const& node, style_env const& env);

    void parse_attribute(stylesheet &map, utree const& node,
                         style_env const& env, rule &rule);


    void parse_variable(utree const& node, style_env& env);

    void parse_map_style(stylesheet &styl, utree const& node, style_env& env);
  
    static mss_parser load(std::string filename, bool strict);  
};

} }
#endif
