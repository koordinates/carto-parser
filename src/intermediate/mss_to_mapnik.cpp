#include <intermediate/mss_to_mapnik.hpp>

#include <utility/utree.hpp>
#include <utility/version.hpp>

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

namespace carto { namespace intermediate {

static 
double const zoom_ranges[] = { 1000000000, 500000000, 200000000, 100000000,
                                 50000000,  25000000,  12500000,   6500000,
                                  3000000,   1500000,    750000,    400000,
                                   200000,    100000,     50000,     25000,
                                    12500,      5000,      2500,      1500,
                                      750,       500,       250,       100};

using carto::detail::as;

mss_to_mapnik::mss_to_mapnik(mapnik::Map &m) : map_(m) { }

mapnik::transform_type mss_to_mapnik::create_transform(std::string const& str)
{
    agg::trans_affine tr;
    if (!mapnik::svg::parse_transform(str.c_str(),tr))
    {
        std::stringstream err;
        err << "Could not parse transform from '" << str 
            << "', expected string like: 'matrix(1, 0, 0, 1, 0, 0)'";
        std::clog << "### WARNING: " << err << std::endl;         
    }
    mapnik::transform_type matrix;
    tr.store_to(&matrix[0]);
    
    return matrix;
}

void mss_to_mapnik::emit_polygon(std::string const& key, utree const& value) {
    mapnik::polygon_symbolizer *s = find_symbolizer<mapnik::polygon_symbolizer>();

    if (key == "polygon-fill") {
        s->set_fill(as<mapnik::color>(value));
    } else if (key == "polygon-gamma") {
        s->set_gamma(as<double>(value));
    } else if (key == "polygon-opacity") {
        s->set_opacity(as<double>(value));
    } else {
        throw generation_error("Unknown key: " + key);
    }
}

void mss_to_mapnik::emit_line(std::string const& key, utree const& value) {
    mapnik::line_symbolizer *s = find_symbolizer<mapnik::line_symbolizer>();
    mapnik::stroke strk = s->get_stroke();

    if (key == "line-dasharray") {
        
        BOOST_ASSERT( (value.size()-1) % 2 == 0 );
        
        typedef utree::const_iterator iter;
        iter it = value.begin(),
            end = value.end();
        
        for(; it!=end;) {
            double dash = as<double>(*it); it++;
            double gap  = as<double>(*it); it++;
            
            strk.add_dash(dash,gap);
        }
    } else if (key == "line-color") {
        strk.set_color(as<mapnik::color>(value));
    } else if (key == "line-width") {
        strk.set_width(as<double>(value));
    } else if (key == "line-opacity") {
        strk.set_opacity(as<double>(value));
    } else if (key == "line-join") {
        mapnik::line_join_e en;
        en.from_string(as<std::string>(value));
        strk.set_line_join(en);
    } else if (key == "line-cap") {
        mapnik::line_cap_e en;
        en.from_string(as<std::string>(value));
        strk.set_line_cap(en);
    } else if (key == "line-gamma") {
        strk.set_gamma(as<double>(value));
    } else if (key == "line-dash-offset") {
        strk.set_dash_offset(as<double>(value));
    } else {
        throw generation_error("Unknown key: " + key);
    }
    s->set_stroke(strk);
}

void mss_to_mapnik::emit_marker(std::string const& key, utree const& value) {
    mapnik::markers_symbolizer *s = find_symbolizer<mapnik::markers_symbolizer>(); 
    mapnik::stroke stroke = s->get_stroke();

    if (key == "marker-file") {
        s->set_filename(mapnik::parse_path(as<std::string>(value)));
    } else if (key == "marker-opacity") {
        s->set_opacity(as<float>(value));
    } else if (key == "marker-line-color") {
        stroke.set_color(as<mapnik::color>(value));
    } else if (key == "marker-line-width") {
        stroke.set_width(as<double>(value));
    } else if (key == "marker-line-opacity") {
        stroke.set_opacity(as<double>(value));
    } else if (key == "marker-placement") {
        mapnik::marker_placement_e en;
        en.from_string(as<std::string>(value));
        s->set_marker_placement(en);
    } else if (key == "marker-type") {
        mapnik::marker_type_e en;
        en.from_string(as<std::string>(value));
        s->set_marker_type(en);
    } else if (key == "marker-width") {
        s->set_width(as<double>(value));
    } else if (key == "marker-height") {
        s->set_height(as<double>(value));
    } else if (key == "marker-fill") {
        s->set_fill(as<mapnik::color>(value));
    } else if (key == "marker-allow-overlap") {
        s->set_allow_overlap(as<bool>(value));
    } else if (key == "marker-spacing") {
        s->set_spacing(as<double>(value));
    } else if (key == "marker-max-error") {
        s->set_max_error(as<double>(value));
    } else if (key == "marker-transform") {
        s->set_transform(create_transform(as<std::string>(value)));
    } else {
        throw generation_error("Unknown key: " + key);
    }
}

void mss_to_mapnik::emit_point(std::string const& key, utree const& value) {
    mapnik::point_symbolizer *s = find_symbolizer<mapnik::point_symbolizer>();

    if (key == "point-file") {
        s->set_filename(mapnik::parse_path(as<std::string>(value)));
    } else if (key == "point-allow-overlap") {
        s->set_allow_overlap(as<bool>(value));
    } else if (key == "point-ignore-placement") {
        s->set_ignore_placement(as<bool>(value));
    } else if (key == "point-opacity") {
        s->set_opacity(as<float>(value));
    } else if (key == "point-placement") {
        mapnik::point_placement_e en;
        en.from_string(as<std::string>(value));
        s->set_point_placement(en);
    } else if (key == "point-transform") {
        s->set_transform(create_transform(as<std::string>(value)));
    } else {
        throw generation_error("Unknown key: " + key);
    }
}

void mss_to_mapnik::emit_line_pattern(std::string const& key, utree const& value) {
    mapnik::line_pattern_symbolizer *s = find_symbolizer<mapnik::line_pattern_symbolizer>();
    
    if (key == "line-pattern-file") {
        s->set_filename(mapnik::parse_path(as<std::string>(value)));
    } else {
        throw generation_error("Unknown key: " + key);
    }
}

void mss_to_mapnik::emit_polygon_pattern(std::string const& key, utree const& value) {
    mapnik::polygon_pattern_symbolizer *s = find_symbolizer<mapnik::polygon_pattern_symbolizer>();

    if (key == "polygon-pattern-file") {
        s->set_filename(mapnik::parse_path(as<std::string>(value)));
    } else if (key == "polygon-pattern-alignment") {
        mapnik::pattern_alignment_e en;
        en.from_string(as<std::string>(value));
        s->set_alignment(en);
    } else {
        throw generation_error("Unknown key: " + key);
    }
}

void mss_to_mapnik::emit_raster(std::string const& key, utree const& value) {
    mapnik::raster_symbolizer *s = find_symbolizer<mapnik::raster_symbolizer>();
    
    if (key == "raster-opacity") {
        s->set_opacity(as<float>(value));
    } else if (key == "raster-mode") {
        s->set_mode(as<std::string>(value));
    } else if (key == "raster-scaling") {
        s->set_scaling(as<std::string>(value));
    } else {
        throw generation_error("Unknown key: " + key);
    }
}

void mss_to_mapnik::emit_building(std::string const& key, utree const& value) {
    mapnik::building_symbolizer *s = find_symbolizer<mapnik::building_symbolizer>();
    
    if (key == "building-fill") {
        s->set_fill(as<mapnik::color>(value));
    } else if (key == "building-fill-opacity") {
        s->set_opacity(as<double>(value));
    } else if (key == "building-height") {
        s->set_height(mapnik::parse_expression(as<std::string>(value)));
    } else {
        throw generation_error("Unknown key: " + key);
    }
}

void mss_to_mapnik::emit_text(std::string const& key, utree const& value) {
    mapnik::text_symbolizer *s = find_symbolizer<mapnik::text_symbolizer>();
    
    using boost::spirit::utree_type;
    
    if (key == "text-face-name") {
        if (value.which() != utree_type::list_type) {
            s->set_face_name(as<std::string>(value));
        } else {
            typedef utree::const_iterator iter;
            iter it, end;
            
            it  = value.begin();
            end = value.end();
            
            std::size_t seed = 0;
            for( ; it!=end; ++it)
                boost::hash_combine(seed, as<std::string>(*it));
            
            std::stringstream ss;
            ss << std::hex << seed;
            
            std::string name = ss.str();
            
            // FIXME - font_set does not have a/ set_name method so have to do this with two loops
            it  = value.begin();
            end = value.end();
            
            mapnik::font_set fs(name);
            for( ; it!=end; ++it)
                fs.add_face_name(as<std::string>(*it));
            
            s->set_fontset(fs);
            s->set_face_name(std::string());
            map_.insert_fontset(name, fs);
        }
    } else if (key == "text-name") {
        s->set_name(mapnik::parse_expression(as<std::string>(value)));
    } else if (key == "text-size") {
        s->set_text_size(round(as<double>(value)));
    } else if (key == "text-ratio") {
        s->set_text_ratio(round(as<double>(value)));
    } else if (key == "text-wrap-width") {
        s->set_wrap_width(round(as<double>(value)));
    } else if (key == "text-spacing") {
        s->set_label_spacing(round(as<double>(value)));
    } else if (key == "text-character-spacing") {
        s->set_character_spacing(round(as<double>(value)));
    } else if (key == "text-line-spacing") {
        s->set_line_spacing(round(as<double>(value)));
    } else if (key == "text-label-position-tolerance") {
        s->set_label_position_tolerance(round(as<double>(value)));
    } else if (key == "text-max-char-angle-delta") {
        s->set_max_char_angle_delta(as<double>(value));
    } else if (key == "text-fill") {
        s->set_fill(as<mapnik::color>(value));
    } else if (key == "text-opacity") {
        s->set_text_opacity(as<double>(value));
    } else if (key == "text-halo-fill") {
        s->set_halo_fill(as<mapnik::color>(value));
    } else if (key == "text-halo-radius") {
        s->set_halo_radius(as<double>(value));
    } else if (key == "text-dx") {
        double x = as<double>(value);
        double y = s->get_displacement().get<1>();
        s->set_displacement(x,y);
    } else if (key == "text-dy") {
        double x = s->get_displacement().get<0>();
        double y = as<double>(value);
        s->set_displacement(x,y);
    } else if (key == "text-vertical-alignment") {
        mapnik::vertical_alignment_e en;
        en.from_string(as<std::string>(value));
        s->set_vertical_alignment(en);
    } else if (key == "text-avoid-edges") {
        s->set_avoid_edges(as<bool>(value));
    } else if (key == "text-min-distance") {
        s->set_minimum_distance(as<double>(value));
    } else if (key == "text-min-padding") {
        s->set_minimum_padding(as<double>(value));
    } else if (key == "text-allow-overlap") {
        s->set_allow_overlap(as<bool>(value));
    } else if (key == "text-placement") {
        mapnik::label_placement_e en;
        en.from_string(as<std::string>(value));
        s->set_label_placement(en);
    } else if (key == "text-placement-type") {
        // FIXME
    } else if (key == "text-placements") {
        // FIXME
    } else if (key == "text-transform") {
        mapnik::text_transform_e en;
        en.from_string(as<std::string>(value));
        s->set_text_transform(en);
    } else {
        throw generation_error("Unknown key: " + key);
    }
}

void mss_to_mapnik::emit_shield(std::string const& key, utree const& value) {
    mapnik::shield_symbolizer *s = find_symbolizer<mapnik::shield_symbolizer>();
    
    if (key == "shield-name") {
        s->set_name(mapnik::parse_expression(as<std::string>(value)));
    } else if (key == "shield-face-name") {
        s->set_face_name(as<std::string>(value));
    } else if (key == "shield-size") {
        s->set_text_size(round(as<double>(value)));
    } else if (key == "shield-spacing") {
        s->set_label_spacing(round(as<double>(value)));
    } else if (key == "shield-character-spacing") {
        s->set_character_spacing(round(as<double>(value)));
    } else if (key == "shield-line-spacing") {
        s->set_line_spacing(round(as<double>(value)));
    } else if (key == "shield-fill") {
        s->set_fill(as<mapnik::color>(value));
    } else if (key == "shield-text-dx") {
        double x = as<double>(value);
        double y = s->get_displacement().get<1>();
        s->set_displacement(x,y);
    } else if (key == "shield-text-dy") {
        double x = s->get_displacement().get<0>();
        double y = as<double>(value);
        s->set_displacement(x,y);
    } else if (key == "shield-dx") {
        double x = as<double>(value);
        double y = s->get_shield_displacement().get<1>();
        s->set_shield_displacement(x,y);
    } else if (key == "shield-dy") {
        double x = s->get_shield_displacement().get<0>();
        double y = as<double>(value);
        s->set_shield_displacement(x,y);
    } else if (key == "shield-min-distance") {
        s->set_minimum_distance(as<double>(value));
    } else if (key == "shield-placement") {
        mapnik::label_placement_e en;
        en.from_string(as<std::string>(value));
        s->set_label_placement(en);
    } else {
        throw generation_error("Unknown key: " + key);
    }
}

std::string stringify_filter_value(utree const& value) {
    std::stringstream oss;
    if (value.which() == boost::spirit::utree_type::list_type) {
        utree::const_iterator it = value.begin(), end = value.end();
        for (; it != end; ++it) 
            oss << stringify_filter_value(*it);
    } else {
        oss << as<std::string>(value);
    }
    return oss.str();
}

void mss_to_mapnik::emit_map_style(stylesheet::map_style_type const& map_style) {
    mapnik::parameters extra_attr;
    bool relative_to_xml = true;

    for(stylesheet::map_style_type::const_iterator it = map_style.begin();
        it != map_style.end();
        ++it) {
        std::string const& key = it->first;
        utree const& value = it->second;
        std::string base = "";

        if (key == "srs") {
            map_.set_srs(as<std::string>(value));
        } else if (key == "background-color") {
            BOOST_ASSERT((carto_node_type) get_node_type(value) == carto_color);
            map_.set_background(as<mapnik::color>(value));
        } else if (key == "background-image") {
            map_.set_background_image(base+as<std::string>(value));
        } else if (key == "buffer-size") {
            map_.set_buffer_size(round(as<double>(value)));
        } else if (key == "base") {
            base = as<std::string>(value); // FIXME - not sure this is correct
        } else if (key == "paths-from-xml") {
            relative_to_xml = as<bool>(value);
        } else if (key == "minimum-version") {
            std::string ver_str = as<std::string>(value);
            extra_attr["minimum-version"] = ver_str;
            
            int min_ver = version_from_string(ver_str);
            
            if (min_ver == -1) {
                throw mapnik::config_error(std::string("Invalid version string ") + ver_str);
            } else if (min_ver > MAPNIK_VERSION) {
                throw mapnik::config_error(std::string("This map uses features only present in Mapnik version ") + ver_str + " and newer");
            }
        } 
        else if (key == "font-directory") {
            std::string dir = base+as<std::string>(value);
            extra_attr["font-directory"] = dir;
            //freetype_engine::register_fonts( ensure_relative_to_xml(dir), false);
        } else {
            throw generation_error("Unknown key: " + key);
        }

    }

    map_.set_extra_attributes(extra_attr);
}

void mss_to_mapnik::emit_filters(rule::filters_type const& filters) {
    if(!filters.size()) return;

    std::vector<std::string> emit_filters;

    for(rule::filters_type::const_iterator it = filters.begin();
        it != filters.end();
        ++it) {
        std::stringstream foss;

        switch(it->pred) {
            case filter_selector::pred_eq:
                if(it->key == "zoom") {
                    int b = as<int>(it->value);
                    rule_->set_min_scale(zoom_ranges[b + 1]);
                    rule_->set_max_scale(zoom_ranges[b]);
                    continue;
                } else {
                    foss << "[" << it->key << "]=" << stringify_filter_value(it->value);
                }
                break;

            case filter_selector::pred_lt:
                if(it->key == "zoom") {
                    int b = as<int>(it->value);
                    rule_->set_min_scale(zoom_ranges[b - 1]);
                    continue;
                } else {
                    foss << "[" << it->key << "]<" << stringify_filter_value(it->value);
                }
                break;

            case filter_selector::pred_le:
                if(it->key == "zoom") {
                    int b = as<int>(it->value);
                    rule_->set_min_scale(zoom_ranges[b]);
                    continue;
                } else {
                    foss << "[" << it->key << "]<=" << stringify_filter_value(it->value);
                }
                break;

            case filter_selector::pred_gt:
                if(it->key == "zoom") {
                    int b = as<int>(it->value);
                    rule_->set_max_scale(zoom_ranges[b + 1]);
                    continue;
                } else {
                    foss << "[" << it->key << "]>" << stringify_filter_value(it->value);
                }
                break;

            case filter_selector::pred_ge:
                if(it->key == "zoom") {
                    int b = as<int>(it->value);
                    rule_->set_max_scale(zoom_ranges[b]);
                    continue;
                } else {
                    foss << "[" << it->key << "]>=" << stringify_filter_value(it->value);
                }
                break;

            case filter_selector::pred_neq:
                if(it->key == "zoom") {
                    throw generation_error("!= unsupported for zoom");
                } else {
                    foss <<"[" << it->key << "]!=" << stringify_filter_value(it->value);
                }
                break;

            case filter_selector::pred_unknown:
                throw generation_error("bad predicate");
                
        }

        if(!foss.str().empty())
            emit_filters.push_back(foss.str());
    }

    if(!emit_filters.size()) return;

    std::stringstream oss;
    oss << "(" << boost::algorithm::join(emit_filters, ") and (") << ")";

    mapnik::expression_ptr expr = mapnik::parse_expression(oss.str(), "utf8");
    rule_->set_filter(expr);
}

void mss_to_mapnik::visit(stylesheet const& styl) {
    emit_map_style(styl.map_style);

    for(stylesheet::rules_type::const_iterator it = styl.rules.begin();
        it != styl.rules.end();
        ++it) {
        visit(*it);
    }
}

void mss_to_mapnik::visit(rule const& rule) {
    std::string name = rule.get_partial_name();

    mapnik::Map::style_iterator style_it, style_end;

    style_it  = map_.styles().find(name);                           
    style_end = map_.styles().end();

    if (style_it == style_end) {
        mapnik::feature_type_style new_style;
        new_style.set_filter_mode(mapnik::FILTER_FIRST);

        map_.insert_style(name, new_style);
        style_it = map_.styles().find(name);
    }

    rule_ = mapnik::rule();
    emit_filters(rule.filters);

    if(rule.attrs.size())
    {
        for(rule::attributes_type::const_iterator it = rule.attrs.begin();
            it != rule.attrs.end();
            ++it) {
            std::string key = it->first;
            if (key.substr(0,8) == "polygon-")
                emit_polygon(it->first, it->second);
            else if (key.substr(0,5) == "line-")  
                emit_line(it->first, it->second);
            else if (key.substr(0,7) == "marker-")
                emit_marker(it->first, it->second);
            else if (key.substr(0,6) == "point-")
                emit_point(it->first, it->second);
            else if (key.substr(0,13)== "line-pattern-")
                emit_line_pattern(it->first, it->second);
            else if (key.substr(0,16)== "polygon-pattern-") 
                emit_polygon_pattern(it->first, it->second);
            else if (key.substr(0,7) == "raster-")
                emit_raster(it->first, it->second);
            else if (key.substr(0,9) == "building-")
                emit_building(it->first, it->second);
            else if (key.substr(0,5) == "text-")
                emit_text(it->first, it->second);
            else if (key.substr(0,7) == "shield-")
                emit_shield(it->first, it->second);
            else 
                throw generation_error("Unknown key: " + key);
        }

        (*style_it).second.add_rule(*rule_);
    }
}

template<>
inline mapnik::text_symbolizer mss_to_mapnik::init_symbolizer<mapnik::text_symbolizer>() 
{
    return mapnik::text_symbolizer(boost::make_shared<mapnik::expr_node>(true), 
                                   "<no default>", 0, 
                                   mapnik::color(0,0,0) );
    
    //return mapnik::text_symbolizer(mapnik::expression_ptr(), "<no default>", 0, 
    //                               mapnik::color(0,0,0) );
}

template<>
inline mapnik::shield_symbolizer mss_to_mapnik::init_symbolizer<mapnik::shield_symbolizer>() 
{
    return mapnik::shield_symbolizer(mapnik::expression_ptr(), "<no default>", 0, 
                                     mapnik::color(0,0,0), mapnik::path_expression_ptr());
}

template<>
inline mapnik::polygon_pattern_symbolizer mss_to_mapnik::init_symbolizer<mapnik::polygon_pattern_symbolizer>() 
{
    return mapnik::polygon_pattern_symbolizer(mapnik::parse_path(""));
}

template<>
inline mapnik::line_pattern_symbolizer mss_to_mapnik::init_symbolizer<mapnik::line_pattern_symbolizer>() 
{
    return mapnik::line_pattern_symbolizer(mapnik::parse_path(""));
}

} }
