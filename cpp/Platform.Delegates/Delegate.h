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

    template <typename... Args>
    class Delegate<void(Args...)>
    {
        std::vector<std::function<void(Args...)>> callbacks;
        std::mutex mutex;

        Delegate(Delegate const&) = delete;

        void operator=(Delegate const&) = delete;

        // Simple function means no std::bind was used
        bool IsSimpleFunction(std::function<void(Args...)> function)
        {
            typedef void(functionType)(Args...);
            functionType** functionPointer = function.template target<functionType*>();
            return functionPointer != NULL;
        }

        bool AreEqual(std::function<void(Args...)> left, std::function<void(Args...)> right)
        {
            const int size = sizeof(std::function<void(Args...)>);
            std::byte leftArray[size] = { {(std::byte)0} };
            std::byte rightArray[size] = { {(std::byte)0} };
            std::byte* leftByte = (std::byte*) new (&leftArray) std::function<void(Args...)>(left);
            std::byte* rightByte = (std::byte*) new (&rightArray) std::function<void(Args...)>(right);

            // PrintFunctionsBytes(leftByte, rightByte, size);

            // Here the HACK starts
            // By resetting certain values we are able to compare functions correctly
            // When values are reset it has the same effect as when these values are ignored
            bool isSimpleFunction = IsSimpleFunction(left);
            if (!isSimpleFunction)
            {
                ResetAt(leftArray, rightArray, 16);
            }
            ResetAt(leftArray, rightArray, 56);
            ResetAt(leftArray, rightArray, 57);
            // Here the HACK ends

            for (int i = 0; i < size; i++, leftByte++, rightByte++)
            {
                if (*leftByte != *rightByte)
                {
                    return false;
                }
            }
            return true;
        }

        void ResetAt(std::byte* leftArray, std::byte* rightArray, int i)
        {
            leftArray[i] = (std::byte)0;
            rightArray[i] = (std::byte)0;
        }

        void PrintFunctionsBytes(std::byte* leftFirstByte, std::byte* rightFirstByte, unsigned long long size)
        {
            std::vector<std::byte> leftVector(leftFirstByte, leftFirstByte + size);
            std::vector<std::byte> rightVector(rightFirstByte, rightFirstByte + size);
            std::cout << "Left: ";
            for (int i = 0; i < size; i++)
            {
                std::cout << i << ':' << (int)leftVector[i] << std::endl;
            }
            std::cout << "Right: ";
            for (int i = 0; i < size; i++)
            {
                std::cout << i << ':' << (int)rightVector[i] << std::endl;
            }
        }

    public:
        Delegate() {}

        void operator+= (std::function<void(Args...)> callback)
        {
            const std::lock_guard<std::mutex> lock(mutex);
            this->callbacks.emplace_back(callback);
        }

        void operator-= (std::function<void(Args...)> callback)
        {
            const std::lock_guard<std::mutex> lock(mutex);
            auto deletedRange = std::remove_if(this->callbacks.begin(), this->callbacks.end(),
                [&](std::function<void(Args...)>& other)
                {
                    return AreEqual(callback, other);
                });
            this->callbacks.erase(deletedRange, this->callbacks.end());
        }

        void operator()(Args... args)
        {
            const std::lock_guard<std::mutex> lock(mutex);
            for (auto callback : this->callbacks)
            {
                callback(args...);
            }
        }
    };
}

#endif