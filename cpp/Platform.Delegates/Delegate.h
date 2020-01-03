#pragma once

#ifndef PLATFORM_DELEGATE
#define PLATFORM_DELEGATE

// Based on https://stackoverflow.com/a/23974539/710069

#include <algorithm>
#include <iostream>
#include <vector>
#include <functional>

namespace Platform::Delegates
{
    template <typename Signature>
    struct Delegate;

    template <typename... Args>
    struct Delegate<void(Args...)>
    {
        std::vector<std::function<void(Args...)>> callbacks;

        Delegate(Delegate const&) = delete;

        void operator=(Delegate const&) = delete;

        // Access to protected Target method of std::function
        class FunctionTargetAccesser : public std::function<void(Args...)>
        {
        public:
            const void* Target()
            {
                return this->_Target(this->_Target_type());
            }
        };

        // This function is a hack and may be unreliable
        size_t getFunctionIdentifier(std::function<void(Args...)> function)
        {
            typedef void(functionType)(Args...);
            FunctionTargetAccesser* functionTargetAccesser = reinterpret_cast<FunctionTargetAccesser*>(&function);
            const functionType** functionPointer = (const functionType**)functionTargetAccesser->Target();
            const functionType** mayBeFirstArgumentPointer = functionPointer + 2;
            if (*mayBeFirstArgumentPointer)
            {
                // Function unique for each member method pointer and it's bound container class pointer
                // This also works for standalone methods
                return (size_t)*functionPointer ^ (size_t)*mayBeFirstArgumentPointer;
            }
            else
            {
                // Function unique for each member method pointer and it's bound arguments
                int* mayBeArgumentValue = (int*)(functionPointer + 1);
                size_t hash = (size_t)*functionPointer;
                int fillerValue = *mayBeArgumentValue;
                do
                {
                    mayBeArgumentValue++;
                    hash ^= (size_t)*mayBeArgumentValue;
                } while (*mayBeArgumentValue != fillerValue);
                return hash;
            }
        }

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
            PrintFunctionsBytes(leftByte, rightByte, size);
            bool isSimpleFunction = IsSimpleFunction(left);
            if (isSimpleFunction)
            {
                for (int i = 0; i < size; i++, leftByte++, rightByte++)
                {
                    if ((i >= 56 && i <= 57))
                    {
                        continue;
                    }
                    if (*leftByte != *rightByte)
                    {
                        return false;
                    }
                }
            }
            else
            {
                for (int i = 0; i < size; i++, leftByte++, rightByte++)
                {
                    if (i == 16 || (i >= 56 && i <= 57))
                    {
                        continue;
                    }
                    if (*leftByte != *rightByte)
                    {
                        return false;
                    }
                }
                return true;
            }
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
            this->callbacks.emplace_back(callback);
        }

        void operator-= (std::function<void(Args...)> callback)
        {
            auto deletedRange = std::remove_if(this->callbacks.begin(), this->callbacks.end(), [&](std::function<void(Args...)>& other) {
                return AreEqual(callback, other);
                //return getFunctionIdentifier(callback) == getFunctionIdentifier(other);
                });
            this->callbacks.erase(deletedRange, this->callbacks.end());
        }

        void operator()(Args... args)
        {
            for (auto callback : this->callbacks)
            {
                callback(args...);
            }
        }
    };
}

#endif