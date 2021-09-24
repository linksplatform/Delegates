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
    d0 -= std::bind(&foo::bar, f1, std::placeholders::_1);
    d0("second call");
}

void TestBindComparisonWithObjectPassedByReference()
{
    MulticastDelegate<void(const char *)> d0 = f;

    foo f1(1);

    d0 += std::bind(&foo::bar, &f1, std::placeholders::_1);
    d0("first call");
    d0 -= std::bind(&foo::bar, &f1, std::placeholders::_1);
    d0("second call");
}


template<class _Ret, class _Fx, class... _Types>
class bind_args
{
private:
    using _Seq = std::index_sequence_for<_Types...>;
    using _First = std::decay_t<_Fx>;
    using _Second = std::tuple<std::decay_t<_Types>...>;

    const std::_Compressed_pair<_First, _Second>& _Mypair;

public:
    bind_args(const std::_Binder<_Ret, _Fx, _Types...>& binder)
        : _Mypair(*reinterpret_cast<const std::_Compressed_pair<_First, _Second>*>(&binder))
    {
        std::cout << "sizeof(binder) = " << sizeof(binder) << std::endl;
        std::cout << "std::tuple_size<_Second> = " << std::tuple_size<_Second>{} << std::endl;
    }

    template<std::size_t I>
    void printTypeAtIndex()
    {
        std::cout << "index " << I << " has type: ";
        using SelectedType = std::tuple_element_t<I, _Second>;
        std::cout << typeid(SelectedType).name();
        std::cout << " sizeof = " << sizeof(SelectedType) << std::endl;
    }

    template<std::size_t I>
    void printValueAtIndex()
    {
        using SelectedType = std::tuple_element_t<I, _Second>;
        const SelectedType& el = get<I>();
        std::uint8_t   data[sizeof(SelectedType)];
        memcpy_s(data, sizeof(SelectedType), reinterpret_cast<const void*>(&get<I>()), sizeof(SelectedType));
        std::cout << "index " << I << " has value: ";
        for (auto i = 0; i < sizeof(data); i++)
           std::cout << std::to_string(data[i]) << " ";
        std::cout << std::endl;
    }

    template<std::size_t I>
    auto& get()
    {
        return std::get<I>(_Mypair._Myval2);
    }

    auto& get_tuple()
    {
        return _Mypair._Myval2;
    }
};


int main()
{
    MulticastDelegate<void(const char *)> d0 = f;
    MulticastDelegate<void(const char *)> d1 = f;

    std::function<void(const char *)> fx = f;

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

    d0 += g;
    d0 += g;
    d0 += h;

    d0 += Delegate(f0, &foo::bar);
    d0 += Delegate(f0, &foo::cbs);
    
    auto b1 = std::bind(&foo::bar, &f1, std::placeholders::_1);
    auto b2 = std::bind(&foo::cbs, &f1, std::placeholders::_1);

    std::cout << "sizeof(b1) = " << sizeof(b1) << std::endl;
    std::cout << "sizeof(b2) = " << sizeof(b2) << std::endl;

    auto ba1 = bind_args(b1);
    ba1.printTypeAtIndex<0>();
    ba1.printValueAtIndex<0>();
    ba1.printTypeAtIndex<1>();
    ba1.printValueAtIndex<1>();

    auto ba2 = bind_args(b2);
    ba2.printTypeAtIndex<0>();
    ba2.printValueAtIndex<0>();
    ba2.printTypeAtIndex<1>();
    ba2.printValueAtIndex<1>();

    d0 += b1;
    d0 += b2;
    d0("first call");
    d0 -= g;
    d0 -= Delegate(f0, &foo::cbs);
    d0 -= std::bind(&foo::bar, &f1, std::placeholders::_1);
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