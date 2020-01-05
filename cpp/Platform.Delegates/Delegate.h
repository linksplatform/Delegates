#pragma once
#ifndef PLATFORM_DELEGATES_DELEGATE
#define PLATFORM_DELEGATES_DELEGATE

// Based on https://stackoverflow.com/a/23974539/710069 and https://stackoverflow.com/a/35920804/710069

#include <algorithm>
#include <iostream>
#include <functional>

namespace Platform::Delegates
{
    template <typename...>
    class Delegate
    {
    public:

        template <typename Class, typename ReturnType, typename... Args>
        static Delegate<ReturnType(Args...)> CreateDelegate(Class* object, ReturnType(Class::* member)(Args...))
        {
            return Delegate<ReturnType(Args...)>(object, member);
        }

        template <typename ReturnType, typename... Args>
        static Delegate<ReturnType(Args...)> CreateDelegate(std::function<ReturnType(Args...)>&& function)
        {
            return Delegate<ReturnType(Args...)>(function);
        }
    };

    template <typename ReturnType, typename... Args>
    class Delegate<ReturnType(Args...)>
    {
        using DelegateRawFunctionType = ReturnType(Args...);
        using DelegateFunctionType = std::function<DelegateRawFunctionType>;

        class MemberMethodBase
        {
        public:
            virtual ReturnType operator()(Args... args) = 0;
            virtual bool operator== (const MemberMethodBase& other) const = 0;
            virtual bool operator!= (const MemberMethodBase& other) const = 0;
        };

        template <typename Class>
        class MemberMethod : public MemberMethodBase
        {
            Class* object;
            ReturnType(Class::* member)(Args...);

        public:

            MemberMethod(Class* object, ReturnType(Class::* member)(Args...)) : object(object), member(member) { }

            virtual ReturnType operator()(Args... args) override
            {
                return (this->object->*this->member)(std::forward<Args>(args)...);
            }

            virtual bool operator== (const MemberMethodBase& other) const override
            {
                const MemberMethod* otherMethod = dynamic_cast<const MemberMethod*>(&other);
                if (otherMethod == nullptr)
                {
                    return false;
                }
                return this->object == otherMethod->object
                    && this->member == otherMethod->member;
            }

            virtual bool operator!= (const MemberMethodBase& other) const override
            {
                return !(*this == other);
            }
        };

        static void* GetFunctionTarget(DelegateFunctionType& function)
        {
            DelegateRawFunctionType** functionPointer = function.template target<DelegateRawFunctionType*>();
            if (functionPointer == nullptr)
            {
                return nullptr;
            }
            return *functionPointer;
        }

        static bool AreFunctionsEqual(DelegateFunctionType& left, DelegateFunctionType& right)
        {
            auto leftTargetPointer = GetFunctionTarget(left);
            auto rightTargetPointer = GetFunctionTarget(right);
            // Only in the case we have two std::functions created using std::bind we have to use alternative way to compare functions
            if (leftTargetPointer == nullptr && rightTargetPointer == nullptr)
            {
                return AreBoundFounctionsEqual(left, right);
            }
            return leftTargetPointer == rightTargetPointer;
        }

        static bool AreBoundFounctionsEqual(const DelegateFunctionType& left, const DelegateFunctionType& right)
        {
            const size_t size = sizeof(DelegateFunctionType);
            std::byte leftArray[size] = { {(std::byte)0} };
            std::byte rightArray[size] = { {(std::byte)0} };
            new (&leftArray) DelegateFunctionType(left);
            new (&rightArray) DelegateFunctionType(right);
            // PrintBytes(leftArray, rightArray, size);
            ApplyHack(leftArray, rightArray, size);
            return std::equal(std::begin(leftArray), std::end(leftArray), std::begin(rightArray));
        }

        static void ApplyHack(std::byte* leftArray, std::byte* rightArray, const size_t size)
        {
            // Throw exception to prevent memory damage
            if (size != 64)
            {
                throw std::logic_error("Function comparison is not supported in your environment.");
            }
            // By resetting certain values we are able to compare functions correctly
            // When values are reset it has the same effect as when these values are ignored
            ResetAt(leftArray, rightArray, 16);
            ResetAt(leftArray, rightArray, 56);
            ResetAt(leftArray, rightArray, 57);
        }

        static void ResetAt(std::byte* leftArray, std::byte* rightArray, const size_t i)
        {
            leftArray[i] = (std::byte)0;
            rightArray[i] = (std::byte)0;
        }

        static void PrintBytes(std::byte* leftFirstByte, std::byte* rightFirstByte, const size_t size)
        {
            std::cout << "Left: " << std::endl;
            PrintBytes(leftFirstByte, size);
            std::cout << "Right: " << std::endl;
            PrintBytes(rightFirstByte, size);
        }

        static void PrintBytes(std::byte* firstByte, const size_t size)
        {
            const std::byte* limitByte = firstByte + size;
            std::byte* byte = firstByte;
            size_t i = 0;
            while (byte != limitByte)
            {
                std::cout << i << ':' << (int)*byte << std::endl;
                i++;
                byte++;
            }
        }

        DelegateRawFunctionType* simpleFunction;
        DelegateFunctionType* complexFunction;
        MemberMethodBase* memberMethod;

    public:

        Delegate() : memberMethod(nullptr), simpleFunction(nullptr), complexFunction(nullptr) {}

        Delegate(MemberMethodBase&& memberMethod) : memberMethod(&memberMethod), simpleFunction(nullptr), complexFunction(nullptr) {}

        Delegate(DelegateRawFunctionType& simpleFunction) : simpleFunction(simpleFunction), memberMethod(nullptr), complexFunction(nullptr) {}

        Delegate(DelegateFunctionType& complexFunction) : complexFunction(&complexFunction), simpleFunction(nullptr), memberMethod(nullptr) {}

        template <typename Class>
        Delegate(Class* object, ReturnType(Class::* member)(Args...)) : Delegate(MemberMethod(object, member)) { }

        Delegate(const Delegate<ReturnType(Args...)>& other) : simpleFunction(other.simpleFunction), memberMethod(other.memberMethod), complexFunction(other.complexFunction) {}

        void operator=(const Delegate<ReturnType(Args...)>& other)
        {
            this->simpleFunction = other.simpleFunction;
            this->memberMethod = other.memberMethod;
            this->complexFunction = other.complexFunction;
        }

        virtual ReturnType operator()(Args... args)
        {
            if (simpleFunction != nullptr)
            {
                return simpleFunction(args...);
            }
            else if (memberMethod != nullptr)
            {
                return (*memberMethod)(args...);
            }
            else if (complexFunction != nullptr)
            {
                return (*complexFunction)(args...);
            }
            else
            {
                throw std::bad_function_call();
            }
        }

        virtual bool operator== (const Delegate<ReturnType(Args...)>& other) const
        {
            if (simpleFunction != nullptr)
            {
                return simpleFunction == other.simpleFunction;
            }
            else if (memberMethod != nullptr && other.memberMethod != nullptr)
            {
                return *memberMethod == *other.memberMethod;
            }
            else if (complexFunction != nullptr && other.complexFunction != nullptr)
            {
                return AreFunctionsEqual(*complexFunction, *other.complexFunction);
            }
            else
            {
                return false;
            }
        }

        virtual bool operator!= (const Delegate<ReturnType(Args...)>& other) const
        {
            return !(*this == other);
        }
    };
}

#endif
