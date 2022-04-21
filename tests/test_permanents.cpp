#include <complex>
#include <catch2/catch.hpp>
#include "../src/permanent.h"
#include <iostream>

static std::vector<std::complex<double>> genSquaredMatrixComplex(int squaredMatrixSize)
{
    std::vector<std::complex<double>> mat(squaredMatrixSize*squaredMatrixSize);
    auto increment = 1.0 / (squaredMatrixSize * squaredMatrixSize);

    auto generateComplexSuite = [&increment] {
        static std::complex<double> base(0, 0);
        base += std::complex<double>(increment, increment/2);
        return base;
    };

    std::generate(mat.begin(), mat.end(), generateComplexSuite);
    return mat;
}

static std::vector<double> genSquaredMatrixDouble(int squaredMatrixSize)
{
    auto increment = 1.0 / (squaredMatrixSize * squaredMatrixSize);
    auto generateSuite = [&increment]() {
        static double base = 0;
        base += increment;
        return base;
    };

    std::vector<double> mat(squaredMatrixSize*squaredMatrixSize);
    std::generate(mat.begin(), mat.end(), generateSuite);
    return mat;
}

static bool isApproximatelyEqual(double a, double b, double tolerance = std::numeric_limits<double>::epsilon() * 5)
{
    double diff = std::abs(a - b);
    if (diff <= tolerance)
        return true;

    return false;
}

static bool isApproximatelyEqual(std::complex<double> a, std::complex<double> b, double tolerance = std::numeric_limits<double>::epsilon() * 5)
{
    double diff = std::abs(a.real() - b.real());
    if (diff > tolerance)
        return false;

    diff = std::abs(a.imag() - b.imag());
    if (diff > tolerance)
        return false;

    return true;
}

SCENARIO("C++ Testing Permanents") {
    GIVEN("the glynn algorythm") {
        WHEN("computing a double matrix of size 2") {
            std::vector<double> matrixSize2 = {1, 2, 3, 4};

            THEN("build Succeed !") {
                auto res = permanent_glynn(matrixSize2.data(), 2);
                REQUIRE(isApproximatelyEqual(res, 10.));
            }
        }
        WHEN("computing a double matrix of size 5") {
            std::vector<double> matrix = genSquaredMatrixDouble(5);
            THEN("build Succeed !") {
                auto res = permanent_glynn(matrix.data(), 5);
                REQUIRE(isApproximatelyEqual(res, 1.4828236800000005));
            }
        }
        WHEN("computing a complex<double> matrix of size 5") {
            std::vector<std::complex<double>> matrix = genSquaredMatrixComplex(5);
            THEN("build Succeed !") {
                auto res = permanent_glynn(matrix.data(), 5);
                REQUIRE(isApproximatelyEqual(res, std::complex<double>(-1.760853120000001, 1.8998678400000022)));
            }
        }
    }
}