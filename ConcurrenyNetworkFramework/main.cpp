#include <ClientBase.h>
#include <iostream>

using namespace std;

class A
{
private:

    B x;
public:
    int a;
    int b;
    class B
    {
        void p()
        {
            cout<<a<<b<<endl;
        }
    };
    void test()
    {
        x.p();
    }
};

int main()
{
    A a;
    a.test();
}
