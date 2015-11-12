#include <iostream>

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

    return 0;
}
