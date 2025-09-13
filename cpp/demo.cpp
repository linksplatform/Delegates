#include <iostream>
#include <memory>
#include <Platform.Delegates/Platform.Delegates.h>

using namespace Platform::Delegates;

struct Object
{
    void Method(const char *str) 
    {
        std::cout << "Object::Method(" << str << ")" << std::endl;
    }
};

struct Setter
{
    void Set(const char *str)
    {
        std::cout << "Setter::Set(" << str << ")" << std::endl;
    }
};

struct SetterExtender
{
    std::shared_ptr<Setter> setter = std::make_shared<Setter>();

    // Using the new MethodDelegate with method pointer as template parameter
    MethodDelegate<&Setter::Set> Set = setter;
};

void call(auto&& delegate)
{
    delegate("calling delegate passed to function");
}

int main()
{
    std::cout << "=== Demonstrating the new MethodDelegate functionality ===" << std::endl;
    
    // Old way (still supported)
    std::cout << "\n1. Traditional delegate syntax:" << std::endl;
    auto obj1 = std::make_shared<Object>();
    Delegate<void(const char*)> traditionalDelegate = {obj1, &Object::Method};
    traditionalDelegate("call via traditional delegate");

    // New way - method pointer as template parameter
    std::cout << "\n2. New MethodDelegate syntax:" << std::endl;
    auto obj2 = std::make_shared<Object>();
    MethodDelegate<&Object::Method> newDelegate(obj2);
    newDelegate("call via new MethodDelegate");

    // Composite usage example
    std::cout << "\n3. Composite usage example:" << std::endl;
    SetterExtender se1;
    se1.Set("call from main");

    std::cout << "\n4. Passing to generic function:" << std::endl;
    call(se1.Set);
    
    std::cout << "\n5. Lambda wrapper (for comparison):" << std::endl;
    call([&](const char *str){ se1.Set(str); });

    // Conversion example
    std::cout << "\n6. Conversion to traditional delegate:" << std::endl;
    Delegate<void(const char*)> converted = newDelegate;
    converted("call via converted delegate");

    std::cout << "\n=== Demo completed successfully! ===" << std::endl;
    return 0;
}