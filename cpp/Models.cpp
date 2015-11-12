#include "Models.h"

unsigned int BasicMessagesQueue::GetAviableDataLength() {
    unsigned int sum = 0;

    for (auto& m : messages) {
        sum += m->GetReamingDataToSend();
    }

    return sum;
}


DataSources BasicMessagesQueue::GetData(unsigned int amount) {
    DataSources result;

    while (amount > 0) {
        QueuedBasicMessagePtr messageUsed = messages.front();
        unsigned int aviableDataFromMessage = messageUsed->GetReamingDataToSend();

        if (amount >= aviableDataFromMessage) {
            messages.erase(messages.begin());

            result.emplace_back(messageUsed->GetBasicMessage(), aviableDataFromMessage, false);
            amount -= aviableDataFromMessage;
        }
        else {
            result.emplace_back(messageUsed->GetBasicMessage(), amount, true);
            messageUsed->GetData(amount);
            amount = 0;
        }
    }

    return result;
}

std::ostream& operator<<(std::ostream& os, BasicMessagesQueue& queue) {
    for (auto& m : queue.messages) {
        BasicMessagePtr bm = m->GetBasicMessage();

        os << bm->GetId() << " " << m->GetReamingDataToSend() << " / " << bm->GetLength() << std::endl;
    }

    return os;
}

std::ostream& operator<<(std::ostream& os, DataSources& dataSources) {
    for (auto& ds : dataSources) {
        os << ds.basicMessage->GetId() << ": " << ds.length <<
            " hasMoreFragments? " << ds.hasMoreFragments << std::endl;
    }

    return os;
}
