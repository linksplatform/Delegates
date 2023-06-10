#include <gtest/gtest.h>
#include <Platform.Delegates.h>

using namespace Platform::Delegates;

namespace Platform::Delegates::Tests::MulticastDelegatesTest
{
    int globalA = 0;

    void increaseGlobalA()
    {
        globalA += 5;
    }

    void decreaseGlobalA()
    {
        globalA -= 5;
    }

    class SampleClass
    {
    public:
        void increaseGlobalA2()
        {
            globalA += 10;
        }
    };

    TEST(MulticastDelegateTest, AddRemoveTest)
    {
        globalA = 0;
        Platform::Delegates::MulticastDelegate<void()> multicastDelegate;
        
        // Adding the same delegate twice.
        multicastDelegate += increaseGlobalA;
        multicastDelegate += increaseGlobalA;
        
        // Removing the delegate once.
        multicastDelegate -= increaseGlobalA;
        
        // Now, multicastDelegate still has one instance of the delegate left.
        EXPECT_NO_THROW(multicastDelegate());
    }

    TEST(MulticastDelegateTest, InvokeTest)
    {
        globalA = 0;
        Platform::Delegates::MulticastDelegate<void()> multicastDelegate;
        multicastDelegate += increaseGlobalA;
        multicastDelegate += decreaseGlobalA;
        multicastDelegate();
        EXPECT_EQ(globalA, 0);
    }

    TEST(MulticastDelegateTest, CopyConstructorTest)
    {
        globalA = 0;
        Platform::Delegates::MulticastDelegate<void()> multicastDelegateOriginal;
        multicastDelegateOriginal += increaseGlobalA;
        Platform::Delegates::MulticastDelegate<void()> multicastDelegateCopy(multicastDelegateOriginal);
        multicastDelegateCopy();
        EXPECT_EQ(globalA, 5);
    }

    TEST(MulticastDelegateTest, MoveConstructorTest)
    {
        globalA = 0;
        Platform::Delegates::MulticastDelegate<void()> multicastDelegateOriginal;
        multicastDelegateOriginal += increaseGlobalA;
        Platform::Delegates::MulticastDelegate<void()> multicastDelegateMove(std::move(multicastDelegateOriginal));
        multicastDelegateMove();
        EXPECT_EQ(globalA, 5);
    }

    TEST(MulticastDelegateTest, CopyAssignmentOperatorTest)
    {
        globalA = 0;
        Platform::Delegates::MulticastDelegate<void()> multicastDelegateOriginal;
        multicastDelegateOriginal += increaseGlobalA;
        Platform::Delegates::MulticastDelegate<void()> multicastDelegateAssign;
        multicastDelegateAssign = multicastDelegateOriginal;
        multicastDelegateAssign();
        EXPECT_EQ(globalA, 5);
    }

    TEST(MulticastDelegateTest, MoveAssignmentOperatorTest)
    {
        globalA = 0;
        Platform::Delegates::MulticastDelegate<void()> multicastDelegateOriginal;
        multicastDelegateOriginal += increaseGlobalA;
        Platform::Delegates::MulticastDelegate<void()> multicastDelegateAssign;
        multicastDelegateAssign = std::move(multicastDelegateOriginal);
        multicastDelegateAssign();
        EXPECT_EQ(globalA, 5);
    }

    TEST(MulticastDelegateTest, AddRemoveMemberFunctionTest)
    {
        globalA = 0;
        SampleClass sampleClass;
        Platform::Delegates::MulticastDelegate<void()> multicastDelegate;
        
        auto memberFunc = [&sampleClass]() { sampleClass.increaseGlobalA2(); };
        multicastDelegate += memberFunc;
        multicastDelegate -= memberFunc;
        
        EXPECT_NO_THROW(multicastDelegate());
    }
}