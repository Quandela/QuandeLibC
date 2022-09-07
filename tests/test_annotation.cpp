// MIT License
//
// Copyright (c) 2022 Quandela
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


#include <catch2/catch.hpp>
#include "../src/annotation.h"

SCENARIO("C++ Testing Annotation") {
    SECTION("parse incorrect annotations") {
        REQUIRE_THROWS_AS(annotation("12:0"), std::invalid_argument);
        REQUIRE_THROWS_AS(annotation("test"), std::invalid_argument);
        REQUIRE_THROWS_AS(annotation("test:"), std::invalid_argument);
        REQUIRE_THROWS_AS(annotation("test:a"), std::invalid_argument);
        REQUIRE_THROWS_AS(annotation("test:0,"), std::invalid_argument);
        REQUIRE_THROWS_AS(annotation("test:(1234)"), std::invalid_argument);
        REQUIRE_THROWS_AS(annotation("test:(1234,2i)"), std::invalid_argument);
        REQUIRE_THROWS_AS(annotation("P:X"), std::invalid_argument);
        REQUIRE_THROWS_AS(annotation("P:0+1"), std::invalid_argument);
        REQUIRE_THROWS_AS(annotation("P:1i+0"), std::invalid_argument);
        REQUIRE_THROWS_AS(annotation("P:1i+1i"), std::invalid_argument);
        REQUIRE_THROWS_AS(annotation("P:1+i"), std::invalid_argument);
    }
    SECTION("parse real value annotations") {
        annotation a1("_:0");
        THEN("annotation name and values are ok") {
            REQUIRE(a1.name() == "_");
            REQUIRE(a1.value() == std::complex<float>(0));
            REQUIRE(a1.to_str() == "_:0");
        }
        annotation a2("test:-1.345");
        THEN("annotation name and values are ok") {
            REQUIRE(a2.name() == "test");
            REQUIRE(a2.value() == std::complex<float>(-1.345));
            REQUIRE(a2.to_str() == "test:-1.345");
        }
    }
    SECTION("parse complex value annotations") {
        annotation a1("_:1i");
        THEN("annotation name and values are ok") {
            REQUIRE(a1.name() == "_");
            REQUIRE(a1.value() == std::complex<float>(0, 1));
            REQUIRE(a1.to_str() == "_:(0,1)");
        }
        annotation a2("AB12:2-3i");
        THEN("annotation name and values are ok") {
            REQUIRE(a2.name() == "AB12");
            REQUIRE(a2.value() == std::complex<float>(2, -3));
            REQUIRE(a2.to_str() == "AB12:(2,-3)");
        }
        annotation a3("AB12:2+3e-4i");
        THEN("annotation name and values are ok") {
            REQUIRE(a3.name() == "AB12");
            REQUIRE(a3.value() == std::complex<float>(2, 0.0003));
            REQUIRE(a3.to_str() == "AB12:(2,0.0003)");
        }
        annotation a4("AB_12:2+-3e-4i");
        THEN("annotation name and values are ok") {
            REQUIRE(a4.name() == "AB_12");
            REQUIRE(a4.value() == std::complex<float>(2, -0.0003));
            REQUIRE(a4.to_str() == "AB_12:(2,-0.0003)");
        }
        annotation a5("_:1j");
        THEN("annotation name and values are ok") {
            REQUIRE(a5.name() == "_");
            REQUIRE(a5.value() == std::complex<float>(0, 1));
            REQUIRE(a5.to_str() == "_:(0,1)");
        }
    }
    SECTION("parse float real value annotations") {
        annotation a1("_:(0.3,-0.5)");
        THEN("annotation name and values are ok") {
            REQUIRE(a1.name() == "_");
            REQUIRE(a1.value() == std::complex<float>(0.3, -0.5));
            REQUIRE(a1.to_str() == "_:(0.3,-0.5)");
        }
    }
    SECTION("parse polarization annotations") {
        annotation a1("P:(0.3,-0.5)");
        THEN("annotation name and values are ok") {
            REQUIRE(a1.name() == "P");
            REQUIRE(a1.value() == std::complex<float>(0.3, -0.5));
            REQUIRE(a1.to_str() == "P:(0.3,-0.5)");
        }
        annotation a2("P:H");
        THEN("annotation name and values are ok") {
            REQUIRE(a2.name() == "P");
            REQUIRE(a2.value() == std::complex<float>(0, 0));
            REQUIRE(a2.to_str() == "P:H");
        }
        annotation a3("P:V");
        THEN("annotation name and values are ok") {
            REQUIRE(a3.name() == "P");
            REQUIRE(a3.value().real() == Approx(3.1415926536));
            REQUIRE(a3.value().imag() == Approx(0));
            REQUIRE(a3.to_str() == "P:V");
        }
        annotation a4("P:D");
        THEN("annotation name and values are ok") {
            REQUIRE(a4.name() == "P");
            REQUIRE(a4.value().real() == Approx(3.1415926536 / 2));
            REQUIRE(a4.value().imag() == Approx(0));
            REQUIRE(a4.to_str() == "P:D");
        }
        annotation a5("P:A");
        THEN("annotation name and values are ok") {
            REQUIRE(a5.name() == "P");
            REQUIRE(a5.value().real() == Approx(3.1415926536 / 2));
            REQUIRE(a5.value().imag() == Approx(3.1415926536));
            REQUIRE(a5.to_str() == "P:A");
        }
        annotation a6("P:L");
        THEN("annotation name and values are ok") {
            REQUIRE(a6.name() == "P");
            REQUIRE(a6.value().real() == Approx(3.1415926536 / 2));
            REQUIRE(a6.value().imag() == Approx(3.1415926536 / 2));
            REQUIRE(a6.to_str() == "P:L");
        }
        annotation a7("P:R");
        THEN("annotation name and values are ok") {
            REQUIRE(a7.name() == "P");
            REQUIRE(a7.value().real() == Approx(3.1415926536 / 2));
            REQUIRE(a7.value().imag() == Approx(3 * 3.1415926536 / 2));
            REQUIRE(a7.to_str() == "P:R");
        }
    }
    SECTION("direct constructor") {
        annotation a1("P", std::complex<float>(0, 0));
        REQUIRE(a1.to_str() == "P:H");
    }
}
