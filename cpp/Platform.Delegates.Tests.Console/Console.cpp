// Based on https://stackoverflow.com/a/23974539/710069

#include <Platform.Delegates.h>
using namespace Platform::Delegates;

// ----------------------------------------------------------------------------

void f(const char *str) { std::cout << "f(" << str << ")\n"; }
void g(const char *str) { std::cout << "g(" << str << ")\n"; }
void h(const char *str) { std::cout << "h(" << str << ")\n"; }

// ----------------------------------------------------------------------------

struct foo
{
    int d_id;
    int bar_calls;
    int cbs_calls;
    explicit foo(int id) : d_id(id), bar_calls(0), cbs_calls(0) {}
    void bar(const char *str) {
        std::cout << "foo(" << this->d_id << ")::bar(" << str << ")\n";
        this->bar_calls++;
    }
    void cbs(const char *str) {
        std::cout << "foo(" << this->d_id << ")::cbs(" << str << ")\n";
        this->cbs_calls++;
    }
};

// ----------------------------------------------------------------------------

void TestBindComparisonWithObjectPassedByValue()
{
    MulticastDelegate<void(const char *)> d0 = f;

    foo f1(1);

    d0 += std::bind(&foo::bar, f1, std::placeholders::_1);
    d0("first call");
    // d0 -= std::bind(&foo::bar, f1, std::placeholders::_1);
    // d0("second call");
}

void TestBindComparisonWithObjectPassedByReference()
{
    MulticastDelegate<void(const char *)> d0 = f;

    foo f1(1);

    d0 += std::bind(&foo::bar, &f1, std::placeholders::_1);
    d0("first call");
    // d0 -= std::bind(&foo::bar, &f1, std::placeholders::_1);
    // d0("second call");
}

int main()
{
    MulticastDelegate<void(const char *)> d0 = f;
    MulticastDelegate<void(const char *)> d1 = f;

    std::function<void(const char *)> fx = f;

    auto lambda = std::make_shared<std::function<void(const char *)>>([](const char *str) 
    { 
        std::cout << "lambda(" << str << ")" << std::endl;
    });

    d0 = d0 = d0;
    d0 = d1;
    d1 = d0;
    d0 = std::move(d1);
    d0 = std::move(f);
    d0 = fx;
    d0 = [&](auto string) {};
    d0 = f; // value is reset here

    std::shared_ptr<foo> f0 = std::make_shared<foo>(0);
    foo f1(1);

    Delegate<void(const char *)> memberMethod(std::make_shared<foo>(0), &foo::bar);

    d0 += g;
    d0 += g;
    d0 += h;
    d0 += lambda;
    // d0 += std::function<void(const char *)>(f);

    std::function<void(const char *)> savedFunction = std::bind(&foo::bar, &f1, std::placeholders::_1);
    std::shared_ptr<std::function<void(const char *)>> pointerToSavedFunction = std::make_shared<std::function<void(const char *)>>(std::move(savedFunction));

    // d0 += Delegate(f0, &foo::bar);
    d0 += memberMethod;
    d0 += { f0, &foo::cbs };
    d0 += pointerToSavedFunction;
    d0 += std::bind(&foo::cbs, &f1, std::placeholders::_1);
    d0("first call");
    d0 -= g;
    d0 -= { f0, &foo::cbs };
    d0 -= pointerToSavedFunction;
    d0 -= lambda;
    
    d0("second call");
}


// Test case for Platform.Exceptions project
class IgnoredExceptions
{
public: static void RaiseExceptionIgnoredEvent(const std::exception &exception) { ExceptionIgnored(NULL, exception); }

private: static void OnExceptionIgnored(void *sender, const std::exception &exception)
{
}

public: static inline MulticastDelegate<void(void *, const std::exception &)> ExceptionIgnored = OnExceptionIgnored;
};