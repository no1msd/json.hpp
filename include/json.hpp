#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/lexical_cast.hpp>

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
                  sp::qi::lit("false")[sp::_val = false] | "null";
    string_rule = sp::lexeme['"' >> *((escaped | sp::qi::char_) - '"') >> '"'];
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

template<class Container, class Transformer>
std::string trans_join(const Container& container, Transformer transformer) {
  std::vector<std::string> out(container.size());
  std::transform(container.begin(), container.end(), out.begin(), transformer);
  return boost::algorithm::join(out, ",");
}

template<class Node, class Map, class Array>
struct json_visitor: boost::static_visitor<std::string> {
  template<class T>
  std::string operator()(const T& value) const {
    return boost::lexical_cast<std::string>(value);
  }

  std::string operator()(const std::nullptr_t&) const {
    return "null";
  }

  std::string operator()(const std::string& value) const {
    return escape_str(value);
  }

  std::string operator()(const bool& value) const {
    return value ? "true" : "false";
  }

  std::string operator()(const Map& value) const {
    return "{" + trans_join(value, render_map_item<Node, Map, Array>) + "}";
  }

  std::string operator()(const Array& value) const {
    return "[" + trans_join(value, stringify<Node, Map, Array>) + "]";
  }
};

}

template<class Node = node, class Map = map, class Array = array>
Node parse(const std::string& str) {
  Node out;
  details::json_grammar<std::string::const_iterator, Node, Map, Array> json;
  auto first = str.begin(), last = str.end();
  bool matched = boost::spirit::qi::phrase_parse(
      first, last, json, boost::spirit::ascii::space, out);
  return (!matched || first != last) ? Node() : out;
}

template<class Node, class Map, class Array>
std::string stringify(const Node& node) {
  return boost::apply_visitor(details::json_visitor<Node, Map, Array>(), node);
}

}
