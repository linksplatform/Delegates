// Based on https://stackoverflow.com/a/23974539/710069

#include "Platform.Delegates.h"
using namespace Platform::Delegates;

// ----------------------------------------------------------------------------

void f(char const* str) { std::cout << "f(" << str << ")\n"; }
void g(char const* str) { std::cout << "g(" << str << ")\n"; }
void h(char const* str) { std::cout << "h(" << str << ")\n"; }

// ----------------------------------------------------------------------------

struct foo
{
    int d_id;
    int bar_calls;
    int cbs_calls;
    explicit foo(int id) : d_id(id), bar_calls(0), cbs_calls(0) {}
    void bar(char const* str) {
        std::cout << "foo(" << this->d_id << ")::bar(" << str << ")\n";
        this->bar_calls++;
    }
    void cbs(char const* str) {
        std::cout << "foo(" << this->d_id << ")::cbs(" << str << ")\n";
        this->cbs_calls++;
    }
};

// ----------------------------------------------------------------------------

void TestBindComparisonWithObjectPassedByValue()
{
    MulticastDelegate<void(char const*)> d0 = f;

    foo f1(1);

    d0 += std::bind(&foo::bar, f1, std::placeholders::_1);
    d0("first call");
    d0 -= std::bind(&foo::bar, f1, std::placeholders::_1);
    d0("second call");
}

void TestBindComparisonWithObjectPassedByReference()
{
    MulticastDelegate<void(char const*)> d0 = f;

    foo f1(1);

    d0 += std::bind(&foo::bar, &f1, std::placeholders::_1);
    d0("first call");
    d0 -= std::bind(&foo::bar, &f1, std::placeholders::_1);
    d0("second call");
}

int main()
{
    /*TestBindComparisonWithObjectPassedByValue();
    TestBindComparisonWithObjectPassedByReference();*/

    MulticastDelegate<void(char const*)> d0 = f;

    d0 = std::move(d0);
    d0 = f; // value is reset here

    std::shared_ptr<foo> f0 = std::make_shared<foo>(0);
    foo f1(1);

    d0 += g;
    d0 += g;
    d0 += h;

    d0 += Delegate<>::CreateDelegate(f0, &foo::bar);
    d0 += Delegate<>::CreateDelegate(f0, &foo::cbs);
    d0 += std::bind(&foo::bar, &f1, std::placeholders::_1);
    d0 += std::bind(&foo::cbs, &f1, std::placeholders::_1);
    d0("first call");
    d0 -= g;
    d0 -= Delegate<>::CreateDelegate(f0, &foo::cbs);
    d0 -= std::bind(&foo::bar, &f1, std::placeholders::_1);
    d0("second call");

    // Delegate<>::CreateDelegate((std::function<void(char const*)>)std::bind(&foo::bar, f0.get(), std::placeholders::_1));
}


// Test case for Platform.Exceptions project
class IgnoredExceptions
{
    public: static void RaiseExceptionIgnoredEvent(const std::exception& exception) { ExceptionIgnored(NULL, exception); }

    private: static void OnExceptionIgnored(void* sender, const std::exception& exception)
    {
    }

    public: static inline MulticastDelegate<void(void*, const std::exception&)> ExceptionIgnored = OnExceptionIgnored;
};