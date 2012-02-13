#ifndef INTERMEDIATE_H_
#define INTERMEDIATE_H_

#include <cassert>
#include <map>
#include <vector>
#include <string>
#include <sstream>

#include <boost/variant.hpp>
#include <boost/optional.hpp>

#include <parse/filter_grammar.hpp>

#include <utility/utree.hpp>

namespace carto { namespace intermediate {

class stylesheet;
class rule;
class attribute;
class class_selector;
class id_selector;
class filter_selector;
class attachment_selector;

class visitor {
public:
    virtual void visit(stylesheet const&) = 0;
    virtual void visit(rule const&) = 0;
    virtual void visit(attribute const&) = 0;
};

class selector {
public:
    virtual ~selector() { }

    virtual const std::string get_selector_name() const = 0;
};

class class_selector : public selector {
public:
    class_selector(std::string n) : name(n) { }

    virtual ~class_selector() { }

    std::string name;

    inline const std::string get_selector_name() const {
        return "." + name;
    }
};

class id_selector : public selector {
public:
    id_selector(std::string n) : name(n) { }

    virtual ~id_selector() { }

    std::string name;

    inline const std::string get_selector_name() const {
        return "#" + name;
    }
};

typedef boost::variant<class_selector, id_selector> name_selector;

class filter_selector : public selector {
public:
    enum predicate {
        pred_eq,        // =
        pred_lt,        // <
        pred_le,        // <=
        pred_gt,        // >
        pred_ge,        // >=
        pred_neq        // !=
    };

    filter_selector(std::string k, predicate p, utree v) :
        key(k), pred(p), value(v) { }

    virtual ~filter_selector() { }

    std::string key;
    predicate pred;
    utree value;

    const std::string get_selector_name() const {
        std::stringstream oss;
        oss << "[";
        oss << key;

        switch(pred)
        {
            case pred_eq:
                oss << "=";
                break;
            case pred_lt:
                oss << "<";
                break;
            case pred_le:
                oss << "<=";
                break;
            case pred_gt:
                oss << ">";
                break;
            case pred_ge:
                oss << ">=";
                break;
            case pred_neq:
                oss << "!=";
                break;
        }

        oss << value;
        oss << "]";
        return oss.str();
    }
};

class attachment_selector : public selector {
public:
    attachment_selector(std::string n) : name(n) { }

    virtual ~attachment_selector() { }

    std::string name;

    inline const std::string get_selector_name() const {
        return "::" + name;
    }
};

class attribute {
public:
    attribute(std::string n, utree v) : name(n), value(v) { }

    std::string name;
    utree value;

    inline void accept(visitor &visitor) const {
        visitor.visit(*this);
    }
};

class rule {
public:
    boost::optional<name_selector> name_selector;
    std::vector<filter_selector> filters;
    boost::optional<attachment_selector> attachment_selector;

    std::vector<attribute> attrs;

    rule(boost::optional<carto::intermediate::name_selector> name_selector = boost::none,
         boost::optional<carto::intermediate::attachment_selector> attachment_selector = boost::none)
      : name_selector(name_selector),
        filters(),
        attachment_selector(attachment_selector),
        attrs() { }

    /*
     * A selector's specificity is calculated as follows:
     *
     * * count the number of ID selectors in the selector (= a)
     *
     * * count the number of class selectors, filters selectors, and
     *   pseudo-classes in the selector (= b)
     *
     * * count the number of type selectors and pseudo-elements in the
     *   selector (= c)
     *
     * * ignore the universal selector
     *
     * Concatenating the three numbers a-b-c (in a number system with a large
     * base) gives the specificity.
     *
     * See: http://www.w3.org/TR/css3-selectors/#specificity
     */
    inline unsigned int specificity() const {
        return (name_selector ? 0xff0000u : 0x000000u) |
               // clamp the max value at 0xff so we don't mess up the higher
               // bits
               ((filters.size() > 0xff ? 0xff : filters.size()) << 8) |
               (attachment_selector ? 0x0000ffu : 0x000000u);
    }

    inline void accept(visitor &visitor) const {
        visitor.visit(*this);
    }
};

class stylesheet {
public:
    typedef std::map<unsigned int, rule> rules_type;

    stylesheet() : rules() { }

    rules_type rules;

    inline void accept(visitor &visitor) const {
        visitor.visit(*this);
    }
};

} }

#endif
