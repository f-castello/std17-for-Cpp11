#pragma once
#include <array>
#include <cstdint>
#include <exception>
#include <type_traits>
#include <typeinfo>

#if __cplusplus < 201703L
namespace std17 {
    class bad_variant_access : public std::exception {
    public:
        bad_variant_access() noexcept = default;
        const char* what() const noexcept override { return "Unexpected type."; }
    };

    template<class Type, class ...Types>
    class variant {
    public:
        variant() : type(&typeid(void)) { static_assert(!contains<void, Type, Types...>::value, "cannot store void type in variant"); }

        variant(const variant& other) : type(other.type) {
            if (other.type != &typeid(void)) {
                bool result = CheckMatchingType<CopyConstructObject, Type, Types...>()(other.type, &data[0], const_cast<uint8_t*>(&other.data[0]));
                if (!result) __builtin_unreachable();
            }
        }

        variant(variant&& other) noexcept : type(&typeid(void)) {
            if (other.type != &typeid(void)) {
                data = std::move(other.data);
                std::swap(type, other.type);
            }
        }

        ~variant() { DestroyObject(); }

        template<class T, class ...Args>
        void emplace(Args&&... args) {
            static_assert(contains<T, Type, Types...>::value, "not a valid alternative type");
            DestroyObject();
            new (&data[0]) T(std::forward<Args>(args)...);
            type = &typeid(T);
        }

        template<class T>
        T* get_if() {
            static_assert(contains<T, Type, Types...>::value, "not a valid alternative type");
            if (type == &typeid(T))
                return reinterpret_cast<T*>(&data[0]);
            else return nullptr;
        }

        template<class T>
        T& get() {
            static_assert(contains<T, Type, Types...>::value, "not a valid alternative type");
            if (type == &typeid(T))
                return *reinterpret_cast<T*>(&data[0]);
            else throw bad_variant_access();
        }

        template<class T>
        static T& get(variant& other) {
            static_assert(contains<T, Type, Types...>::value, "not a valid alternative type");
            if (other.type == &typeid(T))
                return *reinterpret_cast<T*>(&other.data[0]);
            else throw bad_variant_access();
        }

        template<typename T, class F, class ...Ts>
        static T& gets(variant<F, Ts...>& rhs) {
            static_assert(contains<T, F, Ts...>::value, "not a valid alternative type");
            if (rhs.type == &typeid(T))
                return *reinterpret_cast<T*>(&rhs.data[0]);
            else throw bad_variant_access();
        }

        template<typename T>
        variant& operator=(T& other) {
            static_assert(contains<T, Type, Types...>::value, "not a valid alternative type");
            if (&typeid(T) != &typeid(void)) {
                DestroyObject();
                new (&data[0]) T(other);
                type = &typeid(T);
            }
            return *this;
        }

        template<typename T>
        variant& operator=(T&& other) {
            static_assert(contains<T, Type, Types...>::value, "not a valid alternative type");
            if (&typeid(T) != &typeid(void)) {
                DestroyObject();
                new (&data[0]) T(std::forward<T>(other));
                type = &typeid(T);
            }
            return *this;
        }

    private:
        void DestroyObject() {
            if (type != &typeid(void)) {
                bool result = CheckMatchingType<CallDestructor, Type, Types...>()(type, &data[0]);
                if (!result) __builtin_unreachable(); // https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html
                type = &typeid(void);
            }
        }

        template<typename T>
        struct CallDestructor {
            void operator()(T* const ptr, T* const other) const {
                if (std::is_destructible<T>::value && !std::is_trivially_destructible<T>::value)
                    ptr->~T();
            }
        };

        template<typename T>
        struct CopyConstructObject {
            void operator()(T* const ptr, T* const other) const {
                new (ptr) T(*other);
            }
        };

        template<template <typename> class F, typename ...Ts>
        struct CheckMatchingType {
            bool operator()(const std::type_info* const tp, const uint8_t* const ptr, const uint8_t* const other = nullptr) const {
                return false;
            }
        };

        template<template <typename> class F, typename T, typename ...Ts>
        struct CheckMatchingType<F, T, Ts...> {
            bool operator()(const std::type_info* const tp, uint8_t* const ptr, uint8_t* const other = nullptr) const {
                if (tp == &typeid(T)) {
                    F<T>()(reinterpret_cast<T*>(ptr), reinterpret_cast<T*>(other));
                    return true;
                } else return CheckMatchingType<F, Ts...>()(tp, ptr, other);
            }
        };

        template<typename T>
        static constexpr T constexpr_max(const T& val) {
            return val;
        }

        template<typename T0, typename T1, typename ...Ts>
        static constexpr typename std::common_type<T0, T1, Ts...>::type constexpr_max(T0 val, T1 val2, Ts... vs) {
            return val > val2 ? constexpr_max(val, vs...) : constexpr_max(val2, vs...);
        }

        template<class ...Tail>
        struct MaxTypeSize {
            static const std::size_t value = 0;
        };

        template<class T, class ...Tail>
        struct MaxTypeSize<T, Tail...> {
            static const std::size_t value = constexpr_max(sizeof(T), MaxTypeSize<Tail...>::value);
        };

        template<class T, class ...Tail>
        struct contains {
            static const bool value = false;
        };

        template<class T, class Head, class ...Tail>
        struct contains<T, Head, Tail...> {
            static const bool value = std::is_same<T, Head>::value || contains<T, Tail...>::value;
        };

        std::array<uint8_t, MaxTypeSize<Type, Types...>::value> data;
        const std::type_info* type;
    };
}
#else
#warning "Use of deprecated header "variant.hpp", importing system header <variant> instead."
#include <variant>
#define std17 std
#endif // https://docs.microsoft.com/en-us/cpp/build/reference/zc-cplusplus?view=vs-2019
