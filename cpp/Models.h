#ifndef MODELS_H

#define MODELS_H

#include <vector>
#include <memory>
#include <iostream>

class Message
{
    public:
        Message(std::string& id, unsigned int appearanceTime): receiveTime(0), delay(0),
            appearanceTime(appearanceTime), isReceived(false) {}

        void CalculateDelay(unsigned int receiveTime) { this->receiveTime = receiveTime; delay = receiveTime - appearanceTime; }
        bool IsReceived() { return isReceived; }

        unsigned int GetReceiveTime() { return receiveTime; }
        unsigned int GetDelay() { return delay; }
        unsigned int GetAppearanceTime() { return appearanceTime; }
        std::string GetId() { return id; }

    private:
        unsigned int receiveTime;
        unsigned int delay;
        unsigned int appearanceTime;
        bool isReceived;
        std::string id;
};


class BasicMessage : public Message
{
    public:
        BasicMessage(std::string id, unsigned int appearanceTime, unsigned int length): Message(id, appearanceTime), length(length) {}

        unsigned int  GetLength() { return length; }

    private:
        unsigned int length;
};

typedef std::shared_ptr<BasicMessage> BasicMessagePtr;


class QueuedBasicMessage
{
    public:
        QueuedBasicMessage(BasicMessagePtr basicMessage) : basicMessage(basicMessage), alreadyTakenLength(0) {}

        unsigned int GetReamingDataToSend() { return basicMessage->GetLength() - alreadyTakenLength; }
        void GetData(unsigned int amount) { alreadyTakenLength += amount; }
        BasicMessagePtr GetBasicMessage() { return basicMessage; }

    private:
        BasicMessagePtr basicMessage;
        unsigned int alreadyTakenLength;
};

typedef std::shared_ptr<QueuedBasicMessage> QueuedBasicMessagePtr;

struct DataSource
{
    DataSource(BasicMessagePtr basicMessage, unsigned int length,
            bool hasMoreFragments): basicMessage(basicMessage), length(length),
            hasMoreFragments(hasMoreFragments) {}

    BasicMessagePtr basicMessage;
    unsigned int length;
    bool hasMoreFragments;
};

typedef std::vector<DataSource> DataSources;

std::ostream& operator<<(std::ostream& os, DataSources& queue);

class BasicMessagesQueue
{
    friend std::ostream& operator<<(std::ostream& os, BasicMessagesQueue& queue);

    public:
        unsigned int GetAviableDataLength();

        std::vector<DataSource> GetData(unsigned int amount);
        void AddMessage(BasicMessagePtr message) { messages.push_back(std::make_shared<QueuedBasicMessage>(message)); }

    private:
        std::vector<QueuedBasicMessagePtr> messages;
};

std::ostream& operator<<(std::ostream& os, BasicMessagesQueue& queue);

#endif /* end of include guard: MODELS_H */
