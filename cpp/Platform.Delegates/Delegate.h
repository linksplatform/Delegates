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
        static Delegate<ReturnType(Args...)> CreateDelegate(std::shared_ptr<Class> object, ReturnType(Class::* member)(Args...))
        {
            return Delegate<ReturnType(Args...)>(object, member);
        }

        template <typename ReturnType, typename... Args>
        static Delegate<ReturnType(Args...)> CreateDelegate(const std::function<ReturnType(Args...)>& function)
        {
            return Delegate<ReturnType(Args...)>(function);
        }
    };

    template <typename ReturnType, typename... Args>
    class Delegate<ReturnType(Args...)>
    {
        using DelegateRawFunctionType = ReturnType(Args...);
        using DelegateType = Delegate<DelegateRawFunctionType>;
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
            std::shared_ptr<Class> object;
            ReturnType(Class::* member)(Args...);

        public:

            MemberMethod(std::shared_ptr<Class> object, ReturnType(Class::* member)(Args...)) : object(object), member(member) { }

            virtual ReturnType operator()(Args... args) override
            {
                return (this->object.get()->*this->member)(std::forward<Args>(args)...);
            }

            virtual bool operator== (const MemberMethodBase& other) const override
            {
                const MemberMethod* otherMethod = dynamic_cast<const MemberMethod*>(&other);
                if (!otherMethod)
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

        DelegateRawFunctionType* simpleFunction;
        std::shared_ptr<DelegateFunctionType> complexFunction;
        std::shared_ptr<MemberMethodBase> memberMethod;

        static void* GetFunctionTarget(DelegateFunctionType& function)
        {
            DelegateRawFunctionType** functionPointer = function.template target<DelegateRawFunctionType*>();
            if (!functionPointer)
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
            if (!leftTargetPointer && !rightTargetPointer)
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

    public:

        Delegate() : memberMethod(nullptr), simpleFunction(nullptr), complexFunction(nullptr) {}

        Delegate(MemberMethodBase* memberMethod) : memberMethod(std::shared_ptr<MemberMethodBase>(memberMethod)), simpleFunction(nullptr), complexFunction(nullptr) {}

        Delegate(const DelegateRawFunctionType& simpleFunction) : simpleFunction(simpleFunction), memberMethod(nullptr), complexFunction(nullptr) {}

        Delegate(const DelegateFunctionType& complexFunction) : complexFunction(std::shared_ptr<DelegateFunctionType>(new DelegateFunctionType(complexFunction))), simpleFunction(nullptr), memberMethod(nullptr) {}

        template <typename Class>
        Delegate(std::shared_ptr<Class> object, ReturnType(Class::* member)(Args...)) : Delegate(new MemberMethod(object, member)) { }

        Delegate(const DelegateType& other) : simpleFunction(other.simpleFunction), memberMethod(other.memberMethod), complexFunction(other.complexFunction) {}

        void operator=(const DelegateType& other)
        {
            this->simpleFunction = other.simpleFunction;
            this->memberMethod = other.memberMethod;
            this->complexFunction = other.complexFunction;
        }

        virtual ReturnType operator()(Args... args)
        {
            if (simpleFunction)
            {
                return simpleFunction(args...);
            }
            else if (memberMethod)
            {
                return (*memberMethod)(args...);
            }
            else if (complexFunction)
            {
                return (*complexFunction)(args...);
            }
            else
            {
                throw std::bad_function_call();
            }
        }

        virtual bool operator== (const DelegateType& other) const
        {
            if (simpleFunction)
            {
                return simpleFunction == other.simpleFunction;
            }
            else if (memberMethod && other.memberMethod)
            {
                return *memberMethod == *other.memberMethod;
            }
            else if (complexFunction && other.complexFunction)
            {
                return AreFunctionsEqual(*complexFunction, *other.complexFunction);
            }
            else
            {
                return false;
            }
        }

        virtual bool operator!= (const DelegateType& other) const
        {
            return !(*this == other);
        }
    };
}

#endif
