#pragma once
#include <functional>
#include <typeinfo>
#include <type_traits>

#if (_MSC_VER && !__INTEL_COMPILER)
#include <vcruntime.h> // _HAS_CXX17
#define __builtin_unreachable() __assume(0)
#endif // https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros?view=vs-2019

#if !(defined(__cplusplus) && __cplusplus >= 201703L) || (defined(_HAS_CXX17) && _HAS_CXX17 == 1)
namespace std17 {
    class bad_any_cast : public std::bad_cast {
    public:
        bad_any_cast() noexcept = default;
        const char* what() const noexcept override { return "Unexpected type."; }
    };

    class any {
        template<class T>
        friend T any_cast(const any& operand);

        template<class T>
        friend T any_cast(any& operand);

        template<class T>
        friend T any_cast(any&& operand);

        template<typename T>
        friend const T* any_cast(const any* operand) noexcept;

        template<typename T>
        friend T* any_cast(any* operand) noexcept;

    public:
        any() : obj(nullptr), type(&typeid(void)) {}

        any(const any& other) : obj(nullptr), type(other.type), modifiers(other.modifiers) {
            modifiers.copy_constructor(obj, other.obj);
        }

        any(any&& other) noexcept : obj(nullptr), type(&typeid(void)) {
            std::swap(obj, other.obj);
            std::swap(type, other.type);
            std::swap(modifiers, other.modifiers);
        }

        template<
                typename T,
                typename = typename std::enable_if<
                        !std::is_same<
                                any,
                                typename std::remove_cv<
                                        typename std::remove_reference<T>::type
                                >::type
                        >::value
                >::type
        >
        explicit any(T&& t) : obj(nullptr), type(&typeid(void)) {
            using U = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
            emplace<U>(std::forward<T>(t));
        }

        ~any() {
            reset();
        }

        any& operator=(const any& rhs) {
            if (this != &rhs) {
                if (type == rhs.type)
                    modifiers.copy_assigner(obj, rhs.obj);
                else {
                    reset();
                    modifiers.copy_constructor(obj, rhs.obj);
                }
                type = rhs.type;
                modifiers = rhs.modifiers;
            }
            return *this;
        }

        any& operator=(any&& rhs) noexcept {
            if (this != &rhs) {
                std::swap(obj, rhs.obj);
                std::swap(type, rhs.type);
                std::swap(modifiers, rhs.modifiers);
            }
            return *this;
        }

        template<
                typename T,
                typename = typename std::enable_if<
                        !std::is_same<
                                any,
                                typename std::remove_cv<
                                        typename std::remove_reference<T>::type
                                >::type
                        >::value
                >::type
        >
        any& operator=(T&& t) {
            using U = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
            emplace<U>(std::forward<T>(t));
            return *this;
        }

        void reset() {
            if (obj) {
                modifiers.deleter(obj);
                obj = nullptr;
                type = &typeid(void);
                modifiers.reset();
            }
        }

        template<class T, class ...Args>
        void emplace(Args&&... args) {
            if (type == &typeid(T))
                *static_cast<T*>(obj) = T(std::forward<Args>(args)...);
            else {
                reset();
                obj = new T(std::forward<Args>(args)...);
                type = &typeid(T);
                modifiers.set_modifiers<T>();
            }
        }

    private:
        struct obj_modifiers {
            obj_modifiers() {
                reset();
            }

            template<typename T>
            void set_modifiers() {
                deleter = [](void* ptr){ delete static_cast<T*>(ptr); };
                copy_constructor = [](void*& ptr, void* other){ ptr = new T(*static_cast<T*>(other)); };
                move_constructor = [](void*& ptr, void* other){ ptr = new T(std::move(*static_cast<T*>(other))); };
                copy_assigner = [](void*& ptr, void* other){ *static_cast<T*>(ptr) = *static_cast<T*>(other); };
                move_assigner = [](void*& ptr, void* other){ *static_cast<T*>(ptr) = std::move(*static_cast<T*>(other)); };
            }

            void reset() {
                deleter = [](void* ptr){};
                copy_constructor = [](void*& ptr, void* other){};
                move_constructor = [](void*& ptr, void* other){};
                copy_assigner = [](void*& ptr, void* other){};
                move_assigner = [](void*& ptr, void* other){};
            }

            std::function<void(void*)> deleter;
            std::function<void(void*&, void*)> copy_constructor;
            std::function<void(void*&, void*)> move_constructor;
            std::function<void(void*&, void*)> copy_assigner;
            std::function<void(void*&, void*)> move_assigner;
        };

        void* obj;
        const std::type_info* type;
        obj_modifiers modifiers;
    };

    template<class T>
    T any_cast(const any& operand) {
        using U = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
        if (&typeid(U) == operand.type)
            return *static_cast<T*>(operand.obj);
        else
            throw bad_any_cast();
    }

    template<class T>
    T any_cast(any& operand) {
        using U = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
        if (&typeid(U) == operand.type)
            return *static_cast<T*>(operand.obj);
        else
            throw bad_any_cast();
    }

    template<class T>
    T any_cast(any&& operand) {
        using U = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
        if (&typeid(U) == operand.type)
            return std::move(*static_cast<T*>(operand.obj));
        else
            throw bad_any_cast();
    }

    template<typename T>
    const T* any_cast(const any* operand) noexcept {
        if (&typeid(T) == operand->type)
            return static_cast<T*>(operand->obj);
        else
            return nullptr;
    }

    template<typename T>
    T* any_cast(any* operand) noexcept {
        if (&typeid(T) == operand->type)
            return static_cast<T*>(operand->obj);
        else
            return nullptr;
    }
}

#else
#warning "Use of deprecated header "any.hpp", importing system header <any> instead."
#include <any>
#define std17 std
#endif // https://docs.microsoft.com/en-us/cpp/build/reference/zc-cplusplus?view=vs-2019
