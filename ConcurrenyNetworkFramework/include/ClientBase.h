#ifndef CLIENTBASE_H
#define CLIENTBASE_H


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
        virtual void writeCallBack(){}
        virtual void errorCallBack(){}
        void settype(int);
        void gettype();
    protected:

    private:
        int type;
        std::queue<u_char> readBuffer;
        std::queue<u_char> writeBuffer;
        void pushread(std::shared_ptr<u_char> );
        void popread(std::shared_ptr<u_char>);
        void pushwrite(std::shared_ptr<u_char>);
        void popwrite(std::shared_ptr<u_char>);
};

#endif // CLIENTBASE_H
