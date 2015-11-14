#include "Symulator.h"
#include <cmath>

int RPiosson(double lambda) {
    double l = exp(-lambda);
    int k = 0;
    double p = 1.0;

    do {
        k++;
        p *= ((double) rand()) / RAND_MAX;
    } while (p > l);

    return k - 1;
}

int GetRandomInt(int min, int max) {
    return min + rand() % (max - min);
}

int pow(int base, int exponent) {
    int result = 1;

    for (int i = 0; i < exponent; i++) {
        result *= base;
    }

    return result;
}


void PacketWithHiddenData::CalculateDelay(unsigned int receiveTime) {
    for (auto& dataSource : dataSources) {
        if (!dataSource.hasMoreFragments)
            dataSource.basicMessage->CalculateDelay(receiveTime);
    }

    if (!hasMoreFragments)
        hiddenMessage->CalculateDelay(receiveTime);
}

std::string PacketWithHiddenData::GetDescription() {
    std::string result = "PacketWithHiddenData ";
    char hiddenPacketDesc[256];

    sprintf(hiddenPacketDesc, "%s segment: %d hasMore? %d datasources: [", hiddenMessage->GetId().c_str(),
            segment, hasMoreFragments);

    std::string dataSourcesDesc = "";

    for (auto& ds : dataSources) {
        char desc[256];
        sprintf(desc, "%s length: %d hasMore? %d | ", ds.basicMessage->GetId().c_str(), ds.length, ds.hasMoreFragments);
        dataSourcesDesc += desc;
    }

    return result + hiddenPacketDesc + dataSourcesDesc + "]";
}


unsigned int PacketWithoutHiddenData::GetFullSize() {
    return HEADER_SIZE + GetRealDataSize();
}

unsigned int PacketWithoutHiddenData::GetRealDataSize() {
    unsigned int sum = 0;

    for (auto& dataSource : dataSources) {
        sum += dataSource.length;
    }

    return sum;
}

void PacketWithoutHiddenData::CalculateDelay(unsigned int receiveTime) {
    for (auto& dataSource : dataSources) {
        if (!dataSource.hasMoreFragments)
            dataSource.basicMessage->CalculateDelay(receiveTime);
    }
}


std::string PacketWithoutHiddenData::GetDescription() {
    std::string result = "PacketWithoutHiddenData datasources: [";
    std::string dataSourcesDesc = "";

    for (auto& ds : dataSources) {
        char desc[256];
        sprintf(desc, "%s length: %d hasMore? %d | ", ds.basicMessage->GetId().c_str(), ds.length, ds.hasMoreFragments);
        dataSourcesDesc += desc;
    }

    return result + dataSourcesDesc + "]";
}


std::vector<PacketPtr> Channel::GetPacketsThatReachTarget() {
    std::vector<PacketPtr> result;
    unsigned int sentBytes = 0;

    while (sentBytes < bandwith && !packetsQueue.empty()) {
        PacketPtr packet = packetsQueue[0];

        sentBytes += packet->GetFullSize();
        result.push_back(packet);
        packetsQueue.erase(packetsQueue.begin());
    }

    return result;
}


std::vector<PacketPtr> HiddenChannel::Execute() {
    std::vector<PacketPtr> generatedPackets;

    while (CanSendHiddenPacket()) {
        HiddenMessageSegment segmentInfo = hiddenMessageQueue.GetSegmentToSend();
        DataSources dataSources = basicMessagesQueue.GetData(segmentInfo.segment);

        generatedPackets.push_back(std::make_shared<PacketWithHiddenData>(
            segmentInfo.hiddenMessage,
            segmentInfo.hasMoreFragments,
            segmentInfo.segment,
            dataSources));
    }

    unsigned int dataAviable = basicMessagesQueue.GetAviableDataLength();
    if (dataAviable >= pow(2, hiddenDataSegmentLength)) {
        DataSources dataSources = basicMessagesQueue.GetData(dataAviable);

        generatedPackets.push_back(std::make_shared<PacketWithoutHiddenData>(dataSources));
    }

    return generatedPackets;
}


bool HiddenChannel::CanSendHiddenPacket() {
     return !hiddenMessageQueue.IsEmpty() &&
        basicMessagesQueue.GetAviableDataLength() >= hiddenMessageQueue.PeekSegmentToSend();
}

bool HiddenChannel::HasEnoughBasicData() {
    return hiddenMessageQueue.GetDataAmountNeeded() <= basicMessagesQueue.GetAviableDataLength();
}

bool HiddenChannel::CanSendAllBasicData() {
    return !basicMessagesQueue.IsEmpty() &&
        basicMessagesQueue.GetAviableDataLength() >= pow(2, hiddenDataSegmentLength);
}

Symulator::Symulator(unsigned int timeForGeneratingHiddenMessages,
    unsigned int numberOfHiddenMessagesToGenerate,
    unsigned int timeForGeneratingBasicMessages,
    unsigned int hiddenDataSegmentLength,
    unsigned int channelBandwith,
    double hiddenDataAppearance,
    double basicDataAppearance,
    unsigned int basicDataLength,
    unsigned int hiddenMessageSegmentLength,
    unsigned int hiddenDataSegmentValue):
        timeForGeneratingHiddenMessages(timeForGeneratingHiddenMessages),
        numberOfHiddenMessagesToGenerate(numberOfHiddenMessagesToGenerate),
        timeForGeneratingBasicMessages(timeForGeneratingBasicMessages),
        hiddenDataSegmentLength(hiddenDataSegmentLength),
        channelBandwith(channelBandwith),
        hiddenDataAppearance(hiddenDataAppearance),
        basicDataAppearance(basicDataAppearance),
        basicDataLength(basicDataLength),
        hiddenMessageSegmentLength(hiddenMessageSegmentLength),
        hiddenDataSegmentValue(hiddenDataSegmentValue),
        currentTime(0),
        channel(channelBandwith),
        hiddenChannel(hiddenDataSegmentLength) { }

void Symulator::GenerateSymulationData() {
    int t = 0;

    while (t < timeForGeneratingHiddenMessages || allHiddenMessages.size() < numberOfHiddenMessagesToGenerate)
        GenerateHiddenMessages(t++);

    for (t = 0; t < timeForGeneratingBasicMessages; t++)
        GenerateBasicMessages(t);
}

void Symulator::GenerateHiddenMessages(int time) {
    int numberOfMessages = RPiosson(hiddenDataAppearance);

    for (int i = 0; i < numberOfMessages; i++) {
        int numberOfSegments = std::max(1, RPiosson(hiddenMessageSegmentLength));
        HiddenMessageSegments segments(numberOfSegments);

        for (int j = 0; j < numberOfSegments; j++) {
            int segmentValue = GetRandomInt(1, hiddenDataSegmentValue);
            segmentValue = std::min(segmentValue, pow(2, hiddenDataSegmentLength) - 1);
            segments[j] = segmentValue;
        }

        char id[256];
        sprintf(id, "u%d", allHiddenMessages.size());
        HiddenMessagePtr hiddenMessage = std::make_shared<HiddenMessage>(id, time, segments);

        hiddenMessages.push_back(hiddenMessage);
        allHiddenMessages.push_back(hiddenMessage);
    }
}

void Symulator::GenerateBasicMessages(int time) {
    int numberOfMessages = RPiosson(basicDataAppearance);

    for (int i = 0; i < numberOfMessages; i++) {
        unsigned int messageLength = GetRandomInt(1, basicDataLength);
        char id[256];
        sprintf(id, "p%d", allBasicMessages.size());

        BasicMessagePtr message = std::make_shared<BasicMessage>(id, time, messageLength);

        basicMessages.push_back(message);
        allBasicMessages.push_back(message);
    }
}

StepEvents Symulator::Step() {
    GenerateBasicMessages(currentTime);
    GenerateHiddenMessages(currentTime);
    std::vector<HiddenMessagePtr> hiddenMessagesArrived = AddArrivedHiddenMessages();

    std::vector<BasicMessagePtr> basicMessagesArrived = AddArrivedBasicMessages();
    std::vector<PacketPtr> packetsGenerated = hiddenChannel.Execute();

    for (auto& p : packetsGenerated) { channel.AddPacket(p); }

    std::vector<PacketPtr> packetsThatReachedTarget = channel.GetPacketsThatReachTarget();

    for (auto& p : packetsThatReachedTarget) { p->CalculateDelay(currentTime); }

    currentTime++;

    return StepEvents(basicMessagesArrived, hiddenMessagesArrived, packetsGenerated, packetsThatReachedTarget);
}

bool Symulator::HasSymulationEnd() {
    return HasAllMessagesBeenReceived(allHiddenMessages) &&
        HasAllMessagesBeenReceived(allBasicMessages);
}

std::vector<HiddenMessagePtr> Symulator::AddArrivedHiddenMessages() {
    std::vector<HiddenMessagePtr> arrivedMessages;

    while (!hiddenMessages.empty() && hiddenMessages[0]->GetAppearanceTime() == currentTime) {
        arrivedMessages.push_back(hiddenMessages[0]);
        hiddenChannel.AddMessage(hiddenMessages[0]);
        hiddenMessages.erase(hiddenMessages.begin());
    }

    return arrivedMessages;
}

std::vector<BasicMessagePtr> Symulator::AddArrivedBasicMessages() {
    std::vector<BasicMessagePtr> arrivedMessages;

    while (!basicMessages.empty() && basicMessages[0]->GetAppearanceTime() == currentTime) {
        arrivedMessages.push_back(basicMessages[0]);
        hiddenChannel.AddMessage(basicMessages[0]);
        basicMessages.erase(basicMessages.begin());
    }

    return arrivedMessages;
}

bool Symulator::NeedMoreDataForHiddenMessages() {
    return !HasAllMessagesBeenReceived(allHiddenMessages) &&
        HasAllMessagesAppeared(allBasicMessages, currentTime) &&
        !hiddenChannel.HasEnoughBasicData();
}

bool Symulator::NeedMoreDaraForBasicMessages() {
    bool p1 = !HasAllMessagesBeenReceived(allBasicMessages);
    bool p2 = HasAllMessagesAppeared(allBasicMessages, currentTime);
    bool p3 = !hiddenChannel.CanSendAllBasicData();

    return !HasAllMessagesBeenReceived(allBasicMessages) &&
        HasAllMessagesAppeared(allBasicMessages, currentTime) &&
        !hiddenChannel.CanSendAllBasicData();
}

void Symulator::AddBasicMessagesToSymulationIfNeeded() {
    if (NeedMoreDataForHiddenMessages() || NeedMoreDaraForBasicMessages())
        GenerateBasicMessages(currentTime);
}
