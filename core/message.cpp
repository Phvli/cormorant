#include "message.h"

#include "util/string.h"

#include <cstdlib>

using namespace core;

Message::Message()
{
    this->data   = NULL;
    this->text   = NULL;
    this->code   = 0;
    this->parent = NULL;
}

Message::~Message()
{
    delete[] this->text;

    if (this->parent != NULL)
    {
        // remove from parent's bookkeeping
        for (std::vector<Message *>::iterator i = this->parent->data.begin();
        i != this->parent->data.end(); ++i)
        {
            if (*i == this)
            {
                this->parent->data.erase(i);
                break;
            }
        }
        this->parent = NULL;
    }
}

MessageHandler::MessageHandler()
{
}

MessageHandler::~MessageHandler()
{
    for (std::vector<Message *>::iterator i = this->data.begin();
    i != this->data.end(); ++i)
    {
        delete[] *i;
    }
}

void
MessageHandler::empty()
{
    this->data.clear();
}

bool
MessageHandler::is_empty()
{
    return (this->data.size() == 0);
}

Message *
MessageHandler::send(Message *msg)
{
    this->data.push_back(msg);
    return msg;
}

Message *
MessageHandler::send(Message::Type type, int code, void *data)
{
    Message *msg = new Message();
    msg->type = type;
    msg->code = code;
    msg->data = data;

    return this->send(msg);
}

Message *
MessageHandler::send(Message::Type type, const char *text)
{
    Message *msg = new Message();
    msg->type = type;
    msg->text = str::dup(text);

    return this->send(msg);
}
