#include <iostream>
#include <memory>
#include <string>
#include <cassert>
#include "Platform.Delegates/Platform.Delegates.h"

using namespace Platform::Delegates;

struct TestObject {
    int value = 0;
    std::string name;

    void setValue(int val) {
        value = val;
    }

    int getValue() const {
        return value;
    }

    void setName(const std::string& n) {
        name = n;
    }

    std::string getName() const {
        return name;
    }

    int addValues(int a, int b) {
        return a + b;
    }
};

int main() {
    std::cout << "Testing new delegate template functionality..." << std::endl;

    // Test 1: Basic functionality
    {
        auto obj = std::make_shared<TestObject>();
        MethodDelegate<&TestObject::setValue> delegate(obj);
        
        delegate(42);
        assert(obj->value == 42);
        std::cout << "✓ Basic functionality test passed" << std::endl;
    }

    // Test 2: Return value
    {
        auto obj = std::make_shared<TestObject>();
        obj->value = 100;
        
        MethodDelegate<&TestObject::getValue> delegate(obj);
        
        int result = delegate();
        assert(result == 100);
        std::cout << "✓ Return value test passed" << std::endl;
    }

    // Test 3: String handling
    {
        auto obj = std::make_shared<TestObject>();
        
        MethodDelegate<&TestObject::setName> setDelegate(obj);
        MethodDelegate<&TestObject::getName> getDelegate(obj);
        
        setDelegate("Hello World");
        std::string result = getDelegate();
        assert(result == "Hello World");
        std::cout << "✓ String handling test passed" << std::endl;
    }

    // Test 4: Multiple arguments
    {
        auto obj = std::make_shared<TestObject>();
        
        MethodDelegate<&TestObject::addValues> delegate(obj);
        
        int result = delegate(10, 20);
        assert(result == 30);
        std::cout << "✓ Multiple arguments test passed" << std::endl;
    }

    // Test 5: Conversion to traditional delegate
    {
        auto obj = std::make_shared<TestObject>();
        
        MethodDelegate<&TestObject::setValue> methodDelegate(obj);
        
        // Test conversion to traditional delegate
        Delegate<void(int)> traditionalDelegate = methodDelegate;
        
        traditionalDelegate(99);
        assert(obj->value == 99);
        std::cout << "✓ Conversion test passed" << std::endl;
    }

    // Test 6: Traditional delegate still works
    {
        auto obj = std::make_shared<TestObject>();
        Delegate<void(int)> delegate = {obj, &TestObject::setValue};
        delegate(77);
        assert(obj->value == 77);
        std::cout << "✓ Traditional delegate compatibility test passed" << std::endl;
    }

    std::cout << "All tests passed! ✅" << std::endl;
    return 0;
}