#ifndef HRVENGINE_H
#define HRVENGINE_H

#include "RRBuffer.h"

class HRVEngine
{
private:
    float rmssd;
    float averageHR;
    float sdnn;
    float pNN50;

public:
    HRVEngine();

    void calculate(const RRBuffer& buffer);

    float getRMSSD() const;
    float getAverageHR() const;
    float getSDNN() const;
    float getPNN50() const;
};

#endif