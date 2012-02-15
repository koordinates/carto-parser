#ifndef INTERMEDIATE_H_
#define INTERMEDIATE_H_

#include <cassert>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <sstream>
#include <utility>

#include <boost/variant.hpp>
#include <boost/optional.hpp>

#include <parse/filter_grammar.hpp>

#include <utility/utree.hpp>

namespace carto { namespace intermediate {

class stylesheet;
class rule;
class class_selector;
class id_selector;
class filter_selector;
class attachment_selector;

class visitor {
public:
    virtual void visit(stylesheet const&) = 0;
    virtual void visit(rule const&) = 0;
};

class selector {
public:
    virtual ~selector() { }

    virtual const std::string get_selector_name() const = 0;
};

class class_selector : public selector {
public:
    explicit class_selector(std::string n) : name(n) { }

    virtual ~class_selector() { }

    std::string name;

    inline const std::string get_selector_name() const {
        return "." + name;
    }

    inline bool operator==(id_selector const& rhs) const {
        return false;
    }

    inline bool operator==(class_selector const& rhs) const {
        return name == rhs.name;
    }
};

class id_selector : public selector {
public:
    explicit id_selector(std::string n) : name(n) { }

    virtual ~id_selector() { }

    std::string name;

    inline const std::string get_selector_name() const {
        return "#" + name;
    }

    inline bool operator==(id_selector const& rhs) const {
        return name == rhs.name;
    }

    inline bool operator==(class_selector const& rhs) const {
        return false;
    }
};

typedef boost::variant<class_selector, id_selector> name_selector;

class filter_selector : public selector {
public:
    enum predicate {
        pred_unknown,   // ?
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
            case pred_unknown:
                oss << "?";
                break;
        }

        oss << value;
        oss << "]";
        return oss.str();
    }

    inline bool operator==(filter_selector const& rhs) const {
        return key == rhs.key && pred == rhs.pred && value == rhs.value;
    }

    struct comparator {
        bool operator()(filter_selector const &lhs, filter_selector const& rhs) {
            return lhs.key.compare(rhs.key) ? lhs.pred < rhs.pred : true;
        }
    };
};

class attachment_selector : public selector {
public:
    explicit attachment_selector(std::string n) : name(n) { }

    virtual ~attachment_selector() { }

    std::string name;

    inline const std::string get_selector_name() const {
        return "::" + name;
    }

    inline bool operator==(attachment_selector const& rhs) const {
        return name == rhs.name;
    }
};

class rule {
private:
    class selector_visitor : public boost::static_visitor<> {
        std::stringstream &oss;

    public:
        selector_visitor(std::stringstream &oss) : oss(oss) { }

        inline void operator()(id_selector const& id) const {
            oss << id.get_selector_name();
        }

        inline void operator()(class_selector const& cls) const {
            oss << cls.get_selector_name();
        }
    };

public:
    typedef std::vector<name_selector> names_type;
    names_type names;

    typedef std::multiset<filter_selector, filter_selector::comparator> filters_type;
    filters_type filters;

    boost::optional<attachment_selector> attachment_selector;

    typedef std::map<std::string, utree> attributes_type;
    attributes_type attrs;

    rule(boost::optional<carto::intermediate::attachment_selector> attachment_selector = boost::none)
      : names(),
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
        return (names.size() << 16) |
               ((filters.size() > 0xff ? 0xff : filters.size()) << 8) |
               (attachment_selector ? 0x0000ffu : 0x000000u);
    }

    inline void accept(visitor &visitor) const {
        visitor.visit(*this);
    }

    const std::string get_partial_name() const {
        std::stringstream oss;

        for(names_type::const_iterator it = names.begin();
            it != names.end();
            ++it) {
            boost::apply_visitor(selector_visitor(oss), *it);
        }

        if(attachment_selector) oss << attachment_selector->get_selector_name();

        return oss.str();
    }

    const std::string get_selector_name() const {
        std::stringstream oss;

        for(names_type::const_iterator it = names.begin();
            it != names.end();
            ++it) {
            boost::apply_visitor(selector_visitor(oss), *it);
        }

        for(filters_type::const_iterator it = filters.begin();
            it != filters.end();
            ++it) {
            oss << it->get_selector_name();
        }

        if(attachment_selector) oss << attachment_selector->get_selector_name();

        return oss.str();
    }

    struct specificity_comparator {
        bool operator()(rule const &lhs, rule const& rhs) {
            return lhs.specificity() < rhs.specificity();
        }
    };
};

class stylesheet {
public:
    stylesheet() : rules(), map_style() { }

    typedef std::multiset<rule, rule::specificity_comparator> rules_type;
    rules_type rules;

    typedef std::map<std::string, utree> map_style_type;
    map_style_type map_style;

    inline void accept(visitor &visitor) const {
        visitor.visit(*this);
    }
};

} }

#endif
