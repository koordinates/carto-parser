#ifndef EXPRESSION_EVAL_H
#define EXPRESSION_EVAL_H

#include <math.h>

#include <utility/utree.hpp>
#include <utility/environment.hpp>
#include <utility/carto_functions.hpp>

#include <parse/expression_grammar.hpp>

#include <position_iterator.hpp>

namespace carto {

struct expression {

    utree const& tree;
    annotations_type const& annotations;
    style_env const& env;
    
    expression(utree const& tree_, annotations_type const& annotations_, style_env const& env_);
    
    template<class T>
    T as(utree const& ut)
    {
        return detail::as<T>(ut);
    }
    
    int get_node_type(utree const& ut);
    
    source_location get_location(utree const& ut);
    
    inline bool is_color(utree const& ut)
    {
        return get_node_type(ut) == exp_color;
    }
    
    inline bool is_double(utree const& ut)
    {
        return ut.which() == spirit::utree_type::double_type;
    }
  
    utree eval();

    utree eval_var(utree const& node);
    
    utree eval_node(utree const& node);
    
    utree eval_function(utree const& node);
    
    utree fix_color_range(utree const& node);

#define EVAL_OP_PROTO(name, op) utree eval_##name(utree const& lhs, utree const& rhs)
    EVAL_OP_PROTO(add, +);
    EVAL_OP_PROTO(sub, -);
    EVAL_OP_PROTO(mult, *);
    EVAL_OP_PROTO(div, /);
#undef EVAL_OP_PROTO
};

}
#endif 
