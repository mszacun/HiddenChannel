#include <iostream>
#include <cassert>

#include "Models.h"
#include "Symulator.h"

void ShouldSendPacketWithOnlyBasicData();
void ShouldSendPacketWithHiddenDataAndNextOnlyBasicData();
void ShouldGenerateRandomData();
std::string StepToString(StepEvents events);

int main(int argc, const char *argv[])
{
    BasicMessagePtr m1 = std::make_shared<BasicMessage>("m1", 3, 120);
    BasicMessagePtr m2 = std::make_shared<BasicMessage>("m2", 3, 250);

    BasicMessagesQueue queue;

    queue.AddMessage(m1);
    queue.AddMessage(m2);

    std::cout << "Ilosc danych: " << queue.GetAviableDataLength() << std::endl;
    std::cout << queue << std::endl;

    auto dataSources = queue.GetData(150);
    std::cout << dataSources << std::endl;
    std::cout << "Ilosc danych: " << queue.GetAviableDataLength() << std::endl;
    std::cout << queue << std::endl;

    auto dataSources2 = queue.GetData(220);
    std::cout << dataSources2 << std::endl;
    std::cout << "Ilosc danych: " << queue.GetAviableDataLength() << std::endl;
    std::cout << queue << std::endl;


    // test HiddenMessage
    HiddenMessageSegments segments {20, 120, 30};
    HiddenMessageSegments segments2 {200, 120, 10};
    HiddenMessage hm("h1", 2, segments);
    HiddenMessage hm2("h2", 2, segments2);

    assert(hm.GetDataAmountNeeded() == 170);
    assert(hm2.GetDataAmountNeeded() == 330);
    assert(hm2.GetNextSegmentToSend() == 200);
    assert(hm2.GetNextSegmentToSend() == 120);
    assert(hm2.GetNextSegmentToSend() == 10);
    assert(hm2.HasMoreSegmentsToSend() == false);
    assert(hm.HasMoreSegmentsToSend() == true);
    assert(hm.PeekNextSegmentToSend() == 20);
    assert(hm.PeekNextSegmentToSend() == 20);

    // test HiddenMessageQueue
    HiddenMessageSegments segments3 {35, 21};
    HiddenMessageSegments segments4 {120, 230, 12};
    HiddenMessagePtr hm3 = std::make_shared<HiddenMessage>("h3", 1, segments3);
    HiddenMessagePtr hm4 = std::make_shared<HiddenMessage>("h4", 1, segments4);
    HiddenMessageQueue hiddenQueue;

    hiddenQueue.AddMessage(hm3);
    hiddenQueue.AddMessage(hm4);

    assert(hiddenQueue.GetDataAmountNeeded() == 418);
    assert(hiddenQueue.PeekSegmentToSend() == 35);

    HiddenMessageSegment segment1 = hiddenQueue.GetSegmentToSend();
    assert(segment1.segment == 35);
    assert(segment1.hasMoreFragments);
    assert(hiddenQueue.GetDataAmountNeeded() == 383);

    HiddenMessageSegment segment2 = hiddenQueue.GetSegmentToSend();
    assert(segment2.segment == 21);
    assert(!segment2.hasMoreFragments);

    HiddenMessageSegment segment3 = hiddenQueue.GetSegmentToSend();
    assert(segment3.segment == 120);
    assert(segment3.hasMoreFragments);
    assert(hiddenQueue.GetDataAmountNeeded() == 242);

    ShouldSendPacketWithOnlyBasicData();
    ShouldSendPacketWithHiddenDataAndNextOnlyBasicData();
    ShouldGenerateRandomData();

    return 0;
}

void ShouldSendPacketWithOnlyBasicData() {
    BasicMessagePtr m1 = std::make_shared<BasicMessage>("b1", 2, 120);
    BasicMessagePtr m2 = std::make_shared<BasicMessage>("b2", 2, 220);

    Symulator sym(0, // timeForGeneratingHiddenMessages
            0, // numberOfHiddenMessagesToGenerate
            0, // timeForGeneratingBasicMessages
            8, // hiddenDataSegmentLength
            1000, // channelBandwith
            2, // hiddenDataAppearance
            3, // basicDataAppearance
            220, // basicDataLength
            2, // hiddenMessageSegmentLength
            50); // hiddenDataSegmentValue

    std::vector<BasicMessagePtr> bms { m1, m2 };
    sym.SetBasicMessages(bms);

    assert(!sym.HasSymulationEnd());
    assert(StepToString(sym.Step()) == " #  #  # ");
    assert(!sym.HasSymulationEnd());
    assert(StepToString(sym.Step()) == " #  #  # ");
    assert(!sym.HasSymulationEnd());
    assert(StepToString(sym.Step()) == "b1 b2  #  # PacketWithoutHiddenData datasources: [b1 length: 120 hasMore? 0 | b2 length: 220 hasMore? 0 | ]  # PacketWithoutHiddenData datasources: [b1 length: 120 hasMore? 0 | b2 length: 220 hasMore? 0 | ] ");
    assert(sym.HasSymulationEnd());
}

void ShouldSendPacketWithHiddenDataAndNextOnlyBasicData() {
    BasicMessagePtr m1 = std::make_shared<BasicMessage>("b1", 2, 120);
    BasicMessagePtr m2 = std::make_shared<BasicMessage>("b2", 2, 220);
    BasicMessagePtr m3 = std::make_shared<BasicMessage>("b3", 3, 170);

    HiddenMessageSegments segments {200, 10};
    HiddenMessagePtr h1 = std::make_shared<HiddenMessage>("h1", 1, segments);

    Symulator sym(0, // timeForGeneratingHiddenMessages
            0, // numberOfHiddenMessagesToGenerate
            0, // timeForGeneratingBasicMessages
            8, // hiddenDataSegmentLength
            1000, // channelBandwith
            2, // hiddenDataAppearance
            3, // basicDataAppearance
            220, // basicDataLength
            2, // hiddenMessageSegmentLength
            50); // hiddenDataSegmentValue

    std::vector<BasicMessagePtr> bms { m1, m2, m3 };
    std::vector<HiddenMessagePtr> hms { h1 };
    sym.SetBasicMessages(bms);
    sym.SetHiddenMessages(hms);

    assert(StepToString(sym.Step()) == " #  #  # ");
    assert(StepToString(sym.Step()) == " # h1  #  # ");
    assert(StepToString(sym.Step()) == "b1 b2  #  # PacketWithHiddenData h1 segment: 200 hasMore? 1 datasources: [b1 length: 120 hasMore? 0 | b2 length: 80 hasMore? 1 | ] PacketWithHiddenData h1 segment: 10 hasMore? 0 datasources: [b2 length: 10 hasMore? 1 | ]  # PacketWithHiddenData h1 segment: 200 hasMore? 1 datasources: [b1 length: 120 hasMore? 0 | b2 length: 80 hasMore? 1 | ] PacketWithHiddenData h1 segment: 10 hasMore? 0 datasources: [b2 length: 10 hasMore? 1 | ] ");
    assert(StepToString(sym.Step()) == "b3  #  # PacketWithoutHiddenData datasources: [b2 length: 130 hasMore? 0 | b3 length: 170 hasMore? 0 | ]  # PacketWithoutHiddenData datasources: [b2 length: 130 hasMore? 0 | b3 length: 170 hasMore? 0 | ] ");
    assert(sym.HasSymulationEnd());
}

void ShouldGenerateRandomData() {
    srand(666);

    Symulator sym(0, // timeForGeneratingHiddenMessages
            4, // numberOfHiddenMessagesToGenerate
            0, // timeForGeneratingBasicMessages
            8, // hiddenDataSegmentLength
            1000, // channelBandwith
            2, // hiddenDataAppearance
            3, // basicDataAppearance
            220, // basicDataLength
            2, // hiddenMessageSegmentLength
            50); // hiddenDataSegmentValue

    sym.GenerateSymulationData();
    assert(StepToString(sym.Step()) == "p0 p1 p2 p3  # u0  # PacketWithHiddenData u0 segment: 49 hasMore? 1 datasources: [p0 length: 49 hasMore? 1 | ] PacketWithHiddenData u0 segment: 12 hasMore? 1 datasources: [p0 length: 12 hasMore? 1 | ] PacketWithHiddenData u0 segment: 14 hasMore? 0 datasources: [p0 length: 14 hasMore? 1 | ] PacketWithoutHiddenData datasources: [p0 length: 91 hasMore? 0 | p1 length: 83 hasMore? 0 | p2 length: 113 hasMore? 0 | p3 length: 215 hasMore? 0 | ]  # PacketWithHiddenData u0 segment: 49 hasMore? 1 datasources: [p0 length: 49 hasMore? 1 | ] PacketWithHiddenData u0 segment: 12 hasMore? 1 datasources: [p0 length: 12 hasMore? 1 | ] PacketWithHiddenData u0 segment: 14 hasMore? 0 datasources: [p0 length: 14 hasMore? 1 | ] PacketWithoutHiddenData datasources: [p0 length: 91 hasMore? 0 | p1 length: 83 hasMore? 0 | p2 length: 113 hasMore? 0 | p3 length: 215 hasMore? 0 | ] ");
    assert(StepToString(sym.Step()) == "p4 p5 p6 p7  # u1  # PacketWithHiddenData u1 segment: 17 hasMore? 1 datasources: [p4 length: 11 hasMore? 0 | p5 length: 6 hasMore? 1 | ] PacketWithHiddenData u1 segment: 29 hasMore? 0 datasources: [p5 length: 29 hasMore? 1 | ] PacketWithoutHiddenData datasources: [p5 length: 105 hasMore? 0 | p6 length: 133 hasMore? 0 | p7 length: 147 hasMore? 0 | ]  # PacketWithHiddenData u1 segment: 17 hasMore? 1 datasources: [p4 length: 11 hasMore? 0 | p5 length: 6 hasMore? 1 | ] PacketWithHiddenData u1 segment: 29 hasMore? 0 datasources: [p5 length: 29 hasMore? 1 | ] PacketWithoutHiddenData datasources: [p5 length: 105 hasMore? 0 | p6 length: 133 hasMore? 0 | p7 length: 147 hasMore? 0 | ] ");
    assert(StepToString(sym.Step()) == "p8 p9 p10  # u2 u3  # PacketWithHiddenData u2 segment: 37 hasMore? 0 datasources: [p8 length: 37 hasMore? 1 | ] PacketWithHiddenData u3 segment: 14 hasMore? 1 datasources: [p8 length: 14 hasMore? 1 | ] PacketWithHiddenData u3 segment: 21 hasMore? 1 datasources: [p8 length: 21 hasMore? 1 | ] PacketWithHiddenData u3 segment: 47 hasMore? 1 datasources: [p8 length: 40 hasMore? 0 | p9 length: 7 hasMore? 1 | ] PacketWithHiddenData u3 segment: 39 hasMore? 1 datasources: [p9 length: 39 hasMore? 1 | ] PacketWithHiddenData u3 segment: 32 hasMore? 0 datasources: [p9 length: 32 hasMore? 1 | ]  # PacketWithHiddenData u2 segment: 37 hasMore? 0 datasources: [p8 length: 37 hasMore? 1 | ] PacketWithHiddenData u3 segment: 14 hasMore? 1 datasources: [p8 length: 14 hasMore? 1 | ] PacketWithHiddenData u3 segment: 21 hasMore? 1 datasources: [p8 length: 21 hasMore? 1 | ] PacketWithHiddenData u3 segment: 47 hasMore? 1 datasources: [p8 length: 40 hasMore? 0 | p9 length: 7 hasMore? 1 | ] PacketWithHiddenData u3 segment: 39 hasMore? 1 datasources: [p9 length: 39 hasMore? 1 | ] PacketWithHiddenData u3 segment: 32 hasMore? 0 datasources: [p9 length: 32 hasMore? 1 | ] ");
    assert(StepToString(sym.Step()) == "p11 p12  #  # PacketWithoutHiddenData datasources: [p9 length: 129 hasMore? 0 | p10 length: 1 hasMore? 0 | p11 length: 40 hasMore? 0 | p12 length: 124 hasMore? 0 | ]  # PacketWithoutHiddenData datasources: [p9 length: 129 hasMore? 0 | p10 length: 1 hasMore? 0 | p11 length: 40 hasMore? 0 | p12 length: 124 hasMore? 0 | ] ");
    assert(sym.HasSymulationEnd());

    std::vector<HiddenMessagePtr> hiddenMessages = sym.GetAllHiddenMessages();
    for (auto m : hiddenMessages) {
        assert(m->GetDelay() == 0);
    }

    std::vector<BasicMessagePtr> basicMessages = sym.GetAllBasicMessages();
    assert(basicMessages[0]->GetDelay() == 0);
    assert(basicMessages[1]->GetDelay() == 0);
    assert(basicMessages[2]->GetDelay() == 0);
    assert(basicMessages[3]->GetDelay() == 0);

    assert(basicMessages[4]->GetDelay() == 0);
    assert(basicMessages[5]->GetDelay() == 0);
    assert(basicMessages[6]->GetDelay() == 0);
    assert(basicMessages[7]->GetDelay() == 0);

    assert(basicMessages[8]->GetDelay() == 0);
    assert(basicMessages[9]->GetDelay() == 1);
    assert(basicMessages[10]->GetDelay() == 1);
    assert(basicMessages[11]->GetDelay() == 0);
    assert(basicMessages[12]->GetDelay() == 0);

    assert(sym.GetHiddenMessagesDelay() == 0);
    assert(sym.GetBasicMessagesDelay() - 0.1666 < 0.01);
}

std::string StepToString(StepEvents events) {
    std::string basicMessagesArrivedDesc = "";
    std::string hiddenMessagesArrivedDesc = "";
    std::string packetsGeneratedDesc = "";
    std::string packetsAtTargetDesc = "";

    for (auto m : events.arrivedBasicMessages) { basicMessagesArrivedDesc += m->GetId() + " "; }
    for (auto m : events.arrivedHiddenMessages) { hiddenMessagesArrivedDesc += m->GetId() + " "; }
    for (auto p : events.packetsGenerated) { packetsGeneratedDesc += p->GetDescription() + " "; }
    for (auto p : events.packetsThatReachedTarget) { packetsAtTargetDesc += p->GetDescription() + " "; }

    return basicMessagesArrivedDesc + " # " + hiddenMessagesArrivedDesc + " # " + packetsGeneratedDesc + " # " + packetsAtTargetDesc;
}
