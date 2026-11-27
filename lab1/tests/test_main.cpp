#include "polynomial.hpp"
#include "set.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

TEST_CASE("Множество хранит уникальные элементы и поддерживает удаление и поиск") {
    Set set;
    REQUIRE(set.empty());
    set.add("alpha");
    set.add("alpha");
    set.add("beta");

    REQUIRE_FALSE(set.empty());
    REQUIRE(set.cardinality() == 2);
    REQUIRE(set.contains("alpha"));
    REQUIRE_FALSE(set.contains("missing"));
    REQUIRE(set["alpha"]);
    REQUIRE_FALSE(set["missing"]);
    REQUIRE(set.remove("alpha"));
    REQUIRE_FALSE(set.remove("alpha"));
    REQUIRE(set.cardinality() == 1);
}

TEST_CASE("Вложенные множества сравниваются без учёта порядка элементов") {
    const Set first = Set::fromString("{a, {c, b}, {}}");
    const Set second = Set::fromString("{{}, {b, c}, a}");
    const Set inner = Set::fromString("{c, b}");

    REQUIRE(first == second);
    REQUIRE(first.contains(inner));
    REQUIRE(first[inner]);
    REQUIRE(first.toString() == second.toString());
    REQUIRE(SetElement("a") != SetElement(Set::fromString("{a}")));
}

TEST_CASE("Разбор множества поддерживает пустые множества, вложенность и пробелы") {
    REQUIRE(Set::fromString("{}").empty());
    REQUIRE(Set::fromString("{red apple, blue, {x, y}}").cardinality() == 3);
    REQUIRE(Set::fromString("{a, b}") == Set::fromString("{b, a}"));
}

TEST_CASE("Разбор множества отклоняет некорректную запись и недопустимые атомы") {
    REQUIRE_THROWS_AS(Set::fromString("a"), std::invalid_argument);
    REQUIRE_THROWS_AS(Set::fromString("{a,}"), std::invalid_argument);
    REQUIRE_THROWS_AS(Set::fromString("{a"), std::invalid_argument);
    REQUIRE_THROWS_AS(Set::fromString("{a}}"), std::invalid_argument);
    REQUIRE_THROWS_AS(SetElement(""), std::invalid_argument);
    REQUIRE_THROWS_AS(SetElement("   "), std::invalid_argument);
    REQUIRE_THROWS_AS(SetElement("a,b"), std::invalid_argument);
    REQUIRE_THROWS_AS(SetElement(Set{}).atom(), std::logic_error);
    REQUIRE_THROWS_AS(SetElement("atom").nestedSet(), std::logic_error);
}

TEST_CASE("Объединение, пересечение и разность множеств") {
    const Set left = Set::fromString("{a, b, c}");
    const Set right = Set::fromString("{b, c, d}");

    REQUIRE(left + right == Set::fromString("{a, b, c, d}"));
    REQUIRE(left * right == Set::fromString("{b, c}"));
    REQUIRE(left - right == Set::fromString("{a}"));

    Set compound = left;
    compound += right;
    REQUIRE(compound == left + right);
    compound *= right;
    REQUIRE(compound == Set::fromString("{b, c, d}"));
    compound -= Set::fromString("{c}");
    REQUIRE(compound == Set::fromString("{b, d}"));
}

TEST_CASE("Булеан содержит пустое множество и исходное множество") {
    const Set input = Set::fromString("{a, b}");
    const Set result = input.powerSet();

    REQUIRE(result.cardinality() == 4);
    REQUIRE(result.contains(Set{}));
    REQUIRE(result.contains(input));
    REQUIRE(result.contains(Set::fromString("{a}")));
    REQUIRE(result.contains(Set::fromString("{b}")));
    REQUIRE(Set{}.powerSet() == Set::fromString("{{}}"));
}

TEST_CASE("Потоковый ввод и вывод множеств и обработка ошибок") {
    const Set original = Set::fromString("{a, {b, c}}");
    std::stringstream valid;
    valid << original;
    Set parsed;
    valid >> parsed;
    REQUIRE(parsed == original);

    std::istringstream invalid("{a,}");
    invalid >> parsed;
    REQUIRE(invalid.fail());
}

TEST_CASE("Многочлен нормализует коэффициенты и возвращает ноль "
          "для отсутствующей степени") {
    Polynomial zero;
    Polynomial normalized({1.0, 2.0, 0.0, 1e-12});
    REQUIRE(zero.degree() == 0);
    REQUIRE(zero[0] == 0.0);
    REQUIRE(normalized.degree() == 1);
    REQUIRE(normalized[0] == 1.0);
    REQUIRE(normalized[1] == 2.0);
    REQUIRE(normalized[20] == 0.0);
    REQUIRE(Polynomial(std::vector<double>{}).degree() == 0);
    REQUIRE_THROWS_AS(Polynomial({1.0, std::numeric_limits<double>::infinity()}),
                      std::invalid_argument);
}

TEST_CASE("Вычисление многочлена и проверка аргумента") {
    const Polynomial polynomial({1.0, 2.0, 3.0});
    REQUIRE(polynomial.evaluate(2.0) == Catch::Approx(17.0));
    REQUIRE(polynomial(2.0) == Catch::Approx(17.0));
    REQUIRE(Polynomial{}.evaluate(10.0) == 0.0);
    REQUIRE(Polynomial{}(10.0) == 0.0);
    REQUIRE_THROWS_AS(
        polynomial.evaluate(std::numeric_limits<double>::infinity()),
        std::invalid_argument);
    REQUIRE_THROWS_AS(
        polynomial(std::numeric_limits<double>::infinity()),
        std::invalid_argument);
}

TEST_CASE("Копирование множества и многочлена сохраняет независимые значения") {
    Set originalSet = Set::fromString("{a, {b}}");
    const Set copiedSet = originalSet;
    Set assignedSet;
    assignedSet = originalSet;
    originalSet.remove("a");
    REQUIRE(copiedSet.contains("a"));
    REQUIRE(assignedSet.contains("a"));

    Polynomial originalPolynomial({1.0, 2.0, 3.0});
    const Polynomial copiedPolynomial = originalPolynomial;
    Polynomial assignedPolynomial;
    assignedPolynomial = originalPolynomial;
    originalPolynomial += Polynomial({1.0});
    REQUIRE(copiedPolynomial == Polynomial({1.0, 2.0, 3.0}));
    REQUIRE(assignedPolynomial == Polynomial({1.0, 2.0, 3.0}));
}

TEST_CASE("Сложение и вычитание многочленов разной степени") {
    const Polynomial first({1.0, 2.0, 3.0});
    const Polynomial second({4.0, 5.0});

    REQUIRE(first + second == Polynomial({5.0, 7.0, 3.0}));
    REQUIRE(first - second == Polynomial({-3.0, -3.0, 3.0}));
    REQUIRE(second - first == Polynomial({3.0, 3.0, -3.0}));

    Polynomial compound = first;
    compound += second;
    REQUIRE(compound == first + second);
    compound -= second;
    REQUIRE(compound == first);
}

TEST_CASE("Умножение многочленов и умножение на нулевой многочлен") {
    const Polynomial first({1.0, 1.0});
    const Polynomial second({-1.0, 1.0});
    REQUIRE(first * second == Polynomial({-1.0, 0.0, 1.0}));

    Polynomial compound = first;
    compound *= second;
    REQUIRE(compound == first * second);
    REQUIRE((first * Polynomial{}) == Polynomial{});
}

TEST_CASE("Деление многочленов возвращает частное и остаток") {
    const Polynomial dividend({-1.0, 0.0, 1.0});
    const Polynomial divisor({-1.0, 1.0});
    const auto result = dividend.divideWithRemainder(divisor);

    REQUIRE(result.first == Polynomial({1.0, 1.0}));
    REQUIRE(result.second == Polynomial{});
    REQUIRE(dividend / divisor == result.first);

    const auto nonExact = Polynomial({1.0, 0.0, 1.0})
                              .divideWithRemainder(Polynomial({1.0, 1.0}));
    REQUIRE(nonExact.first == Polynomial({-1.0, 1.0}));
    REQUIRE(nonExact.second == Polynomial({2.0}));
    REQUIRE(Polynomial({1.0}).divideWithRemainder(Polynomial({1.0, 1.0})).first ==
            Polynomial{});
}

TEST_CASE("Деление на нулевой многочлен запрещено, составное деление работает") {
    const Polynomial dividend({-1.0, 0.0, 1.0});
    REQUIRE_THROWS_AS(dividend / Polynomial{}, std::domain_error);
    REQUIRE_THROWS_AS(dividend.divideWithRemainder(Polynomial{}), std::domain_error);

    Polynomial compound = dividend;
    compound /= Polynomial({-1.0, 1.0});
    REQUIRE(compound == Polynomial({1.0, 1.0}));
}

TEST_CASE("Равенство, форматирование и потоковый ввод-вывод многочленов") {
    const Polynomial polynomial({1.0, -2.0, 1.0});
    REQUIRE(polynomial == Polynomial({1.0 + 1e-11, -2.0, 1.0}));
    REQUIRE(polynomial != Polynomial({1.0, -2.0}));
    REQUIRE(polynomial.toString() == "x^2 - 2*x + 1");
    REQUIRE(Polynomial{}.toString() == "0");

    std::stringstream stream;
    stream << polynomial;
    Polynomial parsed;
    stream >> parsed;
    REQUIRE(parsed == polynomial);

    std::istringstream invalid("[1, invalid]");
    invalid >> parsed;
    REQUIRE(invalid.fail());
    std::istringstream trailingComma("[1, 2,]");
    trailingComma >> parsed;
    REQUIRE(trailingComma.fail());
    REQUIRE_THROWS_AS(Polynomial({1.0, std::numeric_limits<double>::quiet_NaN()}),
                      std::invalid_argument);
}
