#ifndef PLATFORM_DELEGATES_CREATEDELEGATE_H
#define PLATFORM_DELEGATES_CREATEDELEGATE_H

#include "Platform.Delegates.Delegate.h"

namespace Platform::Delegates
{
    template <typename ReturnType, typename... Args>
    Delegate(ReturnType(function)(Args...)) -> Delegate<ReturnType(Args...)>;

    template <typename ReturnType, typename... Args>
    Delegate(std::function<ReturnType(Args...)> function) -> Delegate<ReturnType(Args...)>;

    template <typename Class, typename ReturnType, typename... Args>
    Delegate(std::shared_ptr<Class> object, ReturnType(Class:: *member)(Args...)) -> Delegate<ReturnType(Args...)>;
}

#endif
