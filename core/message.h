#ifndef _CORE_MESSAGE_H
#define _CORE_MESSAGE_H

#include <vector>

namespace core {
    
    class MessageHandler;

    class Message
    {
    friend class MessageHandler;
    public:
        typedef
            unsigned long int
            Type;

        Type  type;
        
        int   code;
        char *text;
        void *data;

        Message();
        ~Message();

    protected:
        MessageHandler *parent;
    };

    class MessageHandler
    {
    friend class Message;
    public:
        MessageHandler();
        ~MessageHandler();

        Message *
        send(Message *msg);

        Message *
        send(Message::Type type, int code = 0);

        Message *
        send(Message::Type type, const char *text);

        Message *
        send(Message::Type type, int code, void *data);

        Message *
        receive(Message::Type type);

        // void
        // listen(Message::Type type, *(void *)callback(void));

        void
        empty();

        bool
        is_empty();

    protected:
        std::vector<Message *> data;
    };
}

#endif
