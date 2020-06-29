#include <catch.hpp>

#include <mw/serialization/Json.h>

TEST_CASE("Json")
{
    // TODO: Add array, null, object, etc
    json json({
        {"int8_t", (int8_t)-7},
        {"uint8_t", (uint8_t)7},
        {"double", (double)17.1},
        {"bool", false},
        {"string", "hello"}
    });

    Json parser(json);

    {
        boost::optional<int8_t> int8_tOpt = parser.Get<int8_t>("int8_t");
        REQUIRE(int8_tOpt.has_value());
        REQUIRE(int8_tOpt.value() == -7);

        boost::optional<uint8_t> uint8_tOpt = parser.Get<uint8_t>("int8_t");
        REQUIRE(uint8_tOpt.has_value());
        REQUIRE(uint8_tOpt.value() == (uint8_t)-7);

        boost::optional<double> doubleOpt = parser.Get<double>("int8_t");
        REQUIRE(doubleOpt.has_value());
        REQUIRE(doubleOpt.value() == (double)-7);

        boost::optional<std::string> stringOpt = parser.Get<std::string>("int8_t");
        REQUIRE(!stringOpt.has_value());
    }

    {
        boost::optional<int8_t> int8_tOpt = parser.Get<int8_t>("uint8_t");
        REQUIRE(int8_tOpt.has_value());
        REQUIRE(int8_tOpt.value() == 7);

        boost::optional<uint8_t> uint8_tOpt = parser.Get<uint8_t>("uint8_t");
        REQUIRE(uint8_tOpt.has_value());
        REQUIRE(uint8_tOpt.value() == 7);

        boost::optional<double> doubleOpt = parser.Get<double>("uint8_t");
        REQUIRE(doubleOpt.has_value());
        REQUIRE(doubleOpt.value() == (double)7);

        boost::optional<std::string> stringOpt = parser.Get<std::string>("uint8_t");
        REQUIRE(!stringOpt.has_value());
    }

    {
        boost::optional<int8_t> int8_tOpt = parser.Get<int8_t>("double");
        REQUIRE(int8_tOpt.has_value());
        REQUIRE(int8_tOpt.value() == 17);

        boost::optional<uint8_t> uint8_tOpt = parser.Get<uint8_t>("double");
        REQUIRE(uint8_tOpt.has_value());
        REQUIRE(uint8_tOpt.value() == 17);

        boost::optional<double> doubleOpt = parser.Get<double>("double");
        REQUIRE(doubleOpt.has_value());
        REQUIRE(doubleOpt.value() == 17.1);

        boost::optional<std::string> stringOpt = parser.Get<std::string>("double");
        REQUIRE(!stringOpt.has_value());
    }

    {
        boost::optional<int8_t> int8_tOpt = parser.Get<int8_t>("string");
        REQUIRE(!int8_tOpt.has_value());

        boost::optional<uint8_t> uint8_tOpt = parser.Get<uint8_t>("string");
        REQUIRE(!uint8_tOpt.has_value());

        boost::optional<double> doubleOpt = parser.Get<double>("string");
        REQUIRE(!doubleOpt.has_value());

        boost::optional<std::string> stringOpt = parser.Get<std::string>("string");
        REQUIRE(stringOpt.has_value());
        REQUIRE(stringOpt.value() == "hello");
    }
}