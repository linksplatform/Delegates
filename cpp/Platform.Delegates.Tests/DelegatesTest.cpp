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
}