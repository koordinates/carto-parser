#ifndef INTERMEDIATE_DUMPER_H_
#define INTERMEDIATE_DUMPER_H_

#include <ostream>

#include <intermediate/types.hpp>

#include <mapnik/map.hpp>

namespace carto { namespace intermediate {
    class dumper : public visitor {
    private:
        std::ostream &stream;
    public:
        dumper(std::ostream &);

        virtual void visit(stylesheet const&);
        virtual void visit(rule const&);
    };
} }

#endif

