#include <iostream>
#include <memory>
#include "../cpp/Platform.Delegates/Platform.Delegates.h"

using namespace Platform::Delegates;

struct base
{
    void foo() { std::cout << "foo"; }
};

struct derived : public base {};

int main()
{
    auto object = std::make_shared<derived>();
    
    std::cout << "Testing with explicit cast (should work):\n";
    try {
        auto delegate1 = Delegate((std::shared_ptr<base>)object, &derived::foo);
        delegate1();
        std::cout << " - SUCCESS\n";
    } catch (const std::exception& e) {
        std::cout << " - FAILED: " << e.what() << "\n";
    }
    
    std::cout << "Testing without cast (should fail currently):\n";
    try {
        auto delegate2 = Delegate(object, &derived::foo);
        delegate2();
        std::cout << " - SUCCESS\n";
    } catch (const std::exception& e) {
        std::cout << " - FAILED: " << e.what() << "\n";
    }
    
    return 0;
}