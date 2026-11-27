#pragma once

#include <cstddef>
#include <iosfwd>
#include <string>
#include <utility>
#include <vector>

/**
 * @brief Многочлен с вещественными коэффициентами, хранящимися от свободного члена.
 *
 * Например, коэффициенты {1, 2, 3} задают многочлен 1 + 2x + 3x^2.
 */
class Polynomial {
public:
    /** @brief Создаёт нулевой многочлен. */
    Polynomial();

    /**
     * @brief Создаёт многочлен; отсутствующие коэффициенты считаются нулевыми.
     * @param coefficients Коэффициенты в порядке возрастания степеней.
     */
    explicit Polynomial(std::vector<double> coefficients);

    /** @brief Возвращает старшую степень; для нулевого многочлена — ноль. */
    [[nodiscard]] int degree() const noexcept;

    /**
     * @brief Предоставляет доступ к нормализованным коэффициентам
     * только для чтения.
     */
    [[nodiscard]] const std::vector<double>& coefficients() const noexcept;

    /** @brief Вычисляет значение многочлена по схеме Горнера. */
    [[nodiscard]] double evaluate(double argument) const;

    /** @brief Вычисляет значение многочлена при вызове объекта как функции. */
    [[nodiscard]] double operator()(double argument) const;

    /**
     * @brief Делит многочлен и возвращает частное и остаток.
     * @throws std::domain_error Если делитель — нулевой многочлен.
     */
    [[nodiscard]] std::pair<Polynomial, Polynomial>
    divideWithRemainder(const Polynomial& divisor) const;

    /**
     * @brief Возвращает алгебраическую запись многочлена.
     */
    [[nodiscard]] std::string toString() const;

    /** @brief Возвращает коэффициент; для степеней выше старшей возвращает ноль. */
    [[nodiscard]] double operator[](std::size_t exponent) const noexcept;

    /** @brief Прибавляет другой многочлен. */
    Polynomial& operator+=(const Polynomial& other);

    /** @brief Вычитает другой многочлен. */
    Polynomial& operator-=(const Polynomial& other);

    /** @brief Умножает на другой многочлен. */
    Polynomial& operator*=(const Polynomial& other);

    /** @brief Заменяет многочлен частным от деления на другой многочлен. */
    Polynomial& operator/=(const Polynomial& other);

    /** @brief Возвращает сумму двух многочленов. */
    friend Polynomial operator+(Polynomial left, const Polynomial& right);

    /** @brief Возвращает разность двух многочленов. */
    friend Polynomial operator-(Polynomial left, const Polynomial& right);

    /** @brief Возвращает произведение двух многочленов. */
    friend Polynomial operator*(const Polynomial& left, const Polynomial& right);

    /** @brief Возвращает частное; ненулевой остаток отбрасывается. */
    friend Polynomial operator/(const Polynomial& dividend, const Polynomial& divisor);

    /**
     * @brief Сравнивает коэффициенты с малой погрешностью вычислений
     * с плавающей точкой.
     */
    friend bool operator==(const Polynomial& left, const Polynomial& right);

    /** @brief Возвращает результат неравенства многочленов. */
    friend bool operator!=(const Polynomial& left, const Polynomial& right);

    /** @brief Записывает коэффициенты в порядке возрастания степеней. */
    friend std::ostream& operator<<(std::ostream& stream, const Polynomial& polynomial);

    /** @brief Считывает одну строку коэффициентов в квадратных скобках. */
    friend std::istream& operator>>(std::istream& stream, Polynomial& polynomial);

private:
    std::vector<double> coefficients_;
    void normalize();
};
