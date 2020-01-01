// Based on https://stackoverflow.com/a/23974539/710069

#include "Delegate.h"
using namespace Platform::Delegates;

// ----------------------------------------------------------------------------

void f(char const* str) { std::cout << "f(" << str << ")\n"; }
void g(char const* str) { std::cout << "g(" << str << ")\n"; }
void h(char const* str) { std::cout << "h(" << str << ")\n"; }

// ----------------------------------------------------------------------------

struct foo
{
    int d_id;
    explicit foo(int id) : d_id(id) {}
    void bar(char const* str) {
        std::cout << "foo(" << this->d_id << ")::bar(" << str << ")\n";
    }
    void cbs(char const* str) {
        std::cout << "foo(" << this->d_id << ")::cbs(" << str << ")\n";
    }
};

// ----------------------------------------------------------------------------

int main()
{
    Delegate<void(char const*)> d0;

    foo f0(0);
    foo f1(1);

    d0 += f;
    d0 += g;
    d0 += g;
    d0 += h;

    // d0 += std::bind(&foo::bar, f0, std::placeholders::_1);
    d0 += mem_call(f0, &foo::bar);
    d0 += mem_call(f0, &foo::cbs);
    d0 += mem_call(f1, &foo::bar);
    d0 += mem_call(f1, &foo::cbs);
    d0("first call");
    d0 -= g;
    d0 -= mem_call(f0, &foo::cbs);
    d0 -= mem_call(f1, &foo::bar);
    d0("second call");
}