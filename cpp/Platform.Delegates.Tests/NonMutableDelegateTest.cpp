#include <gtest/gtest.h>
#include <Platform.Delegates.h>

using namespace Platform::Delegates;

namespace Platform::Delegates::Tests::NonMutableDelegateTest
{
    int globalValue = 0;

    void setGlobalValue(int value) {
        globalValue = value;
    }

    void setGlobalValue2(int value) {
        globalValue = value + 10;
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

    // Test Case #1: Create from standard delegate
    TEST(NonMutableDelegateTest, CreateFromStandardDelegate)
    {
        auto testObj = std::make_shared<TestClass>();
        Delegate<void(int)> standardDelegate = {testObj, &TestClass::setValue};
        
        NonMutableDelegate<void(int)> nonMutableDelegate = standardDelegate;
        nonMutableDelegate(42);
        
        ASSERT_EQ(testObj->value, 42);
    }

    // Test Case #2: Create from shared_ptr and method pointer
    TEST(NonMutableDelegateTest, CreateFromSharedPtrAndMethod)
    {
        auto testObj = std::make_shared<TestClass>();
        NonMutableDelegate<void(int)> delegate = {testObj, &TestClass::setValue};
        delegate(25);
        
        ASSERT_EQ(testObj->value, 25);
    }

    // Test Case #3: Create from object reference and method pointer
    TEST(NonMutableDelegateTest, CreateFromObjectReferenceAndMethod)
    {
        TestClass testObj;
        NonMutableDelegate<void(int)> delegate = {testObj, &TestClass::setValue};
        delegate(15);
        
        ASSERT_EQ(testObj.value, 15);
    }

    // Test Case #4: Create from unique_ptr and method pointer
    TEST(NonMutableDelegateTest, CreateFromUniquePtrAndMethod)
    {
        auto testObj = std::make_unique<TestClass>();
        NonMutableDelegate<void(int)> delegate = {testObj, &TestClass::setValue};
        delegate(30);
        
        ASSERT_EQ(testObj->value, 30);
    }

    // Test simple function pointer
    TEST(NonMutableDelegateTest, SimpleFunctionTest)
    {
        NonMutableDelegate<void(int)> delegate = setGlobalValue;
        delegate(100);
        
        ASSERT_EQ(globalValue, 100);
    }

    // Test return values work correctly
    TEST(NonMutableDelegateTest, ReturnValueTest)
    {
        TestClass testObj;
        testObj.value = 99;
        
        NonMutableDelegate<int()> delegate = {testObj, &TestClass::getValue};
        int result = delegate();
        
        ASSERT_EQ(result, 99);
    }

    // Test with parameters and return value
    TEST(NonMutableDelegateTest, ParametersAndReturnValueTest)
    {
        auto testObj = std::make_unique<TestClass>();
        testObj->value = 10;
        
        NonMutableDelegate<void(int)> setDelegate = {testObj, &TestClass::incrementValue};
        NonMutableDelegate<int()> getDelegate = {testObj, &TestClass::getValue};
        
        setDelegate(5);  // increment by 5
        int result = getDelegate();
        
        ASSERT_EQ(result, 15);  // 10 + 5 = 15
    }

    // Test equality operator
    TEST(NonMutableDelegateTest, EqualityOperatorTest)
    {
        NonMutableDelegate<void(int)> delegate1 = setGlobalValue;
        NonMutableDelegate<void(int)> delegate2 = delegate1;
        
        ASSERT_TRUE(delegate1 == delegate2);
    }

    // Test inequality operator
    TEST(NonMutableDelegateTest, InequalityOperatorTest)
    {
        NonMutableDelegate<void(int)> delegate1 = setGlobalValue;
        NonMutableDelegate<void(int)> delegate2 = setGlobalValue2;
        
        ASSERT_FALSE(delegate1 == delegate2);
    }

    // Test assignment operator
    TEST(NonMutableDelegateTest, AssignmentOperatorTest)
    {
        NonMutableDelegate<void(int)> delegate1 = setGlobalValue;
        NonMutableDelegate<void(int)> delegate2;
        delegate2 = delegate1;
        delegate2(55);
        
        ASSERT_EQ(globalValue, 55);
    }

    // Test move assignment operator
    TEST(NonMutableDelegateTest, MoveAssignmentOperatorTest)
    {
        NonMutableDelegate<void(int)> delegate1 = setGlobalValue;
        NonMutableDelegate<void(int)> delegate2 = std::move(delegate1);
        delegate2(77);
        
        ASSERT_EQ(globalValue, 77);
    }

    // Test bad function call exception
    TEST(NonMutableDelegateTest, BadFunctionCallTest)
    {
        NonMutableDelegate<void()> delegate;
        
        ASSERT_THROW(delegate(), std::bad_function_call);
    }

    // Test member function equality with same object
    TEST(NonMutableDelegateTest, MemberFunctionEqualityTest)
    {
        TestClass testObj;
        NonMutableDelegate<void(int)> delegate1 = {testObj, &TestClass::setValue};
        NonMutableDelegate<void(int)> delegate2 = {testObj, &TestClass::setValue};
        
        ASSERT_TRUE(delegate1 == delegate2);
    }

    // Test member function inequality with different objects
    TEST(NonMutableDelegateTest, MemberFunctionInequalityTest)
    {
        TestClass testObj1;
        TestClass testObj2;
        NonMutableDelegate<void(int)> delegate1 = {testObj1, &TestClass::setValue};
        NonMutableDelegate<void(int)> delegate2 = {testObj2, &TestClass::setValue};
        
        ASSERT_FALSE(delegate1 == delegate2);
    }

    // Test the key benefit: no shared_ptr overhead for scoped objects
    TEST(NonMutableDelegateTest, ScopedObjectLifetimeTest)
    {
        // This test demonstrates the main purpose - using objects with guaranteed lifetime
        {
            TestClass scopedObj;
            NonMutableDelegate<void(int)> delegate = {scopedObj, &TestClass::setValue};
            
            // The delegate uses raw pointer, no shared_ptr overhead
            delegate(123);
            ASSERT_EQ(scopedObj.value, 123);
            
            // As long as scopedObj is in scope, the delegate is safe to use
            delegate(456);
            ASSERT_EQ(scopedObj.value, 456);
        }
        // After this scope, delegate would be invalid, but that's the user's responsibility
        // This is the trade-off for avoiding shared_ptr overhead
    }
}