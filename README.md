[![NuGet Version and Downloads count](https://img.shields.io/nuget/v/Platform.Delegates.TemplateLibrary?label=nuget&style=flat)](https://www.nuget.org/packages/Platform.Delegates.TemplateLibrary)
[![ConanCenter package](https://repology.org/badge/version-for-repo/conancenter/platform.delegates.svg)](https://conan.io/center/platform.delegates)

# Delegates
LinksPlatform's Platform::Delegates Template Class Library

Delegate and MulticastDelegate classes are modeled after [.NET Delegates](https://docs.microsoft.com/en-us/dotnet/csharp/programming-guide/delegates/).

Delegate is a container for callable function. Delegate can contain:
* Pointer to simple function (represented by a single pointer).
* Pointer to an instanse of class and to a member method of this class.
* Pointer to std::function instance.

Unlike std::function Delegate class supports primitive by reference comparision for equality.

MulticastDelegate represents a thread-safe collection of Delegates that are callable at the same time. If you call MulticastDelegate all Delegates added to its collection will be called. MulticastDelegate can be used as a basis for simple event model (similar of that you can find in .NET). Because Delegate class supports the comparision for equality you can both subscribe and unsubscribe any Delegate instance to MulticastDelegate. To be able to unsubscribe from event represented by MulticastDelegate instance you should store Delegate instance somewhere. Due to its thread safety MulticastDelegate instance can be global/static object to which every thread is safe to subscribe.

NuGet package: [Platform.Delegates.TemplateLibrary](https://www.nuget.org/packages/Platform.Delegates.TemplateLibrary)

Conan package: [platform.delegates](http://www.conan.io/center/platform.delegates)

## Example

```C++
#include <Platform.Delegates.h>

void function(const char *str) 
{ 
    std::cout << "function(" << str << ")" << std::endl; 
}

struct Object
{
    void Method(const char *str) 
    {
        std::cout << "Object::Method(" << str << ")" << std::endl;
    }
};

int main()
{
    MulticastDelegate<void(const char *)> event;

    Delegate<void(const char *)> memberMethod(std::make_shared<Object>(), &Object::Method);

    auto lambda = std::make_shared<std::function<void(const char *)>>([](const char *str) 
    { 
        std::cout << "lambda(" << str << ")" << std::endl;
    });

    // Subscribe
    event += function;
    event += lambda;
    event += memberMethod;

    // Raise the event
    event("value");

    // Unsubscribe
    event -= function;
    event -= lambda;
    event -= memberMethod;
}
```
