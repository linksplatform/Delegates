#pragma once

// Based on https://stackoverflow.com/a/23974539/710069 and https://stackoverflow.com/a/35920804/710069

#include <memory>
#include <iostream>
#include <functional>
#include <tuple>
#include <concepts>

namespace Platform::Delegates
{
    // Type deduction helper for member function pointers
    template <typename M>
    struct MemberFunctionTraits;

    template <typename TClass, typename TReturn, typename... TArgs>
    struct MemberFunctionTraits<TReturn (TClass::*)(TArgs...)>
    {
        using class_type = TClass;
        using return_type = TReturn;
        using argument_types = std::tuple<TArgs...>;
        using function_signature = TReturn(TArgs...);
    };

    template <typename TClass, typename TReturn, typename... TArgs>
    struct MemberFunctionTraits<TReturn (TClass::*)(TArgs...) const>
    {
        using class_type = TClass;
        using return_type = TReturn;
        using argument_types = std::tuple<TArgs...>;
        using function_signature = TReturn(TArgs...);
    };

    template <typename...>
    class Delegate;

    template <typename ReturnType, typename... Args>
    class Delegate<ReturnType(Args...)>
    {
    public:
        using DelegateRawFunctionType = ReturnType(Args...);
        using DelegateFunctionType = std::function<DelegateRawFunctionType>;

        constexpr Delegate() noexcept = default;

        Delegate(const Delegate&) noexcept = default;

        Delegate(Delegate&&) noexcept = default;

        constexpr Delegate(DelegateRawFunctionType simpleFunction) noexcept
            : simpleFunction(simpleFunction) { }

        Delegate(const DelegateFunctionType &complexFunction)
            : complexFunction(std::make_shared<DelegateFunctionType>(complexFunction)) { }

        Delegate(std::shared_ptr<DelegateFunctionType> complexFunctionPointer)
            : complexFunction(complexFunctionPointer) { }

        template <typename Class>
        Delegate(std::shared_ptr<Class> object, ReturnType(Class:: *member)(Args...))
            : Delegate(std::make_shared<MemberMethod<Class>>(std::move(object), member)) { }

        virtual ~Delegate() = default;

        Delegate &operator=(const Delegate &other) noexcept
        {
            if (this != &other)
            {
                this->simpleFunction = other.simpleFunction;
                this->memberMethod = other.memberMethod;
                this->complexFunction = other.complexFunction;
            }
            return *this;
        }

        Delegate &operator=(Delegate&& other) noexcept {
            if (this != &other)
            {
                this->simpleFunction = std::move(other.simpleFunction);
                this->memberMethod = std::move(other.memberMethod);
                this->complexFunction = std::move(other.complexFunction);
            }
            return *this;
        }

        virtual ReturnType operator()(Args... args)
        {
            if (simpleFunction)
            {
                return simpleFunction(std::forward<decltype(args)>(args)...);
            }
            if (memberMethod)
            {
                return (*memberMethod)(std::forward<decltype(args)>(args)...);
            }
            if (complexFunction)
            {
                return (*complexFunction)(std::forward<decltype(args)>(args)...);
            }
            throw std::bad_function_call{};
        }

        virtual bool operator==(const Delegate &other) const
        {
            if (simpleFunction && other.simpleFunction)
            {
                return simpleFunction == other.simpleFunction;
            }
            if (memberMethod && other.memberMethod)
            {
                return *memberMethod == *other.memberMethod;
            }
            if (complexFunction && other.complexFunction)
            {
                return complexFunction == other.complexFunction;
            }
            return false;
        }

    private:
        class MemberMethodBase
        {
        public:
            virtual ReturnType operator()(Args... args) = 0;
            virtual bool operator== (const MemberMethodBase &other) const = 0;
            virtual ~MemberMethodBase() = default;
        };

        template <typename Class>
        class MemberMethod : public MemberMethodBase
        {
        public:
            MemberMethod(std::shared_ptr<Class> object, ReturnType(Class:: *method)(Args...)) noexcept
                    : object(std::move(object)), method(method) { }

            ReturnType operator()(Args... args) override
            {
                return (*object.*method)(args...);
            }

            bool operator==(const MemberMethodBase &other) const override
            {
                const MemberMethod *otherMethod = dynamic_cast<const MemberMethod *>(&other);
                if (!otherMethod)
                {
                    return false;
                }
                return this->object == otherMethod->object
                    && this->method == otherMethod->method;
            }

        private:
            std::shared_ptr<Class> object;
            ReturnType(Class:: *method)(Args...);
        };

        Delegate(std::shared_ptr<MemberMethodBase> memberMethod) noexcept
            : memberMethod(std::move(memberMethod)) { }

        DelegateRawFunctionType *simpleFunction = nullptr;
        std::shared_ptr<DelegateFunctionType> complexFunction = nullptr;
        std::shared_ptr<MemberMethodBase> memberMethod = nullptr;
    };

    template <typename ReturnType, typename... Args>
    Delegate(ReturnType(function)(Args...)) -> Delegate<ReturnType(Args...)>;

    template <typename ReturnType, typename... Args>
    Delegate(std::function<ReturnType(Args...)> function) -> Delegate<ReturnType(Args...)>;

    template <typename Class, typename ReturnType, typename... Args>
    Delegate(std::shared_ptr<Class> object, ReturnType(Class:: *member)(Args...)) -> Delegate<ReturnType(Args...)>;

    // Method pointer-based delegate for automatic type deduction
    template <auto method>
    class MethodDelegate
    {
        using Traits = MemberFunctionTraits<decltype(method)>;
        using TClass = typename Traits::class_type;
        using TReturn = typename Traits::return_type;
        using TArgsTuple = typename Traits::argument_types;
        
        std::shared_ptr<TClass> object;

    public:
        constexpr MethodDelegate(std::shared_ptr<TClass> obj) noexcept : object(std::move(obj)) { }

        template<typename ...TArgs>
        TReturn operator()(TArgs&&... args)
        {
            return (*object.*method)(std::forward<TArgs>(args)...);
        }

        bool operator==(const MethodDelegate &other) const
        {
            return object == other.object;
        }

        // Allow conversion to the traditional Delegate type
        operator Delegate<typename Traits::function_signature>() const
        {
            return Delegate<typename Traits::function_signature>(object, method);
        }
    };
}
