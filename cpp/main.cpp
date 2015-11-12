#include <iostream>
#include <cassert>

#include "Models.h"

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

    return 0;
}
