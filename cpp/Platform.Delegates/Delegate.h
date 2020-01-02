#pragma once

#ifndef PLATFORM_DELEGATE
#define PLATFORM_DELEGATE

// Based on https://stackoverflow.com/a/23974539/710069

#include <algorithm>
#include <iostream>
#include <memory>
#include <utility>
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

        class FunctionAccesser : public std::function<void(Args...)>
        {
        public:
            const void* Target()
            {
                return this->_Target(this->_Target_type());
            }
        };

        template<typename... U>
        size_t getAddress(std::function<void(Args...)> f) {
            typedef void(fnType)(Args...);
            const auto functionPointerTypeName = f.target_type().name();
            FunctionAccesser* functionAccesser = reinterpret_cast<FunctionAccesser*>(&f);
            fnType** fnPointer = f.template target<fnType*>();
            const fnType** voidPointer = (const fnType **) functionAccesser->Target();
            return (size_t)*voidPointer;
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
                return getAddress(callback) == getAddress(other);
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