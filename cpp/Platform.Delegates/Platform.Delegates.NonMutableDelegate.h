#pragma once

#include "Platform.Delegates.Delegate.h"
#include <memory>
#include <functional>
#include <optional>

namespace Platform::Delegates
{
    template <typename...>
    class NonMutableDelegate;

    template <typename ReturnType, typename... Args>
    class NonMutableDelegate<ReturnType(Args...)>
    {
    public:
        using DelegateRawFunctionType = ReturnType(Args...);

        constexpr NonMutableDelegate() noexcept = default;

        NonMutableDelegate(const NonMutableDelegate&) noexcept = default;

        NonMutableDelegate(NonMutableDelegate&&) noexcept = default;

        // Case #1: Create from standard delegate
        NonMutableDelegate(const Delegate<ReturnType(Args...)>& delegate)
            : sourceDelegate(delegate) { }

        // Case #2: Create from ref to object and ptr to method (shared_ptr)
        template <typename Class>
        NonMutableDelegate(const std::shared_ptr<Class>& object, ReturnType(Class::*member)(Args...))
            : memberMethod(std::make_shared<MemberMethodWrapper<Class>>(object.get(), member)) { }

        template <typename Class>
        NonMutableDelegate(const std::shared_ptr<Class>& object, ReturnType(Class::*member)(Args...) const)
            : memberMethod(std::make_shared<ConstMemberMethodWrapper<Class>>(object.get(), member)) { }

        // Case #3: Create from ref to object and ptr to method (raw reference)
        template <typename Class>
        NonMutableDelegate(Class& object, ReturnType(Class::*member)(Args...))
            : memberMethod(std::make_shared<MemberMethodWrapper<Class>>(&object, member)) { }

        template <typename Class>
        NonMutableDelegate(Class& object, ReturnType(Class::*member)(Args...) const)
            : memberMethod(std::make_shared<ConstMemberMethodWrapper<Class>>(&object, member)) { }

        // Case #4: Create from unique_ptr to object and ptr to method
        template <typename Class>
        NonMutableDelegate(const std::unique_ptr<Class>& object, ReturnType(Class::*member)(Args...))
            : memberMethod(std::make_shared<MemberMethodWrapper<Class>>(object.get(), member)) { }

        template <typename Class>
        NonMutableDelegate(const std::unique_ptr<Class>& object, ReturnType(Class::*member)(Args...) const)
            : memberMethod(std::make_shared<ConstMemberMethodWrapper<Class>>(object.get(), member)) { }

        // Simple function pointer constructor
        constexpr NonMutableDelegate(DelegateRawFunctionType simpleFunction) noexcept
            : simpleFunction(simpleFunction) { }

        virtual ~NonMutableDelegate() = default;

        NonMutableDelegate& operator=(const NonMutableDelegate& other) noexcept
        {
            if (this != &other)
            {
                this->simpleFunction = other.simpleFunction;
                this->memberMethod = other.memberMethod;
                this->sourceDelegate = other.sourceDelegate;
            }
            return *this;
        }

        NonMutableDelegate& operator=(NonMutableDelegate&& other) noexcept
        {
            if (this != &other)
            {
                this->simpleFunction = std::move(other.simpleFunction);
                this->memberMethod = std::move(other.memberMethod);
                this->sourceDelegate = std::move(other.sourceDelegate);
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
            if (sourceDelegate)
            {
                return sourceDelegate.value()(std::forward<decltype(args)>(args)...);
            }
            throw std::bad_function_call{};
        }

        virtual bool operator==(const NonMutableDelegate& other) const
        {
            if (simpleFunction && other.simpleFunction)
            {
                return simpleFunction == other.simpleFunction;
            }
            if (memberMethod && other.memberMethod)
            {
                return *memberMethod == *other.memberMethod;
            }
            if (sourceDelegate && other.sourceDelegate)
            {
                return sourceDelegate.value() == other.sourceDelegate.value();
            }
            return false;
        }

    private:
        class MemberMethodWrapperBase
        {
        public:
            virtual ReturnType operator()(Args... args) = 0;
            virtual bool operator==(const MemberMethodWrapperBase& other) const = 0;
            virtual ~MemberMethodWrapperBase() = default;
        };

        template <typename Class>
        class MemberMethodWrapper : public MemberMethodWrapperBase
        {
        public:
            MemberMethodWrapper(Class* object, ReturnType(Class::*method)(Args...)) noexcept
                : objectPtr(object), method(method) { }

            ReturnType operator()(Args... args) override
            {
                return (objectPtr->*method)(args...);
            }

            bool operator==(const MemberMethodWrapperBase& other) const override
            {
                const MemberMethodWrapper* otherWrapper = dynamic_cast<const MemberMethodWrapper*>(&other);
                if (!otherWrapper)
                {
                    return false;
                }
                return this->objectPtr == otherWrapper->objectPtr
                    && this->method == otherWrapper->method;
            }

        private:
            Class* objectPtr;
            ReturnType(Class::*method)(Args...);
        };

        template <typename Class>
        class ConstMemberMethodWrapper : public MemberMethodWrapperBase
        {
        public:
            ConstMemberMethodWrapper(Class* object, ReturnType(Class::*method)(Args...) const) noexcept
                : objectPtr(object), method(method) { }

            ReturnType operator()(Args... args) override
            {
                return (objectPtr->*method)(args...);
            }

            bool operator==(const MemberMethodWrapperBase& other) const override
            {
                const ConstMemberMethodWrapper* otherWrapper = dynamic_cast<const ConstMemberMethodWrapper*>(&other);
                if (!otherWrapper)
                {
                    return false;
                }
                return this->objectPtr == otherWrapper->objectPtr
                    && this->method == otherWrapper->method;
            }

        private:
            Class* objectPtr;
            ReturnType(Class::*method)(Args...) const;
        };

        DelegateRawFunctionType* simpleFunction = nullptr;
        std::shared_ptr<MemberMethodWrapperBase> memberMethod = nullptr;
        std::optional<Delegate<ReturnType(Args...)>> sourceDelegate;
    };

    // Deduction guides
    template <typename ReturnType, typename... Args>
    NonMutableDelegate(ReturnType(function)(Args...)) -> NonMutableDelegate<ReturnType(Args...)>;

    template <typename ReturnType, typename... Args>
    NonMutableDelegate(const Delegate<ReturnType(Args...)>& delegate) -> NonMutableDelegate<ReturnType(Args...)>;

    template <typename Class, typename ReturnType, typename... Args>
    NonMutableDelegate(Class& object, ReturnType(Class::*member)(Args...)) -> NonMutableDelegate<ReturnType(Args...)>;

    template <typename Class, typename ReturnType, typename... Args>
    NonMutableDelegate(Class& object, ReturnType(Class::*member)(Args...) const) -> NonMutableDelegate<ReturnType(Args...)>;

    template <typename Class, typename ReturnType, typename... Args>
    NonMutableDelegate(const std::shared_ptr<Class>& object, ReturnType(Class::*member)(Args...)) -> NonMutableDelegate<ReturnType(Args...)>;

    template <typename Class, typename ReturnType, typename... Args>
    NonMutableDelegate(const std::shared_ptr<Class>& object, ReturnType(Class::*member)(Args...) const) -> NonMutableDelegate<ReturnType(Args...)>;

    template <typename Class, typename ReturnType, typename... Args>
    NonMutableDelegate(const std::unique_ptr<Class>& object, ReturnType(Class::*member)(Args...)) -> NonMutableDelegate<ReturnType(Args...)>;

    template <typename Class, typename ReturnType, typename... Args>
    NonMutableDelegate(const std::unique_ptr<Class>& object, ReturnType(Class::*member)(Args...) const) -> NonMutableDelegate<ReturnType(Args...)>;
}