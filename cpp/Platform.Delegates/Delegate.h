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
        template<typename... U>
        size_t getFunctionIdentifier(std::function<void(Args...)> f) {
            typedef void(functionType)(Args...);
            FunctionTargetAccesser* functionTargetAccesser = reinterpret_cast<FunctionTargetAccesser*>(&f);
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

    public:
        Delegate() {}

        void operator+= (std::function<void(Args...)> callback)
        {
            this->callbacks.emplace_back(callback);
        }

        void operator-= (std::function<void(Args...)> callback)
        {
            auto deletedRange = std::remove_if(this->callbacks.begin(), this->callbacks.end(), [&](std::function<void(Args...)>& other) {
                return getFunctionIdentifier(callback) == getFunctionIdentifier(other);
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