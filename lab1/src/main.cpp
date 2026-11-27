#include "polynomial.hpp"
#include "set.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

void configureConsole() {
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif
}

std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) {
        throw std::runtime_error("Поток ввода закрыт");
    }
    return line;
}

int readChoice(const std::string& prompt) {
    const std::string line = readLine(prompt);
    std::istringstream input(line);
    int choice = -1;
    char extra = '\0';
    if (!(input >> choice) || (input >> extra)) {
        std::cout << "Введите целое число из меню.\n";
        return -1;
    }
    return choice;
}

Set readSet(const std::string& prompt) {
    const std::string line = readLine(prompt);
    return Set::fromString(line);
}

SetElement readElement(const std::string& prompt) {
    const std::string line = readLine(prompt);
    if (!line.empty() && line.front() == '{') {
        return SetElement(Set::fromString(line));
    }
    return SetElement(line);
}

Polynomial readPolynomial(const std::string& prompt) {
    const std::string line = readLine(prompt);
    std::istringstream input(line);
    Polynomial polynomial;
    if (!(input >> polynomial)) {
        throw std::invalid_argument(
            "Введите коэффициенты в формате [1, 2, 3], начиная со свободного члена");
    }
    return polynomial;
}

double readDouble(const std::string& prompt) {
    const std::string line = readLine(prompt);
    std::istringstream input(line);
    double value = 0.0;
    char extra = '\0';
    if (!(input >> value) || (input >> extra)) {
        throw std::invalid_argument("Введите вещественное число");
    }
    return value;
}

void printSetMenu() {
    std::cout << "\nОперации над множеством\n"
              << "1. Показать множество\n"
              << "2. Добавить элемент (атом или вложенное множество)\n"
              << "3. Удалить элемент\n"
              << "4. Проверить принадлежность элемента\n"
              << "5. Объединить с другим множеством\n"
              << "6. Пересечь с другим множеством\n"
              << "7. Найти разность с другим множеством\n"
              << "8. Построить булеан\n"
              << "0. Назад\n";
}

void applySetOperation(Set& current, int choice) {
    const Set other = readSet("Введите другое множество: ");
    if (choice == 5) {
        current += other;
    } else if (choice == 6) {
        current *= other;
    } else {
        current -= other;
    }
    std::cout << "Результат: " << current << '\n';
}

void removeSetElement(Set& current) {
    const bool removed = current.remove(readElement("Элемент: "));
    std::cout << (removed ? "Элемент удалён.\n" : "Элемент не найден.\n");
}

void reportSetMembership(const Set& current) {
    const bool found = current.contains(readElement("Элемент: "));
    std::cout << (found ? "Элемент принадлежит множеству.\n"
                        : "Элемент не принадлежит множеству.\n");
}

void processSetChoice(Set& current, int choice) {
    switch (choice) {
    case 1:
        std::cout << current << " (мощность " << current.cardinality() << ")\n";
        break;
    case 2:
        current.add(readElement("Элемент: "));
        break;
    case 3:
        removeSetElement(current);
        break;
    case 4:
        reportSetMembership(current);
        break;
    case 5:
    case 6:
    case 7:
        applySetOperation(current, choice);
        break;
    case 8:
        std::cout << "Булеан: " << current.powerSet() << '\n';
        break;
    default:
        std::cout << "Нет такого пункта меню.\n";
    }
}

void runSetMenu() {
    Set current;
    while (true) {
        printSetMenu();
        const int choice = readChoice("Выбор: ");
        if (choice == 0) {
            return;
        }
        try {
            processSetChoice(current, choice);
        } catch (const std::invalid_argument& error) {
            std::cout << "Некорректный ввод: " << error.what() << '\n';
        } catch (const std::length_error& error) {
            std::cout << "Не удалось построить результат: " << error.what() << '\n';
        }
    }
}

void printPolynomialMenu() {
    std::cout << "\nОперации над многочленом\n"
              << "1. Задать текущий многочлен\n"
              << "2. Показать многочлен и коэффициенты\n"
              << "3. Получить коэффициент по степени\n"
              << "4. Вычислить значение при заданном x\n"
              << "5. Сложить с другим многочленом\n"
              << "6. Вычесть другой многочлен\n"
              << "7. Умножить на другой многочлен\n"
              << "8. Разделить на другой многочлен\n"
              << "0. Назад\n";
}

void applyPolynomialOperation(Polynomial& current, int choice) {
    const Polynomial other = readPolynomial(
        "Коэффициенты другого многочлена [c0, c1, ...]: ");
    if (choice == 5) {
        current += other;
    } else if (choice == 6) {
        current -= other;
    } else if (choice == 7) {
        current *= other;
    } else {
        const auto result = current.divideWithRemainder(other);
        current = result.first;
        std::cout << "Остаток: " << result.second.toString() << '\n';
    }
    std::cout << "Результат: " << current.toString() << '\n';
}

void printPolynomial(const Polynomial& current) {
    std::cout << "P(x) = " << current.toString()
              << "\nКоэффициенты: " << current << '\n';
}

void printPolynomialCoefficient(const Polynomial& current) {
    const int exponent = readChoice("Степень: ");
    if (exponent < 0) {
        throw std::invalid_argument("Степень не может быть отрицательной");
    }
    std::cout << current[static_cast<std::size_t>(exponent)] << '\n';
}

void printPolynomialValue(const Polynomial& current) {
    const double argument = readDouble("Введите x: ");
    std::cout << "P(" << argument << ") = " << current(argument) << '\n';
}

void processPolynomialChoice(Polynomial& current, int choice) {
    switch (choice) {
    case 1:
        current = readPolynomial("Коэффициенты [c0, c1, ...]: ");
        break;
    case 2:
        printPolynomial(current);
        break;
    case 3:
        printPolynomialCoefficient(current);
        break;
    case 4:
        printPolynomialValue(current);
        break;
    case 5:
    case 6:
    case 7:
    case 8:
        applyPolynomialOperation(current, choice);
        break;
    default:
        std::cout << "Нет такого пункта меню.\n";
    }
}

void runPolynomialMenu() {
    Polynomial current;
    while (true) {
        printPolynomialMenu();
        const int choice = readChoice("Выбор: ");
        if (choice == 0) {
            return;
        }
        try {
            processPolynomialChoice(current, choice);
        } catch (const std::invalid_argument& error) {
            std::cout << "Некорректный ввод: " << error.what() << '\n';
        } catch (const std::domain_error& error) {
            std::cout << "Операция не выполнена: " << error.what() << '\n';
        }
    }
}

void runMainMenu() {
    while (true) {
        std::cout << "\nЛабораторная работа №1\n"
                  << "1. Множества\n"
                  << "2. Многочлены\n"
                  << "0. Выход\n";
        const int choice = readChoice("Выбор: ");
        if (choice == 0) {
            return;
        }
        if (choice == 1) {
            runSetMenu();
        } else if (choice == 2) {
            runPolynomialMenu();
        } else {
            std::cout << "Нет такого пункта меню.\n";
        }
    }
}

} // namespace

int main() {
    configureConsole();
    try {
        runMainMenu();
    } catch (const std::runtime_error& error) {
        std::cerr << "Ошибка ввода: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
