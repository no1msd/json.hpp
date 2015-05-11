#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <map>

namespace json {

using node = boost::make_recursive_variant<
    std::nullptr_t, std::string, int, double, bool,
    std::map<const std::string, boost::recursive_variant_>,
    std::vector<boost::recursive_variant_>>::type;
using map = std::map<const std::string, node>;
using array = std::vector<node>;

template<class Node = node, class Map = map, class Array = array>
std::string stringify(const Node& node);

namespace details {

namespace sp = boost::spirit;

template<class Iterator, class Node, class Map, class Array>
struct json_grammar: sp::qi::grammar<Iterator, Node(), sp::ascii::space_type> {
  struct esc_parser: sp::qi::symbols<char,char> {
    esc_parser() {
      add("\\\\" , '\\') ("\\\"" , '"' ) ("\\n"  , '\n') ("\\r"  , '\r')
         ("\\b"  , '\b') ("\\f"  , '\f') ("\\t"  , '\t');
    }
  } escaped;

  json_grammar(): json_grammar::base_type(value_rule) {
    value_rule %= string_rule | sp::qi::double_ | object_rule | array_rule |
                  sp::qi::lit("true" )[sp::_val = true ] |
                  sp::qi::lit("false")[sp::_val = false] |
                  "null";
    string_rule = sp::lexeme['"' >> *(escaped | (sp::qi::char_ - '"')) >> '"'];
    object_rule = '{' >> -((string_rule >> ':' >> value_rule) % ',') >> '}';
    array_rule  = '[' >> -(value_rule % ',') >> ']';
  }

  sp::qi::rule<Iterator, Node(),        sp::ascii::space_type> value_rule;
  sp::qi::rule<Iterator, Map(),         sp::ascii::space_type> object_rule;
  sp::qi::rule<Iterator, Array(),       sp::ascii::space_type> array_rule;
  sp::qi::rule<Iterator, std::string(), sp::ascii::space_type> string_rule;
};

std::string escape_str(std::string str) {
  boost::replace_all(str, "\\", "\\\\");
  boost::replace_all(str, "\"", "\\\"");
  return "\"" + str + "\"";
}

template<class Node, class Map, class Array>
std::string render_map_item(const typename Map::value_type& i) {
  return escape_str(i.first) + ":" + stringify<Node, Map, Array>(i.second);
}

template<class Node, class Map, class Array>
struct json_visitor: boost::static_visitor<std::string> {
  std::string operator()(const std::nullptr_t& v) const {
    return "null";
  }

  std::string operator()(const std::string& v) const {
    return escape_str(v);
  }

  std::string operator()(const int& v) const {
    return std::to_string(v);
  }

  std::string operator()(const double& v) const {
    std::stringstream ss;
    ss << v;
    return ss.str();
  }

  std::string operator()(const bool& v) const {
    return v ? "true" : "false";
  }

  std::string operator()(const Map& value) const {
    std::vector<std::string> out(value.size());
    std::transform(value.begin(), value.end(), out.begin(),
        render_map_item<Node, Map, Array>);
    return "{" + boost::algorithm::join(out, ",") + "}";
  }

  std::string operator()(const Array& value) const {
    std::vector<std::string> out(value.size());
    std::transform(value.begin(), value.end(), out.begin(),
        stringify<Node, Map, Array>);
    return "[" + boost::algorithm::join(out, ",") + "]";
  }
};

}

template<class Node = node, class Map = map, class Array = array>
Node parse(const std::string& str) {
  Node out;
  details::json_grammar<std::string::const_iterator, Node, Map, Array> json;
  auto first = str.begin(), last = str.end();
  bool ret = boost::spirit::qi::phrase_parse(
      first, last, json, boost::spirit::ascii::space, out);
  return (!ret || first != last) ? Node() : out;
}

template<class Node = node, class Map = map, class Array = array>
std::string stringify(const Node& node) {
  return boost::apply_visitor(details::json_visitor<Node, Map, Array>(), node);
}

}
