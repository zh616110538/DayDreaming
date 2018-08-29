#include <ClientBase.h>
#include <iostream>
#include <memory>
#include <memory.h>
using namespace std;


int main()
{
    ClientBase c;
    //unsigned char buf[30] = "Hello world";
//    unsigned char *buf = new unsigned char[30];
//    strcpy((char*)buf,"Hello world");
//    shared_ptr<unsigned char> ptr(buf);
//    c.pushread(ptr,12);
//    auto b = c.popread(12);
//    for(auto &i:b)
//        cout<<i;
//    cout<<endl;
//    cout<<buf<<endl;
    auto f = c.gettask();
    f.get();

}
