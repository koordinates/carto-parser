#ifndef INTERMEDIATE_GENERATOR_H_
#define INTERMEDIATE_GENERATOR_H_

#include <intermediate/types.hpp>

#include <mapnik/map.hpp>

namespace carto { namespace intermediate {
    class mss_to_mapnik : public visitor {
        mapnik::Map map;

    public:
        mss_to_mapnik();

        virtual void visit(stylesheet const&);
        virtual void visit(rule const&);
        virtual void visit(attribute const&);
    };
} }

#endif

