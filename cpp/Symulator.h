#ifndef SYMULATOR_H

#define SYMULATOR_H

#include "Models.h"

const unsigned int HEADER_SIZE = 30;

template <typename T>
bool HasAllMessagesBeenReceived(std::vector<T>& messages)
{
    for (int i = messages.size() - 1; i >= 0; i--)
        if (!messages[i]->IsReceived())
            return false;

    return true;
}

template <typename T>
bool HasAllMessagesAppeared(std::vector<T>& messages, unsigned int currentTime)
{
    bool result = true;

    for (auto& m : messages) {
        result &= m->GetAppearanceTime() < currentTime;
    }

    return result;
}

template <typename T>
double CalculateAverageDelay(std::vector<T> messages) {
    double sum = 0;

    int numberOfReceivedMessages = 0;

    while (numberOfReceivedMessages < messages.size() && messages[numberOfReceivedMessages]->IsReceived())
        sum += messages[numberOfReceivedMessages++]->GetDelay();

    return sum / numberOfReceivedMessages;
}

class Packet
{
public:
    virtual ~Packet() {}

    virtual unsigned int GetFullSize() = 0;
    virtual unsigned int GetRealDataSize() = 0;
    virtual void CalculateDelay(unsigned int receiveTime) = 0;
    virtual bool HasHiddenData() = 0;
    virtual std::string GetDescription() = 0;

private:

};

typedef std::shared_ptr<Packet> PacketPtr;

class PacketWithHiddenData : public Packet
{
public:
    PacketWithHiddenData(HiddenMessagePtr hiddenMessage, bool hasMoreFragments,
        unsigned int segment, DataSources dataSources) : hiddenMessage(hiddenMessage),
        hasMoreFragments(hasMoreFragments), segment(segment), dataSources(dataSources) {}
    virtual ~PacketWithHiddenData() {}

    unsigned int GetFullSize() override { return segment + HEADER_SIZE; }
    unsigned int GetRealDataSize() override { return segment; }
    void CalculateDelay(unsigned int receiveTime) override;
    bool HasHiddenData() override {return true; }
    std::string GetDescription() override;

private:
    HiddenMessagePtr hiddenMessage;
    bool hasMoreFragments;
    unsigned int segment;
    DataSources dataSources;
};

typedef std::shared_ptr<PacketWithHiddenData> PacketWithHiddenDataPtr;

class PacketWithoutHiddenData : public Packet
{
public:
    PacketWithoutHiddenData(DataSources dataSources) : dataSources(dataSources) {}
    virtual ~PacketWithoutHiddenData() {};

    unsigned int GetFullSize() override;
    unsigned int GetRealDataSize() override;
    void CalculateDelay(unsigned int receiveTime) override;
    bool HasHiddenData() override {return false; }
    std::string GetDescription() override;

private:
    DataSources dataSources;
};

typedef std::shared_ptr<PacketWithoutHiddenData> PacketWithoutHiddenDataPtr;


class Channel
{
public:
    Channel(unsigned int bandwith) : bandwith(bandwith) {}

    void AddPacket(PacketPtr packet) { packetsQueue.push_back(packet); }
    std::vector<PacketPtr> GetPacketsThatReachTarget();

private:
    unsigned int bandwith;
    std::vector<PacketPtr> packetsQueue;
};


class HiddenChannel
{
public:
    HiddenChannel(unsigned int hiddenDataSegmentLength): hiddenDataSegmentLength(hiddenDataSegmentLength) {}

    void AddMessage(BasicMessagePtr message) { basicMessagesQueue.AddMessage(message); }
    void AddMessage(HiddenMessagePtr message) { hiddenMessageQueue.AddMessage(message); }
    bool HasEnoughBasicData();
    bool CanSendAllBasicData();

    std::vector<PacketPtr> Execute();

private:
    unsigned int hiddenDataSegmentLength;
    HiddenMessageQueue hiddenMessageQueue;
    BasicMessagesQueue basicMessagesQueue;

    bool CanSendHiddenPacket();
};

struct StepEvents {
    StepEvents(std::vector<BasicMessagePtr> arrivedBasicMessages1,
        std::vector<HiddenMessagePtr> arrivedHiddenMessages,
        std::vector<PacketPtr> packetsGenerated,
        std::vector<PacketPtr> packetsThatReachedTarget):
        arrivedBasicMessages(arrivedBasicMessages1),
        arrivedHiddenMessages(arrivedHiddenMessages),
        packetsGenerated(packetsGenerated),
        packetsThatReachedTarget(packetsThatReachedTarget) { }

    std::vector<BasicMessagePtr> arrivedBasicMessages;
    std::vector<HiddenMessagePtr> arrivedHiddenMessages;
    std::vector<PacketPtr> packetsGenerated;
    std::vector<PacketPtr> packetsThatReachedTarget;

};

class Symulator
{
public:
    Symulator(unsigned int timeForGeneratingHiddenMessages, unsigned int numberOfHiddenMessagesToGenerate,
            unsigned int timeForGeneratingBasicMessages, unsigned int hiddenDataSegmentLength,
            unsigned int channelBandwith, double hiddenDataAppearance,
            double basicDataAppearance, unsigned int basicDataLength, unsigned int hiddenMessageSegmentLength,
            unsigned int hiddenDataSegmentValue);

    void SetHiddenMessages(std::vector<HiddenMessagePtr> hiddenMessages) { this->hiddenMessages = allHiddenMessages = hiddenMessages; }
    void SetBasicMessages(std::vector<BasicMessagePtr> basicMessages) { this->basicMessages = allBasicMessages = basicMessages; }
    void GenerateSymulationData();
    StepEvents Step();
    bool HasSymulationEnd();
    std::vector<HiddenMessagePtr> GetAllHiddenMessages() { return allHiddenMessages; }
    std::vector<BasicMessagePtr> GetAllBasicMessages() { return allBasicMessages; }
    double GetHiddenMessagesDelay() { return CalculateAverageDelay(allHiddenMessages); }
    double GetBasicMessagesDelay() { return CalculateAverageDelay(allBasicMessages); }
    unsigned int GetCurrentTime() { return currentTime; }

private:
    unsigned int timeForGeneratingHiddenMessages;
    unsigned int numberOfHiddenMessagesToGenerate;
    unsigned int timeForGeneratingBasicMessages;
    unsigned int hiddenDataSegmentLength;
    unsigned int channelBandwith;
    double hiddenDataAppearance;
    double basicDataAppearance;
    unsigned int basicDataLength;
    unsigned int hiddenMessageSegmentLength;
    unsigned int hiddenDataSegmentValue;
    unsigned int currentTime;

    std::vector<BasicMessagePtr> basicMessages;
    std::vector<BasicMessagePtr> allBasicMessages;

    std::vector<HiddenMessagePtr> hiddenMessages;
    std::vector<HiddenMessagePtr> allHiddenMessages;

    Channel channel;
    HiddenChannel hiddenChannel;

    void GenerateHiddenMessages(int time);
    void GenerateBasicMessages(int time);
    std::vector<HiddenMessagePtr> AddArrivedHiddenMessages();
    std::vector<BasicMessagePtr> AddArrivedBasicMessages();
    bool NeedMoreDataForHiddenMessages();
    bool NeedMoreDaraForBasicMessages();
    void AddBasicMessagesToSymulationIfNeeded();
};

#endif /* end of include guard: SYMULATOR_H */
