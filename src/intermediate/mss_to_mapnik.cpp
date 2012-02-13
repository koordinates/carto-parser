#include <intermediate/mss_to_mapnik.hpp>

namespace carto { namespace intermediate {

mss_to_mapnik::mss_to_mapnik() : map() { }

void mss_to_mapnik::visit(stylesheet const& styl) {
    for(stylesheet::rules_type::const_iterator it = styl.rules.begin();
        it != styl.rules.end();
        ++it) {
        visit(it->second);
    }
}

void mss_to_mapnik::visit(rule const& rule) {
}

void mss_to_mapnik::visit(attribute const& attr) {
}

} }

