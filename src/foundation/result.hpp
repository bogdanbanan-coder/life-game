#pragma once

#include <type_traits>
#include <utility>
#include <variant>

namespace lifeGame::foundation {

template <typename T, typename Error>
class Result {
    static_assert(!std::is_reference_v<T>, "Result values cannot be references");
    static_assert(!std::is_same_v<T, Error>, "Result value and error types must differ");

public:
    [[nodiscard]] static Result success(T value)
    {
        return Result(std::in_place_index<0>, std::move(value));
    }

    [[nodiscard]] static Result failure(Error error)
    {
        return Result(std::in_place_index<1>, std::move(error));
    }

    [[nodiscard]] bool hasValue() const noexcept
    {
        return value_.index() == 0;
    }

    [[nodiscard]] bool hasError() const noexcept
    {
        return !hasValue();
    }

    [[nodiscard]] explicit operator bool() const noexcept
    {
        return hasValue();
    }

    [[nodiscard]] T& value() &
    {
        return std::get<0>(value_);
    }

    [[nodiscard]] const T& value() const&
    {
        return std::get<0>(value_);
    }

    [[nodiscard]] T&& value() &&
    {
        return std::get<0>(std::move(value_));
    }

    [[nodiscard]] const Error& error() const&
    {
        return std::get<1>(value_);
    }

    [[nodiscard]] Error& error() &
    {
        return std::get<1>(value_);
    }

    [[nodiscard]] Error&& error() &&
    {
        return std::get<1>(std::move(value_));
    }

private:
    template <typename... Arguments>
    explicit Result(std::in_place_index_t<0>, Arguments&&... arguments)
        : value_(std::in_place_index<0>, std::forward<Arguments>(arguments)...)
    {
    }

    template <typename... Arguments>
    explicit Result(std::in_place_index_t<1>, Arguments&&... arguments)
        : value_(std::in_place_index<1>, std::forward<Arguments>(arguments)...)
    {
    }

    std::variant<T, Error> value_;
};

template <typename Error>
class Result<void, Error> {
public:
    [[nodiscard]] static Result success() noexcept
    {
        return Result(std::in_place_index<0>);
    }

    [[nodiscard]] static Result failure(Error error)
    {
        return Result(std::in_place_index<1>, std::move(error));
    }

    [[nodiscard]] bool hasValue() const noexcept
    {
        return value_.index() == 0;
    }

    [[nodiscard]] bool hasError() const noexcept
    {
        return !hasValue();
    }

    [[nodiscard]] explicit operator bool() const noexcept
    {
        return hasValue();
    }

    void value() const
    {
        static_cast<void>(std::get<0>(value_));
    }

    [[nodiscard]] const Error& error() const&
    {
        return std::get<1>(value_);
    }

    [[nodiscard]] Error& error() &
    {
        return std::get<1>(value_);
    }

    [[nodiscard]] Error&& error() &&
    {
        return std::get<1>(std::move(value_));
    }

private:
    explicit Result(std::in_place_index_t<0>)
        : value_(std::in_place_index<0>)
    {
    }

    template <typename... Arguments>
    explicit Result(std::in_place_index_t<1>, Arguments&&... arguments)
        : value_(std::in_place_index<1>, std::forward<Arguments>(arguments)...)
    {
    }

    std::variant<std::monostate, Error> value_;
};

} // namespace lifeGame::foundation
