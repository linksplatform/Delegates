#ifndef PLATFORM_DELEGATES_CREATEDELEGATE_H
#define PLATFORM_DELEGATES_CREATEDELEGATE_H

#include "Platform.Delegates.Delegate.h"

namespace Platform::Delegates
{
    template <typename ReturnType, typename... Args>
    inline Delegate<ReturnType(Args...)> CreateDelegate(ReturnType(&function)(Args...))
    {
        return Delegate<ReturnType(Args...)>{function};
    }

    template <typename ReturnType, typename... Args>
    inline Delegate<ReturnType(Args...)> CreateDelegate(const std::function<ReturnType(Args...)> &function)
    {
        return Delegate<ReturnType(Args...)>{function};
    }

    template <typename Class, typename ReturnType, typename... Args>
    inline Delegate<ReturnType(Args...)> CreateDelegate(std::shared_ptr<Class> object, ReturnType(Class:: *member)(Args...))
    {
        return Delegate<ReturnType(Args...)>{object, member};
    }
}

#endif
