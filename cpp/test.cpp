#include <iostream>
#include <cassert>

#include "Models.h"
#include "Symulator.h"

void ShouldSendPacketWithOnlyBasicData();
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
    std::cout << StepToString(sym.Step()) << std::endl;
    assert(!sym.HasSymulationEnd());
    std::cout << StepToString(sym.Step()) << std::endl;
    assert(!sym.HasSymulationEnd());
    std::cout << StepToString(sym.Step()) << std::endl;
    assert(sym.HasSymulationEnd());
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
