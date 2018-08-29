#include "ClientBase.h"
#include <iostream>

ClientBase::ClientBase()
{
    //ctor
}

ClientBase::~ClientBase()
{
    //dtor
}

void ClientBase::settype(int t)
{
    type = t;
}

void ClientBase::gettype()
{
    std::cout<<type<<std::endl;
}

int ClientBase::getfd()
{
    return fd;
}

std::future<void> ClientBase::gettask()
{
    std::future<void> f = std::async(&ClientBase::readCallBack,this);
    return std::move(f);
}

void ClientBase::pushread(std::shared_ptr<unsigned char> buf,size_t size)
{
    for(size_t i = 0;i<size;++i)
        readBuffer.push(buf.get()[i]);
}

void ClientBase::pushread(std::vector<unsigned char> &buf)
{
    for(auto it = buf.begin();it != buf.end();++it)
        readBuffer.push(*it);
}

std::vector<unsigned char> ClientBase::popread(size_t size)
{
    std::vector<unsigned char> v;
    size = (readBuffer.size() < size)?readBuffer.size():size;
    for(size_t i = 0;i<size;++i){
        v.push_back(readBuffer.front());
        readBuffer.pop();
    }
    return v;
}
