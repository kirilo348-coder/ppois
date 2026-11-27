#pragma once

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

class Set;

/**
 * @brief Атомарная строка или вложенное математическое множество.
 */
class SetElement {
public:
    /** @brief Создаёт элемент из непустого атома. */
    explicit SetElement(std::string atom);

    /** @brief Создаёт элемент, содержащий копию другого множества. */
    explicit SetElement(const Set& nestedSet);

    /** @brief Проверяет, содержит ли элемент вложенное множество. */
    [[nodiscard]] bool isSet() const noexcept;

    /** @brief Возвращает атом; для вложенного множества выбрасывает исключение. */
    [[nodiscard]] const std::string& atom() const;

    /** @brief Возвращает вложенное множество; для атома выбрасывает исключение. */
    [[nodiscard]] const Set& nestedSet() const;

    /** @brief Возвращает строковое представление элемента. */
    [[nodiscard]] std::string toString() const;

    /** @brief Сравнивает атомы по значению, а множества — по равенству. */
    friend bool operator==(const SetElement& left, const SetElement& right);

    /** @brief Возвращает результат неравенства элементов. */
    friend bool operator!=(const SetElement& left, const SetElement& right);

private:
    std::variant<std::string, std::shared_ptr<const Set>> value_;
};

/**
 * @brief Неупорядоченное множество уникальных атомов и/или вложенных множеств.
 *
 * Текстовая запись использует фигурные скобки и запятые, например
 * "{a, b, {c, d}, {}}". Атомы могут содержать пробелы, но не фигурные скобки
 * и запятые.
 */
class Set {
public:
    using Elements = std::vector<SetElement>;

    Set() = default;

    /** @brief Проверяет, пусто ли множество. */
    [[nodiscard]] bool empty() const noexcept;

    /** @brief Возвращает мощность множества. */
    [[nodiscard]] std::size_t cardinality() const noexcept;

    /** @brief Предоставляет доступ к элементам только для чтения. */
    [[nodiscard]] const Elements& elements() const noexcept;

    /** @brief Проверяет принадлежность атома или вложенного множества. */
    [[nodiscard]] bool contains(const SetElement& element) const;

    /** @brief Проверяет принадлежность атома. */
    [[nodiscard]] bool contains(const std::string& atom) const;

    /** @brief Проверяет принадлежность вложенного множества. */
    [[nodiscard]] bool contains(const Set& nestedSet) const;

    /** @brief Проверяет принадлежность элемента с помощью оператора []. */
    [[nodiscard]] bool operator[](const SetElement& element) const;

    /** @brief Проверяет принадлежность атома с помощью оператора []. */
    [[nodiscard]] bool operator[](const std::string& atom) const;

    /** @brief Проверяет принадлежность вложенного множества с помощью []. */
    [[nodiscard]] bool operator[](const Set& nestedSet) const;

    /** @brief Добавляет элемент, если равного элемента ещё нет. */
    void add(const SetElement& element);

    /** @brief Добавляет атом, если его ещё нет. */
    void add(const std::string& atom);

    /** @brief Добавляет вложенное множество, если его ещё нет. */
    void add(const Set& nestedSet);

    /** @brief Удаляет элемент и сообщает, был ли он в множестве. */
    bool remove(const SetElement& element);

    /** @brief Удаляет атом и сообщает, был ли он в множестве. */
    bool remove(const std::string& atom);

    /** @brief Удаляет вложенное множество и сообщает, было ли оно в множестве. */
    bool remove(const Set& nestedSet);

    /** @brief Строит множество всех подмножеств (его размер растёт экспоненциально). */
    [[nodiscard]] Set powerSet() const;

    /** @brief Преобразует множество в строку с детерминированным порядком элементов. */
    [[nodiscard]] std::string toString() const;

    /** @brief Разбирает запись множества, в том числе рекурсивно вложенные множества. */
    [[nodiscard]] static Set fromString(std::string_view text);

    /** @brief Заменяет множество его объединением с другим множеством. */
    Set& operator+=(const Set& other);

    /** @brief Заменяет множество его пересечением с другим множеством. */
    Set& operator*=(const Set& other);

    /** @brief Удаляет элементы, входящие в другое множество. */
    Set& operator-=(const Set& other);

    /** @brief Возвращает объединение двух множеств. */
    friend Set operator+(Set left, const Set& right);

    /** @brief Возвращает пересечение двух множеств. */
    friend Set operator*(Set left, const Set& right);

    /** @brief Возвращает разность двух множеств. */
    friend Set operator-(Set left, const Set& right);

    /** @brief Сравнивает множества независимо от порядка добавления элементов. */
    friend bool operator==(const Set& left, const Set& right);

    /** @brief Возвращает результат неравенства множеств. */
    friend bool operator!=(const Set& left, const Set& right);

    /** @brief Записывает множество в текстовом формате с фигурными скобками. */
    friend std::ostream& operator<<(std::ostream& stream, const Set& set);

    /** @brief Считывает одну строку с текстовой записью множества. */
    friend std::istream& operator>>(std::istream& stream, Set& set);

private:
    Elements elements_;
};
