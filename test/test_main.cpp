#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "json.hpp"

TEST_CASE("JSON") {
  using boost::get;

  std::string orig_str{
      "{\n"
      "  \"firstName\": \"John\",\n"
      "  \"lastName\": \"Smith\",\n"
      "  \"isAlive\": true,\n"
      "  \"age\": 25,\n"
      "  \"double\": 42.42,\n"
      "  \"address\": {\n"
      "    \"streetAddress\": \"21 2nd Street\",\n"
      "    \"city\": \"New York\",\n"
      "    \"state\": \"NY\",\n"
      "    \"postalCode\": \"10021-3100\"\n"
      "  },\n"
      "  \"phoneNumbers\": [\n"
      "    {\n"
      "      \"type\": \"home\",\n"
      "      \"number\": \"212 555-1234\"\n"
      "    },\n"
      "    {\n"
      "      \"type\": \"office\",\n"
      "      \"number\": \"646 555-4567\"\n"
      "    }\n"
      "  ],\n"
      "  \"children\": [],\n"
      "  \"spouse\": null,\n"
      "  \"emptyobj\": {},\n"
      "  \"escaped\": \"hello\\\"world\\\\!\"\n"
      "}"};

  auto data = json::parse(orig_str);
  REQUIRE(get<std::string>(get<json::map>(data)["firstName"]) == "John");
  REQUIRE(get<bool>(get<json::map>(data)["isAlive"]) == true);
  REQUIRE(get<double>(get<json::map>(data)["age"]) == 25);
  REQUIRE(get<double>(get<json::map>(data)["double"]) == 42.42);
  REQUIRE(get<std::string>(get<json::map>(get<json::map>(data)["address"])["city"]) == "New York");
  REQUIRE(get<json::array>(get<json::map>(data)["phoneNumbers"]).size() == 2);

  auto str_generated = json::stringify(data);
  auto data2 = json::parse(str_generated);
  REQUIRE(get<std::string>(get<json::map>(data2)["firstName"]) == "John");
  REQUIRE(get<bool>(get<json::map>(data2)["isAlive"]) == true);
  REQUIRE(get<double>(get<json::map>(data2)["age"]) == 25);
  REQUIRE(get<double>(get<json::map>(data2)["double"]) == 42.42);
  REQUIRE(get<std::string>(get<json::map>(get<json::map>(data2)["address"])["city"]) == "New York");
  REQUIRE(get<json::array>(get<json::map>(data2)["phoneNumbers"]).size() == 2);

  REQUIRE(str_generated == json::stringify(data2));
};
