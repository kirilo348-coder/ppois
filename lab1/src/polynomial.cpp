#include "polynomial.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <istream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

constexpr double kEpsilon = 1e-10;

bool isZero(double value) {
    return std::abs(value) <= kEpsilon;
}

bool hasValidCommaList(const std::string& line) {
    const auto first = line.find_first_not_of(" \t\r\n");
    const auto last = line.find_last_not_of(" \t\r\n");
    if (first == std::string::npos || line[first] == ',' || line[last] == ',') {
        return false;
    }

    bool previousWasComma = false;
    for (char character : line) {
        if (character == ',' && previousWasComma) {
            return false;
        }
        if (!std::isspace(static_cast<unsigned char>(character))) {
            previousWasComma = character == ',';
        }
    }
    return true;
}

bool normalizeCoefficientText(std::string& line) {
    const bool opensBracket = !line.empty() && line.front() == '[';
    const bool closesBracket = !line.empty() && line.back() == ']';
    if (opensBracket != closesBracket) {
        return false;
    }
    if (!opensBracket) {
        return true;
    }

    line = line.substr(1, line.size() - 2);
    if (!hasValidCommaList(line)) {
        return false;
    }
    std::replace(line.begin(), line.end(), ',', ' ');
    return true;
}

std::vector<double> parseCoefficientList(const std::string& text) {
    std::istringstream input(text);
    std::vector<double> coefficients;
    double coefficient = 0.0;
    while (input >> coefficient) {
        coefficients.push_back(coefficient);
    }
    if (coefficients.empty() || !input.eof()) {
        throw std::invalid_argument("Некорректный список коэффициентов");
    }
    return coefficients;
}

void trim(std::vector<double>& coefficients) {
    while (coefficients.size() > 1 && isZero(coefficients.back())) {
        coefficients.pop_back();
    }
}

std::pair<std::size_t, double> reduceRemainder(
    std::vector<double>& remainder, const std::vector<double>& divisor) {
    const std::size_t shift = remainder.size() - divisor.size();
    const double factor = remainder.back() / divisor.back();
    for (std::size_t index = 0; index < divisor.size(); ++index) {
        remainder[shift + index] -= factor * divisor[index];
    }
    trim(remainder);
    return {shift, factor};
}

void appendTerm(std::ostringstream& output, double coefficient,
                std::size_t exponent, bool& hasTerm) {
    if (isZero(coefficient)) {
        return;
    }
    if (hasTerm) {
        output << (coefficient < 0.0 ? " - " : " + ");
    } else if (coefficient < 0.0) {
        output << '-';
    }
    const double magnitude = std::abs(coefficient);
    if (exponent == 0 || !isZero(magnitude - 1.0)) {
        output << magnitude;
        if (exponent != 0) {
            output << '*';
        }
    }
    if (exponent != 0) {
        output << 'x';
        if (exponent != 1) {
            output << '^' << exponent;
        }
    }
    hasTerm = true;
}

} // namespace

Polynomial::Polynomial() : coefficients_{0.0} {}

Polynomial::Polynomial(std::vector<double> coefficients)
    : coefficients_(std::move(coefficients)) {
    if (coefficients_.empty()) {
        coefficients_.push_back(0.0);
    }
    if (!std::all_of(coefficients_.begin(), coefficients_.end(),
                     [](double value) { return std::isfinite(value); })) {
        throw std::invalid_argument("Коэффициенты многочлена должны быть конечными числами");
    }
    normalize();
}

int Polynomial::degree() const noexcept {
    return static_cast<int>(coefficients_.size()) - 1;
}

const std::vector<double>& Polynomial::coefficients() const noexcept {
    return coefficients_;
}

double Polynomial::evaluate(double argument) const {
    if (!std::isfinite(argument)) {
        throw std::invalid_argument("Аргумент должен быть конечным числом");
    }
    double value = 0.0;
    for (auto coefficient = coefficients_.rbegin();
         coefficient != coefficients_.rend(); ++coefficient) {
        value = value * argument + *coefficient;
    }
    return value;
}

double Polynomial::operator()(double argument) const {
    return evaluate(argument);
}

std::pair<Polynomial, Polynomial>
Polynomial::divideWithRemainder(const Polynomial& divisor) const {
    if (divisor.coefficients_.size() == 1 && isZero(divisor.coefficients_[0])) {
        throw std::domain_error("Деление на нулевой многочлен");
    }

    std::vector<double> remainder = coefficients_;
    if (degree() < divisor.degree()) {
        return {Polynomial{}, *this};
    }

    std::vector<double> quotient(
        static_cast<std::size_t>(degree() - divisor.degree() + 1), 0.0);

    while (remainder.size() >= divisor.coefficients_.size() &&
           !(remainder.size() == 1 && isZero(remainder[0]))) {
        const auto [shift, factor] = reduceRemainder(remainder, divisor.coefficients_);
        quotient[shift] = factor;
    }

    return {Polynomial(std::move(quotient)), Polynomial(std::move(remainder))};
}

std::string Polynomial::toString() const {
    std::ostringstream output;
    bool hasTerm = false;
    for (std::size_t exponent = coefficients_.size(); exponent-- > 0;) {
        appendTerm(output, coefficients_[exponent], exponent, hasTerm);
    }
    return hasTerm ? output.str() : "0";
}

double Polynomial::operator[](std::size_t exponent) const noexcept {
    return exponent < coefficients_.size() ? coefficients_[exponent] : 0.0;
}

Polynomial& Polynomial::operator+=(const Polynomial& other) {
    coefficients_.resize(
        std::max(coefficients_.size(), other.coefficients_.size()), 0.0);
    for (std::size_t index = 0; index < other.coefficients_.size(); ++index) {
        coefficients_[index] += other.coefficients_[index];
    }
    normalize();
    return *this;
}

Polynomial& Polynomial::operator-=(const Polynomial& other) {
    coefficients_.resize(
        std::max(coefficients_.size(), other.coefficients_.size()), 0.0);
    for (std::size_t index = 0; index < other.coefficients_.size(); ++index) {
        coefficients_[index] -= other.coefficients_[index];
    }
    normalize();
    return *this;
}

Polynomial& Polynomial::operator*=(const Polynomial& other) {
    if ((coefficients_.size() == 1 && isZero(coefficients_[0])) ||
        (other.coefficients_.size() == 1 && isZero(other.coefficients_[0]))) {
        coefficients_ = {0.0};
        return *this;
    }

    std::vector<double> product(
        coefficients_.size() + other.coefficients_.size() - 1, 0.0);
    for (std::size_t left = 0; left < coefficients_.size(); ++left) {
        for (std::size_t right = 0; right < other.coefficients_.size(); ++right) {
            product[left + right] += coefficients_[left] * other.coefficients_[right];
        }
    }
    coefficients_ = std::move(product);
    normalize();
    return *this;
}

Polynomial& Polynomial::operator/=(const Polynomial& other) {
    *this = *this / other;
    return *this;
}

Polynomial operator+(Polynomial left, const Polynomial& right) {
    left += right;
    return left;
}

Polynomial operator-(Polynomial left, const Polynomial& right) {
    left -= right;
    return left;
}

Polynomial operator*(const Polynomial& left, const Polynomial& right) {
    Polynomial product = left;
    product *= right;
    return product;
}

Polynomial operator/(const Polynomial& dividend, const Polynomial& divisor) {
    return dividend.divideWithRemainder(divisor).first;
}

bool operator==(const Polynomial& left, const Polynomial& right) {
    if (left.coefficients_.size() != right.coefficients_.size()) {
        return false;
    }
    for (std::size_t index = 0; index < left.coefficients_.size(); ++index) {
        if (!isZero(left.coefficients_[index] - right.coefficients_[index])) {
            return false;
        }
    }
    return true;
}

bool operator!=(const Polynomial& left, const Polynomial& right) {
    return !(left == right);
}

std::ostream& operator<<(std::ostream& stream, const Polynomial& polynomial) {
    stream << '[';
    for (std::size_t index = 0; index < polynomial.coefficients_.size(); ++index) {
        if (index != 0) {
            stream << ", ";
        }
        stream << polynomial.coefficients_[index];
    }
    return stream << ']';
}

std::istream& operator>>(std::istream& stream, Polynomial& polynomial) {
    std::string line;
    if (!std::getline(stream >> std::ws, line)) {
        return stream;
    }

    if (!normalizeCoefficientText(line)) {
        stream.setstate(std::ios::failbit);
        return stream;
    }
    try {
        polynomial = Polynomial(parseCoefficientList(line));
    } catch (const std::invalid_argument&) {
        stream.setstate(std::ios::failbit);
    }
    return stream;
}

void Polynomial::normalize() {
    while (coefficients_.size() > 1 && isZero(coefficients_.back())) {
        coefficients_.pop_back();
    }
    std::replace_if(coefficients_.begin(), coefficients_.end(), isZero, 0.0);
}

