#ifndef CLIENTBASE_H
#define CLIENTBASE_H

#include <queue>
#include <memory>
#include <vector>
#include <iostream>
#include <future>
class ClientBase
{
    public:
        friend class EventInternal;
        static const int ev_read = 0x01;
        static const int ev_write = 0x02;
        static const int ev_error = 0x04;
        ClientBase();
        virtual ~ClientBase();
        virtual void readCallBack(){}
        //virtual void writeCallBack(){}
        virtual void errorCallBack(){}
        void settype(int);
        void gettype();
        int getfd();
        std::future<void> gettask();
        void pushread(std::shared_ptr<unsigned char>,size_t);
        void pushread(std::vector<unsigned char> &);
        std::vector<unsigned char> popread(size_t);
    protected:
    private:
        int type;
        int fd;
        std::queue<unsigned char> readBuffer;
        //std::queue<unsigned char> writeBuffer;
        /*
        void pushread(std::shared_ptr<unsigned char>,size_t);
        void pushread(std::vector<unsigned char>);
        std::vector<unsigned char> popread(size_t);
        */
        //void pushwrite(std::shared_ptr<unsigned char>);
        //void popwrite(std::shared_ptr<unsigned char>);
};

#endif // CLIENTBASE_H
