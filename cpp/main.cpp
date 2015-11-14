#include <iostream>
#include <boost/lexical_cast.hpp>

#include "Symulator.h"

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

int main(int argc, const char *argv[])
{
    // ./symulator --timeForGeneratingHiddenMessages <> --numberOfHiddenMessagesToGenerate <> --timeForGeneratingBasicMessages <> --hiddenDataSegmentLength <> --channelBandwidth <> --hiddenDataAppearance <> --basicDataAppearance <> --basicDataLength <> --hiddenMessageSegmentLength <> --hiddenDataSegmentValue <> --numberOfSymulations <> --timeForSymulation <>

    int timeForGeneratingHiddenMessages = boost::lexical_cast<int>(argv[2]);
    int numberOfHiddenMessagesToGenerate = boost::lexical_cast<int>(argv[4]);
    int timeForGeneratingBasicMessages = boost::lexical_cast<int>(argv[6]);
    int hiddenDataSegmentLength = boost::lexical_cast<int>(argv[8]);
    int channelBandwidth = boost::lexical_cast<int>(argv[10]);
    double hiddenDataAppearance = boost::lexical_cast<double>(argv[12]);
    double basicDataAppearance = boost::lexical_cast<double>(argv[14]);
    int basicDataLength = boost::lexical_cast<int>(argv[16]);
    int hiddenMessageSegmentLength = boost::lexical_cast<int>(argv[18]);
    int hiddenDataSegmentValue = boost::lexical_cast<int>(argv[20]);
    int numberOfSymulations = boost::lexical_cast<int>(argv[22]);
    int timeForSymulation =  boost::lexical_cast<int>(argv[24]);

    double hiddenMessagesDelaySum = 0;
    double basicMessagesDelaySum = 0;

    for (int numberOfSymulation = 0; numberOfSymulation < numberOfSymulations; numberOfSymulation++) {
        Symulator sym(timeForGeneratingHiddenMessages, numberOfHiddenMessagesToGenerate,
                timeForGeneratingBasicMessages, hiddenDataSegmentLength, channelBandwidth,
                hiddenDataAppearance, basicDataAppearance, basicDataLength, hiddenMessageSegmentLength,
                hiddenDataSegmentValue);

        while (timeForSymulation > sym.GetCurrentTime()) {
            //std::cout << StepToString(sym.Step()) << std::endl;
            sym.Step();
        }

        std::cout << " [ " << numberOfSymulation << " / " << numberOfSymulations << " ] "
            << "Time: " << sym.GetCurrentTime()
            << " HiddenMessages: " << sym.GetHiddenMessagesDelay() << "(" << sym.GetAllHiddenMessages().size() << ") "
            << " BasicMessages: " << sym.GetBasicMessagesDelay() << "(" << sym.GetAllBasicMessages().size() << ") "
            << std::endl;

        hiddenMessagesDelaySum += sym.GetHiddenMessagesDelay();
        basicMessagesDelaySum += sym.GetBasicMessagesDelay();

    }

    std::cout << "Avg. hidden delay: " << hiddenMessagesDelaySum / numberOfSymulations << std::endl;
    std::cout << "Avg. basic delay: " << basicMessagesDelaySum / numberOfSymulations << std::endl;
    return 0;
}
