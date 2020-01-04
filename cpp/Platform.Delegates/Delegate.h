#pragma once
#ifndef PLATFORM_DELEGATES_DELEGATE
#define PLATFORM_DELEGATES_DELEGATE

// Based on https://stackoverflow.com/a/23974539/710069 and https://stackoverflow.com/a/35920804/710069

#include <algorithm>
#include <iostream>
#include <vector>
#include <functional>
#include <thread>
#include <mutex>

namespace Platform::Delegates
{
    template <typename Signature>
    class Delegate;

    template <typename ReturnType, typename... Args>
    class Delegate<ReturnType(Args...)>
    {
        using DelegateRawFunctionType = ReturnType(Args...);
        using DelegateFunctionType = std::function<DelegateRawFunctionType>;

        std::vector<DelegateFunctionType> callbacks;
        std::mutex mutex;

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

    public:
        Delegate() {}

        Delegate(const DelegateFunctionType& callback)
        {
            this += callback;
        }

        Delegate(const Delegate&) = delete;

        void operator=(const Delegate&) = delete;

        void operator+= (const DelegateFunctionType&& callback)
        {
            const std::lock_guard<std::mutex> lock(mutex);
            this->callbacks.emplace_back(callback);
        }

        void operator-= (DelegateFunctionType&& callback)
        {
            const std::lock_guard<std::mutex> lock(mutex);
            auto searchResult = std::find_if(this->callbacks.rbegin(), this->callbacks.rend(),
                [&](DelegateFunctionType& other)
                {
                    return AreFunctionsEqual(callback, other);
                });
            if (searchResult != this->callbacks.rend()) {
                auto removedCallbackPosition = --(searchResult.base());
                this->callbacks.erase(removedCallbackPosition);
            }
        }

        ReturnType operator()(Args... args)
        {
            const std::lock_guard<std::mutex> lock(mutex);
            auto callbacksSize = this->callbacks.size();
            if (callbacksSize == 0)
            {
                throw std::bad_function_call();
            }
            auto lastElement = callbacksSize - 1;
            for (size_t i = 0; i < lastElement; i++)
            {
                this->callbacks[i](args...);
            }
            return this->callbacks[lastElement](args...);
        }
    };
}

#endif
