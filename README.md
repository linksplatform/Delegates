[![badge](https://img.shields.io/badge/conan.io-platform.delegates%2F0.1.3-green.svg?logo=data:image/png;base64%2CiVBORw0KGgoAAAANSUhEUgAAAA4AAAAOCAMAAAAolt3jAAAA1VBMVEUAAABhlctjlstkl8tlmMtlmMxlmcxmmcxnmsxpnMxpnM1qnc1sn85voM91oM11oc1xotB2oc56pNF6pNJ2ptJ8ptJ8ptN9ptN8p9N5qNJ9p9N9p9R8qtOBqdSAqtOAqtR%2BrNSCrNJ/rdWDrNWCsNWCsNaJs9eLs9iRvNuVvdyVv9yXwd2Zwt6axN6dxt%2Bfx%2BChyeGiyuGjyuCjyuGly%2BGlzOKmzOGozuKoz%2BKqz%2BOq0OOv1OWw1OWw1eWx1eWy1uay1%2Baz1%2Baz1%2Bez2Oe02Oe12ee22ujUGwH3AAAAAXRSTlMAQObYZgAAAAFiS0dEAIgFHUgAAAAJcEhZcwAACxMAAAsTAQCanBgAAAAHdElNRQfgBQkREyOxFIh/AAAAiklEQVQI12NgAAMbOwY4sLZ2NtQ1coVKWNvoc/Eq8XDr2wB5Ig62ekza9vaOqpK2TpoMzOxaFtwqZua2Bm4makIM7OzMAjoaCqYuxooSUqJALjs7o4yVpbowvzSUy87KqSwmxQfnsrPISyFzWeWAXCkpMaBVIC4bmCsOdgiUKwh3JojLgAQ4ZCE0AMm2D29tZwe6AAAAAElFTkSuQmCC)](http://www.conan.io/center/platform.delegates)
[![NuGet Version and Downloads count](https://buildstats.info/nuget/Platform.Delegates.TemplateLibrary)](https://www.nuget.org/packages/Platform.Delegates.TemplateLibrary)

# Delegates
LinksPlatform's Platform.Delegates Template Class Library

Delegate and MulticastDelegate classes are modeled after [.NET Delegates](https://docs.microsoft.com/en-us/dotnet/csharp/programming-guide/delegates/).

Delegate is a container for callable function. Delegate can contain:
* Pointer to simple function (represented by a single pointer).
* Pointer to an instanse of class and to a member method of this class.
* Pointer to std::function instance.

Unlike std::function Delegate class supports primitive by reference comparision for equality.

MulticastDelegate represents a collection of Delegates that are callable at the same time. If you call MulticastDelegate all Delegates added to its collection will be called. MulticastDelegate can be used as a basis for simple event model (similar of that you can find in .NET). Because Delegate class supports the comparision for equality you can both subscribe and unsubscribe any Delegate instance to MulticastDelegate. To be able to unsubscribe from event represented by MulticastDelegate instance you should store Delegate instance somewhere.

```C++
void function(const char *str) { std::cout << "function(" << str << ")\n"; }

struct Object
{
    void Method(const char *str) {
        std::cout << "Object::Method(" << str << ")" << std::endl;
    }
};

int main()
{
  MulticastDelegate<void(const char *)> event;

  Delegate<void(const char *)> memberMethod(std::make_shared<Object>(), &Object::Method);
  
  std::function<void(const char *)> lambda = [](const char *str) { std::cout << "lambda(" << str << ")\n"; };

  // Subscribe
  event += function;
  event += lambda;
  event += memberMethod;

  // Raise event
  event("value");

  // Unsubscribe
  event -= function;
  event -= lambda;
  event -= memberMethod;
}
```
