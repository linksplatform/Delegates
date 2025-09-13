#include <gtest/gtest.h>
#include <Platform.Delegates.h>

using namespace Platform::Delegates;

namespace Platform::Delegates::Tests::DelegatesTest
{
    int globalA = 0;

    // Standalone function for testing
    void setGlobalA(int value) {
        globalA = value;
    }

    void setGlobalA2(int value) {
        globalA = value + 1;
    }

    TEST(DelegateTest, SimpleFunctionTest)
    {
        Delegate<void(int)> delegate = setGlobalA;
        delegate(1);

        ASSERT_EQ(globalA, 1);
    }

    struct TestStruct {
        int a = 0;

        void setA(int value) {
            a = value;
        }
    };

    TEST(DelegateTest, MemberFunctionTest)
    {
        auto testStruct = std::make_shared<TestStruct>();
        Delegate<void(int)> delegate = {testStruct, &TestStruct::setA};
        delegate(5);

        ASSERT_EQ(testStruct->a, 5);
    }

    TEST(DelegateTest, EqualityOperatorTest)
    {
        Delegate<void(int)> delegate1 = setGlobalA;
        Delegate<void(int)> delegate2 = delegate1;

        ASSERT_TRUE(delegate1 == delegate2);
    }

    TEST(DelegateTest, InequalityOperatorTest)
    {
        Delegate<void(int)> delegate1 = setGlobalA;
        Delegate<void(int)> delegate2 = setGlobalA2;

        ASSERT_FALSE(delegate1 == delegate2);
    }

    TEST(DelegateTest, AssignmentOperatorTest)
    {
        Delegate<void(int)> delegate1 = setGlobalA;
        Delegate<void(int)> delegate2;
        delegate2 = delegate1;
        delegate2(1);

        ASSERT_EQ(globalA, 1);
    }

    TEST(DelegateTest, MoveAssignmentOperatorTest)
    {
        Delegate<void(int)> delegate1 = setGlobalA;
        Delegate<void(int)> delegate2 = std::move(delegate1);
        delegate2(1);

        ASSERT_EQ(globalA, 1);
    }

    TEST(DelegateTest, InvokeTest)
    {
        Delegate<void(int)> delegate = setGlobalA;
        delegate(1);

        ASSERT_EQ(globalA, 1);
    }

    TEST(DelegateTest, BadFunctionCallTest)
    {
        Delegate<void()> delegate;

        ASSERT_THROW(delegate(), std::bad_function_call);
    }

    // Tests for the new template method pointer-based delegate
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

    TEST(DelegateTest, MethodPointerTemplateBasicTest)
    {
        auto obj = std::make_shared<TestObject>();
        MethodDelegate<&TestObject::setValue> delegate(obj);
        
        delegate(42);
        ASSERT_EQ(obj->value, 42);
    }

    TEST(DelegateTest, MethodPointerTemplateReturnValueTest)
    {
        auto obj = std::make_shared<TestObject>();
        obj->value = 100;
        
        MethodDelegate<&TestObject::getValue> delegate(obj);
        
        int result = delegate();
        ASSERT_EQ(result, 100);
    }

    TEST(DelegateTest, MethodPointerTemplateStringTest)
    {
        auto obj = std::make_shared<TestObject>();
        
        MethodDelegate<&TestObject::setName> setDelegate(obj);
        MethodDelegate<&TestObject::getName> getDelegate(obj);
        
        setDelegate("Hello World");
        std::string result = getDelegate();
        ASSERT_EQ(result, "Hello World");
    }

    TEST(DelegateTest, MethodPointerTemplateMultipleArgsTest)
    {
        auto obj = std::make_shared<TestObject>();
        
        MethodDelegate<&TestObject::addValues> delegate(obj);
        
        int result = delegate(10, 20);
        ASSERT_EQ(result, 30);
    }

    TEST(DelegateTest, MethodPointerTemplateEqualityTest)
    {
        auto obj1 = std::make_shared<TestObject>();
        auto obj2 = std::make_shared<TestObject>();
        
        MethodDelegate<&TestObject::setValue> delegate1(obj1);
        MethodDelegate<&TestObject::setValue> delegate2(obj1);  // Same object
        MethodDelegate<&TestObject::setValue> delegate3(obj2);  // Different object
        
        ASSERT_TRUE(delegate1 == delegate2);
        ASSERT_FALSE(delegate1 == delegate3);
    }

    TEST(DelegateTest, MethodPointerTemplateConversionTest)
    {
        auto obj = std::make_shared<TestObject>();
        
        MethodDelegate<&TestObject::setValue> methodDelegate(obj);
        
        // Test conversion to traditional delegate
        Delegate<void(int)> traditionalDelegate = methodDelegate;
        
        traditionalDelegate(99);
        ASSERT_EQ(obj->value, 99);
    }

    // Test that we can pass the new delegate type to functions expecting auto
    template<typename T>
    void callDelegate(T&& delegate)
    {
        delegate(555);
    }

    TEST(DelegateTest, MethodPointerTemplateGenericFunctionTest)
    {
        auto obj = std::make_shared<TestObject>();
        MethodDelegate<&TestObject::setValue> delegate(obj);
        
        callDelegate(delegate);
        ASSERT_EQ(obj->value, 555);
    }
}