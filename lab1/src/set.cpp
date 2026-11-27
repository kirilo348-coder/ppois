#include "set.hpp"

#include <algorithm>
#include <cctype>
#include <istream>
#include <limits>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace {

class SetParser {
public:
    explicit SetParser(std::string_view text) : text_(text) {}

    Set parse() {
        skipWhitespace();
        Set result = parseSet();
        skipWhitespace();
        if (position_ != text_.size()) {
            fail("Лишний текст после множества");
        }
        return result;
    }

private:
    std::string_view text_;
    std::size_t position_ = 0;

    [[noreturn]] void fail(const std::string& message) const {
        throw std::invalid_argument(message + " (позиция " +
                                    std::to_string(position_) + ")");
    }

    void skipWhitespace() {
        while (position_ < text_.size() &&
               std::isspace(static_cast<unsigned char>(text_[position_]))) {
            ++position_;
        }
    }

    Set parseSet() {
        if (position_ == text_.size() || text_[position_] != '{') {
            fail("Ожидалась открывающая фигурная скобка '{'");
        }
        ++position_;
        skipWhitespace();

        Set result;
        if (take('}')) {
            return result;
        }
        parseSetElements(result);
        return result;
    }

    void parseSetElements(Set& result) {
        while (true) {
            result.add(parseElement());
            skipWhitespace();
            if (take('}')) {
                return;
            }
            if (!take(',')) {
                fail("Ожидалась запятая ',' или закрывающая скобка '}'");
            }
            skipWhitespace();
        }
    }

    SetElement parseElement() {
        if (position_ < text_.size() && text_[position_] == '{') {
            return SetElement(parseSet());
        }

        const std::size_t start = position_;
        while (position_ < text_.size() && text_[position_] != ',' &&
               text_[position_] != '}' && text_[position_] != '{') {
            ++position_;
        }

        std::size_t end = position_;
        while (end > start &&
               std::isspace(static_cast<unsigned char>(text_[end - 1]))) {
            --end;
        }
        if (end == start) {
            fail("Ожидался атом или вложенное множество");
        }
        return SetElement(std::string(text_.substr(start, end - start)));
    }

    bool take(char expected) {
        if (position_ < text_.size() && text_[position_] == expected) {
            ++position_;
            return true;
        }
        return false;
    }
};

void appendExpandedSubsets(Set::Elements& subsets, const SetElement& element) {
    const std::size_t existingCount = subsets.size();
    if (existingCount > subsets.max_size() - existingCount) {
        throw std::length_error("Булеан слишком велик для представления");
    }
    subsets.reserve(existingCount * 2);
    for (std::size_t index = 0; index < existingCount; ++index) {
        Set expanded = subsets[index].nestedSet();
        expanded.add(element);
        subsets.emplace_back(expanded);
    }
}

} // namespace

SetElement::SetElement(std::string atom) : value_(std::move(atom)) {
    std::string& text = std::get<std::string>(value_);
    const auto first = std::find_if_not(
        text.begin(), text.end(),
        [](unsigned char character) { return std::isspace(character); });
    const auto last = std::find_if_not(
        text.rbegin(), text.rend(),
        [](unsigned char character) { return std::isspace(character); }).base();
    if (first >= last) {
        throw std::invalid_argument("Элемент множества не может быть пустым");
    }
    text = std::string(first, last);
    if (text.find_first_of("{},") != std::string::npos) {
        throw std::invalid_argument(
            "Элемент множества не может содержать символы '{', '}' или ','");
    }
}

SetElement::SetElement(const Set& nestedSet)
    : value_(std::make_shared<const Set>(nestedSet)) {}

bool SetElement::isSet() const noexcept {
    return std::holds_alternative<std::shared_ptr<const Set>>(value_);
}

const std::string& SetElement::atom() const {
    if (isSet()) {
        throw std::logic_error("У вложенного множества нет атомарного значения");
    }
    return std::get<std::string>(value_);
}

const Set& SetElement::nestedSet() const {
    if (!isSet()) {
        throw std::logic_error("Атом не является вложенным множеством");
    }
    return *std::get<std::shared_ptr<const Set>>(value_);
}

std::string SetElement::toString() const {
    return isSet() ? nestedSet().toString() : atom();
}

bool operator==(const SetElement& left, const SetElement& right) {
    if (left.isSet() != right.isSet()) {
        return false;
    }
    return left.isSet() ? left.nestedSet() == right.nestedSet()
                        : left.atom() == right.atom();
}

bool operator!=(const SetElement& left, const SetElement& right) {
    return !(left == right);
}

bool Set::empty() const noexcept {
    return elements_.empty();
}

std::size_t Set::cardinality() const noexcept {
    return elements_.size();
}

const Set::Elements& Set::elements() const noexcept {
    return elements_;
}

bool Set::contains(const SetElement& element) const {
    return std::find(elements_.begin(), elements_.end(), element) != elements_.end();
}

bool Set::contains(const std::string& atom) const {
    return contains(SetElement(atom));
}

bool Set::contains(const Set& nestedSet) const {
    return contains(SetElement(nestedSet));
}

bool Set::operator[](const SetElement& element) const {
    return contains(element);
}

bool Set::operator[](const std::string& atom) const {
    return contains(atom);
}

bool Set::operator[](const Set& nestedSet) const {
    return contains(nestedSet);
}

void Set::add(const SetElement& element) {
    if (!contains(element)) {
        elements_.push_back(element);
    }
}

void Set::add(const std::string& atom) {
    add(SetElement(atom));
}

void Set::add(const Set& nestedSet) {
    add(SetElement(nestedSet));
}

bool Set::remove(const SetElement& element) {
    const auto found = std::find(elements_.begin(), elements_.end(), element);
    if (found == elements_.end()) {
        return false;
    }
    elements_.erase(found);
    return true;
}

bool Set::remove(const std::string& atom) {
    return remove(SetElement(atom));
}

bool Set::remove(const Set& nestedSet) {
    return remove(SetElement(nestedSet));
}

Set Set::powerSet() const {
    if (elements_.size() >= std::numeric_limits<std::size_t>::digits) {
        throw std::length_error("Булеан слишком велик для представления");
    }

    Elements subsets;
    subsets.emplace_back(Set{});
    for (const SetElement& element : elements_) {
        appendExpandedSubsets(subsets, element);
    }

    Set result;
    for (const SetElement& subset : subsets) {
        result.add(subset.nestedSet());
    }
    return result;
}

std::string Set::toString() const {
    std::vector<std::string> values;
    values.reserve(elements_.size());
    for (const SetElement& element : elements_) {
        values.push_back(element.toString());
    }
    std::sort(values.begin(), values.end());

    std::ostringstream output;
    output << '{';
    for (std::size_t index = 0; index < values.size(); ++index) {
        if (index != 0) {
            output << ", ";
        }
        output << values[index];
    }
    output << '}';
    return output.str();
}

Set Set::fromString(std::string_view text) {
    return SetParser(text).parse();
}

Set& Set::operator+=(const Set& other) {
    for (const SetElement& element : other.elements_) {
        add(element);
    }
    return *this;
}

Set& Set::operator*=(const Set& other) {
    Elements intersection;
    for (const SetElement& element : elements_) {
        if (other.contains(element)) {
            intersection.push_back(element);
        }
    }
    elements_ = std::move(intersection);
    return *this;
}

Set& Set::operator-=(const Set& other) {
    elements_.erase(
        std::remove_if(elements_.begin(), elements_.end(),
                       [&other](const SetElement& element) {
                           return other.contains(element);
                       }),
        elements_.end());
    return *this;
}

Set operator+(Set left, const Set& right) {
    left += right;
    return left;
}

Set operator*(Set left, const Set& right) {
    left *= right;
    return left;
}

Set operator-(Set left, const Set& right) {
    left -= right;
    return left;
}

bool operator==(const Set& left, const Set& right) {
    if (left.cardinality() != right.cardinality()) {
        return false;
    }
    return std::all_of(left.elements_.begin(), left.elements_.end(),
                       [&right](const SetElement& element) {
                           return right.contains(element);
                       });
}

bool operator!=(const Set& left, const Set& right) {
    return !(left == right);
}

std::ostream& operator<<(std::ostream& stream, const Set& set) {
    return stream << set.toString();
}

std::istream& operator>>(std::istream& stream, Set& set) {
    std::string line;
    if (!std::getline(stream >> std::ws, line)) {
        return stream;
    }
    try {
        set = Set::fromString(line);
    } catch (const std::invalid_argument&) {
        stream.setstate(std::ios::failbit);
    }
    return stream;
}
