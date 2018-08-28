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


