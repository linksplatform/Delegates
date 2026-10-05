#include <iostream>
#include <memory>
#include "../cpp/Platform.Delegates/Platform.Delegates.h"

using namespace Platform::Delegates;

int globalValue = 0;

void setGlobalValue(int value) {
    globalValue = value;
}

class TestClass {
public:
    int value = 0;

    void setValue(int v) {
        value = v;
    }

    int getValue() const {
        return value;
    }

    void incrementValue(int increment) {
        value += increment;
    }
};

void testCase1_CreateFromStandardDelegate() {
    std::cout << "=== Test Case 1: Create from standard delegate ===" << std::endl;
    
    auto testObj = std::make_shared<TestClass>();
    Delegate<void(int)> standardDelegate = {testObj, &TestClass::setValue};
    
    NonMutableDelegate<void(int)> nonMutableDelegate = standardDelegate;
    nonMutableDelegate(42);
    
    std::cout << "Expected: 42, Got: " << testObj->value << std::endl;
    std::cout << (testObj->value == 42 ? "PASS" : "FAIL") << std::endl << std::endl;
}

void testCase2_CreateFromSharedPtr() {
    std::cout << "=== Test Case 2: Create from shared_ptr and method ===" << std::endl;
    
    auto testObj = std::make_shared<TestClass>();
    NonMutableDelegate<void(int)> delegate = {testObj, &TestClass::setValue};
    delegate(25);
    
    std::cout << "Expected: 25, Got: " << testObj->value << std::endl;
    std::cout << (testObj->value == 25 ? "PASS" : "FAIL") << std::endl << std::endl;
}

void testCase3_CreateFromObjectReference() {
    std::cout << "=== Test Case 3: Create from object reference and method ===" << std::endl;
    
    TestClass testObj;
    NonMutableDelegate<void(int)> delegate = {testObj, &TestClass::setValue};
    delegate(15);
    
    std::cout << "Expected: 15, Got: " << testObj.value << std::endl;
    std::cout << (testObj.value == 15 ? "PASS" : "FAIL") << std::endl << std::endl;
}

void testCase4_CreateFromUniquePtr() {
    std::cout << "=== Test Case 4: Create from unique_ptr and method ===" << std::endl;
    
    auto testObj = std::make_unique<TestClass>();
    NonMutableDelegate<void(int)> delegate = {testObj, &TestClass::setValue};
    delegate(30);
    
    std::cout << "Expected: 30, Got: " << testObj->value << std::endl;
    std::cout << (testObj->value == 30 ? "PASS" : "FAIL") << std::endl << std::endl;
}

void testSimpleFunction() {
    std::cout << "=== Test: Simple function pointer ===" << std::endl;
    
    NonMutableDelegate<void(int)> delegate = setGlobalValue;
    delegate(100);
    
    std::cout << "Expected: 100, Got: " << globalValue << std::endl;
    std::cout << (globalValue == 100 ? "PASS" : "FAIL") << std::endl << std::endl;
}

void testReturnValue() {
    std::cout << "=== Test: Return values ===" << std::endl;
    
    TestClass testObj;
    testObj.value = 99;
    
    NonMutableDelegate<int()> delegate = {testObj, &TestClass::getValue};
    int result = delegate();
    
    std::cout << "Expected: 99, Got: " << result << std::endl;
    std::cout << (result == 99 ? "PASS" : "FAIL") << std::endl << std::endl;
}

void demonstrateMainBenefit() {
    std::cout << "=== Demonstration: Key benefit - no shared_ptr overhead ===" << std::endl;
    
    // This demonstrates the main purpose - using objects with guaranteed lifetime
    {
        TestClass scopedObj;
        NonMutableDelegate<void(int)> delegate = {scopedObj, &TestClass::setValue};
        
        // The delegate uses raw pointer, no shared_ptr overhead
        delegate(123);
        std::cout << "First call - Expected: 123, Got: " << scopedObj.value << std::endl;
        std::cout << (scopedObj.value == 123 ? "PASS" : "FAIL") << std::endl;
        
        // As long as scopedObj is in scope, the delegate is safe to use
        delegate(456);
        std::cout << "Second call - Expected: 456, Got: " << scopedObj.value << std::endl;
        std::cout << (scopedObj.value == 456 ? "PASS" : "FAIL") << std::endl;
    }
    // After this scope, delegate would be invalid, but that's the user's responsibility
    // This is the trade-off for avoiding shared_ptr overhead
    
    std::cout << std::endl;
}

int main() {
    std::cout << "Testing NonMutableDelegate Implementation" << std::endl;
    std::cout << "=========================================" << std::endl << std::endl;
    
    try {
        testCase1_CreateFromStandardDelegate();
        testCase2_CreateFromSharedPtr();
        testCase3_CreateFromObjectReference();
        testCase4_CreateFromUniquePtr();
        testSimpleFunction();
        testReturnValue();
        demonstrateMainBenefit();
        
        std::cout << "All tests completed!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}