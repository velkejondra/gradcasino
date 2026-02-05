#pragma once

#include <optional>
#include <variant>
#include <string>
#include <type_traits>
#include <utility>

namespace gc {

struct Error {
    std::string message;
    
    [[nodiscard]] bool operator==(const Error& other) const {
        return message == other.message;
    }
};

template<typename T>
class Result {
public:
    Result(T value) : data_{std::move(value)} {}
    Result(Error error) : data_{std::move(error)} {}
    
    [[nodiscard]] bool has_value() const noexcept {
        return std::holds_alternative<T>(data_);
    }
    
    [[nodiscard]] explicit operator bool() const noexcept {
        return has_value();
    }
    
    [[nodiscard]] T& value() & { return std::get<T>(data_); }
    [[nodiscard]] const T& value() const& { return std::get<T>(data_); }
    [[nodiscard]] T&& value() && { return std::get<T>(std::move(data_)); }
    
    [[nodiscard]] Error& error() & { return std::get<Error>(data_); }
    [[nodiscard]] const Error& error() const& { return std::get<Error>(data_); }
    
    [[nodiscard]] T* operator->() { return &value(); }
    [[nodiscard]] const T* operator->() const { return &value(); }
    [[nodiscard]] T& operator*() & { return value(); }
    [[nodiscard]] const T& operator*() const& { return value(); }

    template<typename... Args>
    [[nodiscard]] auto operator()(Args&&... args) 
        -> decltype(std::declval<T&>()(std::forward<Args>(args)...)) {
        return value()(std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    [[nodiscard]] auto operator()(Args&&... args) const
        -> decltype(std::declval<const T&>()(std::forward<Args>(args)...)) {
        return value()(std::forward<Args>(args)...);
    }

private:
    std::variant<T, Error> data_;
};

template<>
class Result<void> {
public:
    Result() = default;
    Result(Error error) : error_{std::move(error)} {}
    
    [[nodiscard]] bool has_value() const noexcept {
        return !error_.has_value();
    }
    
    [[nodiscard]] explicit operator bool() const noexcept {
        return has_value();
    }
    
    [[nodiscard]] const Error& error() const { return *error_; }

private:
    std::optional<Error> error_;
};

} // namespace gc
