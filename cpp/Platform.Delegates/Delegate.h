#pragma once
#ifndef PLATFORM_DELEGATES_DELEGATE
#define PLATFORM_DELEGATES_DELEGATE

namespace Platform::Delegates
{
    template <typename Signature>
    class Delegate;

    template <typename ReturnType, typename... Args>
    class Delegate<ReturnType(Args...)>
    {
    };
}

#endif
