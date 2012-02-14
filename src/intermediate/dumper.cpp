#include <intermediate/dumper.hpp>

#include <iostream>

namespace carto { namespace intermediate {

dumper::dumper(std::ostream &stream) : stream(stream) { }

void dumper::visit(stylesheet const& styl) {
    for(stylesheet::rules_type::const_iterator it = styl.rules.begin();
        it != styl.rules.end();
        ++it) {
        visit(*it);
    }
}

void dumper::visit(rule const& rule) {
    stream << rule.get_selector_name() << " {" << std::endl;

    for(rule::attributes_type::const_iterator it = rule.attrs.begin();
        it != rule.attrs.end();
        ++it) {
        stream << "    " << it->first << ": " << it->second << ";" << std::endl;
    }
    stream << "}" << std::endl << std::endl;
}

} }
