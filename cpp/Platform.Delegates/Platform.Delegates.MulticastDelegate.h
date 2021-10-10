#pragma once

// Based on https://stackoverflow.com/a/23974539/710069 

#include <mutex>
#include <list>
#include "Platform.Delegates.Delegate.h"

namespace Platform::Delegates
{
    template <typename...>
    class MulticastDelegate;

    template <typename ReturnType, typename... Args>
    class MulticastDelegate<ReturnType(Args...)> : public Delegate<ReturnType(Args...)>
    {
    public:
        using DelegateRawFunctionType = ReturnType(Args...);
        using DelegateType = Delegate<DelegateRawFunctionType>;
        using DelegateFunctionType = std::function<DelegateRawFunctionType>;

        constexpr MulticastDelegate() noexcept = default;

        MulticastDelegate(const MulticastDelegate &multicastDelegate)
        {
            CopyCallbacks(multicastDelegate);
        }

        MulticastDelegate(MulticastDelegate &&multicastDelegate) noexcept
        {
            MoveCallbacksUnsync(std::move(multicastDelegate));
        }

        MulticastDelegate(const DelegateType &callback)
        {
            *this += callback;
        }

        MulticastDelegate(DelegateRawFunctionType callback)
        {
            *this += callback;
        }

        MulticastDelegate(const DelegateFunctionType &callback)
        {
            *this += callback;
        }

        MulticastDelegate &operator=(const MulticastDelegate &other)
        {
            if (this != &other)
            {
                CopyCallbacks(other);
            }
            return *this;
        }

        MulticastDelegate &operator=(MulticastDelegate &&other) noexcept
        {
            if (this != &other)
            {
                MoveCallbacksUnsync(std::move(other));
            }
            return *this;
        }

        MulticastDelegate &operator=(const DelegateType &other)
        {
            ClearCallbacks();
            *this += other;
            return *this;
        }

        MulticastDelegate &operator=(DelegateRawFunctionType other)
        {
            ClearCallbacks();
            *this += other;
            return *this;
        }

        MulticastDelegate &operator=(const DelegateFunctionType &other)
        {
            ClearCallbacks();
            *this += other;
            return *this;
        }

        MulticastDelegate &operator+=(const DelegateType &callback)
        {
            std::lock_guard lock(mutex);
            this->callbacks.emplace_back(callback);
            return *this;
        }

        MulticastDelegate &operator+=(DelegateRawFunctionType callback)
        {
            return *this += DelegateType{callback};
        }

        MulticastDelegate &operator+=(const DelegateFunctionType &callback)
        {
            return *this += DelegateType{callback};
        }

        MulticastDelegate &operator-=(const DelegateType &callback)
        {
            const std::lock_guard lock{mutex};
            auto searchResult = std::find(this->callbacks.begin(), this->callbacks.end(), callback);
            if (searchResult != this->callbacks.end())
            {
                this->callbacks.erase(searchResult);
            }
            return *this;
        }

        MulticastDelegate &operator-=(DelegateRawFunctionType &callback)
        {
            return *this -= DelegateType{callback};
        }

        MulticastDelegate &operator-=(const DelegateFunctionType &callback)
        {
            return *this -= DelegateType{callback};
        }

        ReturnType operator()(Args... args) override
        {
            const std::lock_guard lock(mutex);
            if (this->callbacks.size() == 0)
            {
                throw std::bad_function_call();
            }
            for (auto callbackIt = this->callbacks.rbegin();
                 callbackIt != std::prev(this->callbacks.rend());
                 ++callbackIt)
            {
                (*callbackIt)(std::forward<decltype(args)>(args)...);
            }
            return this->callbacks.front()(std::forward<decltype(args)>(args)...);
        }

    private:
        void MoveCallbacksUnsync(MulticastDelegate &&other)
        {
            callbacks = std::move(other.callbacks);
        }

        void CopyCallbacksUnsync(const MulticastDelegate &other)
        {
            callbacks = other.callbacks;
        }

        void CopyCallbacks(const MulticastDelegate &other)
        {
            if (this == &other)
            {
                return;
            }
            // To prevent the deadlock the order of locking should be the same for all threads.
            // We can use the addresses of MulticastDelegates to sort the mutexes.
            if (this < &other)
            {
                std::lock_guard lock1(mutex);
                std::lock_guard lock2(other.mutex);
                CopyCallbacksUnsync(other);
            }
            else
            {
                std::lock_guard lock1(other.mutex);
                std::lock_guard lock2(mutex);
                CopyCallbacksUnsync(other);
            }
        }

        void ClearCallbacks()
        {
            std::lock_guard lock(mutex);
            callbacks.clear();
        }

        mutable std::mutex mutex;
        std::list<DelegateType> callbacks;
    };
}

