#ifndef INTERMEDIATE_GENERATOR_H_
#define INTERMEDIATE_GENERATOR_H_

#include <intermediate/types.hpp>

#include <mapnik/map.hpp>
#include <mapnik/rule.hpp>

#include <boost/optional.hpp>

namespace carto { namespace intermediate {
    class generation_error : std::runtime_error {
    public:
        generation_error(std::string const& msg) : std::runtime_error(msg) { }
        virtual ~generation_error() throw() { }
    };

    class mss_to_mapnik : public visitor {
    private:
        mapnik::Map &map_;
        boost::optional<mapnik::rule> rule_;

        template<class symbolizer>
        inline symbolizer init_symbolizer() 
        {
            return symbolizer();
        }

        template<class symbolizer>
        symbolizer *find_symbolizer()
        {
            for(mapnik::rule::symbolizers::iterator it = rule_->begin();
                it != rule_->end();
                ++it) {
                if (symbolizer *sym = boost::get<symbolizer>(&(*it)))
                    return(sym);
            }

            BOOST_ASSERT(it==end);
            rule_->append(init_symbolizer<symbolizer>());

            symbolizer *sym = boost::get<symbolizer>(& (*(--rule_->end())) );
            return sym;
        }

        mapnik::transform_type create_transform(std::string const& str);

        void emit_polygon(std::string const&, utree const&);
        void emit_line(std::string const&, utree const&);
        void emit_marker(std::string const&, utree const&);
        void emit_point(std::string const&, utree const&);
        void emit_line_pattern(std::string const&, utree const&);
        void emit_polygon_pattern(std::string const&, utree const&);
        void emit_raster(std::string const&, utree const&);
        void emit_building(std::string const&, utree const&);
        void emit_text(std::string const&, utree const&);
        void emit_shield(std::string const&, utree const&);

    public:
        explicit mss_to_mapnik(mapnik::Map &m);

        virtual void visit(stylesheet const&);
        virtual void visit(rule const&);
    };
} }

#endif

