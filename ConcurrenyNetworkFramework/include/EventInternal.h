#ifndef EVENTINTERNAL_H
#define EVENTINTERNAL_H


class EventInternal
{
    public:
        EventInternal();
        virtual ~EventInternal();

    protected:

    private:
        struct node
        {

            ClientBase *cli;
            std::atomic_flag isCalling;
        };
        std::map<int,node>
};

#endif // EVENTINTERNAL_H
