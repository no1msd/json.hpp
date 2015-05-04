# json.hpp - C++ JSON parsing with Boost.Spirit

json.hpp is a tiny (less than 100 lines of code) C++ JSON parsing library using 
[Boost.Spirit](http://www.boost.org/doc/libs/1_58_0/libs/spirit/doc/html/index.html) 
for parsing, and [Boost.Variant](http://www.boost.org/doc/libs/1_58_0/doc/html/variant.html)
for the data structure. It works seamlessly with [mstch](https://github.com/no1msd/mstch), 
a [{{mustache}}](http://mustache.github.io/) template library.

## Motivation

The world doesn't really need another JSON parser, and I'm sure there are others
who used Spirit for parsing JSON. My main goal was simply to try out Spirit in
a real use case, and to be able to parse JSON to a ```boost::variant``` as easily as
possible.

## Usage

Parsing JSON:

```c++
#include <iostream>
#include <json.hpp>

int main() {
  using boost::get;

  std::string json_string{
    "{\"names\": ["
    "  {\"name\": \"Chris\"},"
    "  {\"name\": \"Mark\"},"
    "  {\"name\": \"Scott\"}"
    "]}"};
  auto data = json::parse(json_string);
  
  for(auto& name_item: get<json::array>(get<json::map>(data)["names"]))
    std::cout << get<std::string>(get<json::map>(name_item)["name"]) << std::endl;
  
  return 0;
}
```

Output:

```html
Chris
Mark
Scott
```

You could also use your own compatible data type for storing the result:

```c++
auto data = json::parse<mstch::node, mstch::map, mstch::array>(json_string);
```

Serializing to JSON:

```c++
#include <iostream>
#include <json.hpp>

int main() {
  json::map data{
    {"names", json::array{
      json::map{{"name", std::string{"Chris"}}},
      json::map{{"name", std::string{"Mark"}}},
      json::map{{"name", std::string{"Scott"}}},
    }}
  };
  
  auto json_string = json::stringify(data);
  std::cout << json_string << std::endl;
    
  return 0;
}
```

Output:

```javascript
{"names": [{"name": "Chris"}, {"name": "Mark"}, {"name": "Scott"}]}
```

Using your own data type:

```c++
auto json_string = json::stringify<mstch::node>(data);
```

### Data structure

The types in the example above, `json::array` and `json::map` are  actually 
aliases for standard types:

```c++
using map = std::map<const std::string, node>;
using array = std::vector<node>;
```

`json::node` is a `boost::variant` that can hold a `std::string`, `int`, `double`
, `bool`, or a map, or an array recursively. Essentially it works just like 
a JSON object.

For more information on using variants see the boost [documentation](http://boost.org/doc/libs/1_58_0/doc/html/variant.html).

## Requirements

 - A C++ compiler with decent C++11 support.
 - Boost 1.54+
 - CMake 2.8+ for building the unit tests

## Installing

Just copy the header file to your project.

## Running the unit tests

Unit tests are using the [Catch](https://github.com/philsquared/Catch) framework.

```bash
 $ mkdir build
 $ cd build
 $ cmake ..
 $ make test
```

## License

json.hpp is licensed under the [MIT license](https://github.com/no1msd/json.hpp/blob/master/LICENSE).
