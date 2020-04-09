#pragma once

#ifndef PLATFORM_DELEGATES_CREATEDELEGATE
#define PLATFORM_DELEGATES_CREATEDELEGATE

#include "Platform.Delegates.Delegate.h"

namespace Platform::Delegates
{
    template <typename ReturnType, typename... Args>
    static Delegate<ReturnType(Args...)> CreateDelegate(ReturnType(&function)(Args...))
    {
        return Delegate<ReturnType(Args...)>(function);
    }

    template <typename ReturnType, typename... Args>
    static Delegate<ReturnType(Args...)> CreateDelegate(const std::function<ReturnType(Args...)> &function)
    {
        return Delegate<ReturnType(Args...)>(function);
    }

    template <typename Class, typename ReturnType, typename... Args>
    static Delegate<ReturnType(Args...)> CreateDelegate(std::shared_ptr<Class> object, ReturnType(Class:: *member)(Args...))
    {
        return Delegate<ReturnType(Args...)>(object, member);
    }
}

#endif