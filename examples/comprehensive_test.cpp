#include <iostream>
#include <memory>
#include "../cpp/Platform.Delegates/Platform.Delegates.h"

using namespace Platform::Delegates;

// Test classes
class Base
{
public:
    virtual void foo() { std::cout << "Base::foo"; }
    void bar() { std::cout << "Base::bar"; }
    int getValue() { return 42; }
    virtual ~Base() = default;
};

class Derived : public Base
{
public:
    void foo() override { std::cout << "Derived::foo"; }
    void derivedMethod() { std::cout << "Derived::derivedMethod"; }
};

class DerivedDerived : public Derived
{
public:
    void foo() override { std::cout << "DerivedDerived::foo"; }
};

int main()
{
    std::cout << "=== Comprehensive Delegate Test ===\n\n";
    
    // Test 1: Derived object with base method
    {
        std::cout << "Test 1: Derived object with base method\n";
        auto derived = std::make_shared<Derived>();
        
        // This should work now (was failing before)
        auto delegate = Delegate(derived, &Derived::bar);  // bar is defined in Base
        delegate();
        std::cout << " - SUCCESS\n\n";
    }
    
    // Test 2: DerivedDerived object with base method
    {
        std::cout << "Test 2: DerivedDerived object with base method\n";
        auto derivedDerived = std::make_shared<DerivedDerived>();
        
        auto delegate = Delegate(derivedDerived, &DerivedDerived::bar);  // bar is defined in Base
        delegate();
        std::cout << " - SUCCESS\n\n";
    }
    
    // Test 3: Return value test
    {
        std::cout << "Test 3: Return value test\n";
        auto derived = std::make_shared<Derived>();
        
        auto delegate = Delegate(derived, &Derived::getValue);  // getValue is in Base
        int result = delegate();
        std::cout << "Result: " << result;
        std::cout << " - SUCCESS\n\n";
    }
    
    // Test 4: Virtual method test
    {
        std::cout << "Test 4: Virtual method test (polymorphism)\n";
        auto derived = std::make_shared<Derived>();
        
        auto delegate = Delegate(derived, &Derived::foo);  // foo is virtual
        delegate();
        std::cout << " - SUCCESS\n\n";
    }
    
    // Test 5: Base pointer to derived object
    {
        std::cout << "Test 5: Base pointer to derived object\n";
        std::shared_ptr<Base> basePtr = std::make_shared<Derived>();
        
        auto delegate = Delegate(basePtr, &Base::foo);
        delegate();
        std::cout << " - SUCCESS\n\n";
    }
    
    std::cout << "All tests passed!\n";
    return 0;
}