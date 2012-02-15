/*==============================================================================
    Copyright (c) 2010 Colin Rundel

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file BOOST_LICENSE_1_0.rst or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#include <mss_parser.hpp>

#include <iosfwd>
#include <fstream>
#include <sstream>

#include <boost/functional/hash.hpp>

#include <parse/carto_grammar.hpp>
#include <parse/parse_tree.hpp>
#include <parse/json_grammar.hpp>

#include <mapnik/map.hpp>
#include <mapnik/config_error.hpp>

#include <mapnik/color.hpp>
#include <mapnik/color_factory.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/font_set.hpp>
#include <mapnik/expression_string.hpp>
#include <mapnik/filter_factory.hpp>
#include <mapnik/version.hpp>
#include <mapnik/rule.hpp>

#include <mapnik/svg/svg_parser.hpp>
#include <mapnik/svg/svg_path_parser.hpp>

#include <agg_trans_affine.h>

#include <expression_eval.hpp>
#include <generate/generate_filter.hpp>
#include <utility/utree.hpp>
#include <utility/environment.hpp>
#include <utility/version.hpp>
#include <utility/round.hpp>


namespace carto {

using mapnik::config_error;

mss_parser::mss_parser(parse_tree const& pt, bool strict_, std::string const& path_)
  : intermediate_parser(carto::intermediate::mss_parser(pt, strict_, path_)) { }
  
mss_parser::mss_parser(std::string const& in, bool strict_, std::string const& path_)
  : intermediate_parser(carto::intermediate::mss_parser(in, strict_, path_)) { }

void mss_parser::parse_stylesheet(mapnik::Map& map, style_env& env)
{
    carto::intermediate::stylesheet styl;
    intermediate_parser.parse_stylesheet(styl, env);

    carto::intermediate::mss_to_mapnik(map).visit(styl);
}

mss_parser load_mss(std::string filename, bool strict)
{
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

}

