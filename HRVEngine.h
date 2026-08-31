#ifndef HRVENGINE_H
#define HRVENGINE_H

#include "RRBuffer.h"

class HRVEngine
{
private:
    float rmssd;
    float averageHR;
    float sdnn;

public:
    HRVEngine();

    void calculate(const RRBuffer& buffer);

    float getRMSSD() const;
    float getAverageHR() const;
    float getSDNN() const;
};

#endif