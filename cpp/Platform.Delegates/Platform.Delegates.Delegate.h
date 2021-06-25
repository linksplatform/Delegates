#ifndef PLATFORM_DELEGATES_DELEGATE_H
#define PLATFORM_DELEGATES_DELEGATE_H

// Based on https://stackoverflow.com/a/23974539/710069 and https://stackoverflow.com/a/35920804/710069

#include <memory>
#include <iostream>
#include <functional>

namespace Platform::Delegates
{
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
                : simpleFunction(simpleFunction) {}

        Delegate(const DelegateFunctionType &complexFunction)
                : complexFunction(std::make_shared<DelegateFunctionType>(complexFunction)) {}

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
                return simpleFunction(args...);
            }
            if (memberMethod)
            {
                return (*memberMethod)(args...);
            }
            if (complexFunction)
            {
                return (*complexFunction)(args...);
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
                return AreFunctionsEqual(*complexFunction, *other.complexFunction);
            }
            return false;
        }

        virtual bool operator!=(const Delegate &other) const
        {
            return !(*this == other);
        }
    private:
        class MemberMethodBase
        {
        public:
            virtual ReturnType operator()(Args... args) = 0;
            virtual bool operator== (const MemberMethodBase &other) const = 0;
            virtual bool operator!= (const MemberMethodBase &other) const = 0;
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

            bool operator== (const MemberMethodBase &other) const override
            {
                const MemberMethod *otherMethod = dynamic_cast<const MemberMethod *>(&other);
                if (!otherMethod)
                {
                    return false;
                }
                return this->object == otherMethod->object
                    && this->method == otherMethod->method;
            }

            bool operator!= (const MemberMethodBase &other) const override
            {
                return !(*this == other);
            }
        private:
            std::shared_ptr<Class> object;
            ReturnType(Class:: *method)(Args...);
        };

        Delegate(std::shared_ptr<MemberMethodBase> memberMethod) noexcept
                : memberMethod(std::move(memberMethod)) {}

        static DelegateRawFunctionType *GetFunctionTarget(DelegateFunctionType &function) noexcept
        {
            DelegateRawFunctionType **functionPointer = function.template target<DelegateRawFunctionType *>();
            if (!functionPointer)
            {
                return nullptr;
            }
            return *functionPointer;
        }

        static bool AreFunctionsEqual(DelegateFunctionType &left, DelegateFunctionType &right)
        {
            auto leftTargetPointer = GetFunctionTarget(left);
            auto rightTargetPointer = GetFunctionTarget(right);
            // Only in the case we have two std::functions created using std::bind we have to use alternative way to compare functions
            if (!leftTargetPointer && !rightTargetPointer)
            {
                return AreBoundFunctionsEqual(left, right);
            }
            return leftTargetPointer == rightTargetPointer;
        }

        static bool AreBoundFunctionsEqual(const DelegateFunctionType &left, const DelegateFunctionType &right)
        {
            constexpr size_t size = sizeof(DelegateFunctionType);
            std::byte leftArray[size] = { {(std::byte)0} };
            std::byte rightArray[size] = { {(std::byte)0} };
            new (&leftArray) DelegateFunctionType(left);
            new (&rightArray) DelegateFunctionType(right);
            //PrintBytes(leftArray, rightArray, size);
            ApplyHack<size>(leftArray, rightArray);
            return std::equal(std::begin(leftArray), std::end(leftArray), std::begin(rightArray));
        }

        // By resetting certain values we are able to compare functions correctly
        // When values are reset it has the same effect as when these values are ignored
        template<size_t size>
        static void ApplyHack(std::byte *leftArray, std::byte *rightArray)
        {
            if constexpr (size == 64) // x64 (64-bit) MSC 19.24.28314 for x64
            {
                ResetAt(leftArray, rightArray, 16);
                ResetAt(leftArray, rightArray, 56);
                ResetAt(leftArray, rightArray, 57);
            }
            else if constexpr (size == 40) // x86 (32-bit) MSC 19.24.28314 for x64
            {
                ResetAt(leftArray, rightArray, 8);
                ResetAt(leftArray, rightArray, 16);
                ResetAt(leftArray, rightArray, 36);
            }
            else
            {
                // Throw exception to prevent memory damage
                throw std::logic_error{"Comparison for function created using "
                        "std::bind is not supported in your environment."};
            }
        }

        static void ResetAt(std::byte *leftArray, std::byte *rightArray, const size_t i)
        {
            leftArray[i] = (std::byte)0;
            rightArray[i] = (std::byte)0;
        }

        static void PrintBytes(std::byte *leftFirstByte, std::byte *rightFirstByte, const size_t size)
        {
            std::cout << "Left: " << std::endl;
            PrintBytes(leftFirstByte, size);
            std::cout << "Right: " << std::endl;
            PrintBytes(rightFirstByte, size);
        }

        static void PrintBytes(std::byte *firstByte, const size_t size)
        {
            const std::byte *limitByte = firstByte + size;
            std::byte *byte = firstByte;
            size_t i = 0;
            while (byte != limitByte)
            {
                std::cout << i << ':' << (int)*byte << std::endl;
                i++;
                byte++;
            }
        }

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
}

#endif
