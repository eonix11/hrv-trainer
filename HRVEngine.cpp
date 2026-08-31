#include "HRVEngine.h"
#include <math.h>

HRVEngine::HRVEngine()
{
    rmssd = 0;
    averageHR = 0;
    sdnn = 0;
    pNN50 = 0;

}

void HRVEngine::calculate(const RRBuffer& buffer)
{
    int n = buffer.size();

    if (n < 2)
        return;

    float sumSq = 0;
    float avgRR = 0;

    // Average RR
    for (int i = 0; i < n; i++) {
        avgRR += buffer.get(i);
    }

    avgRR /= n;

    // Convert average RR → BPM
    averageHR = 60000.0 / avgRR;

    //sdnn
    float variance = 0;
    for (int i = 0; i < n; i++ ) {
        float difference = buffer.get(i) - avgRR;
        variance += difference * difference; 
    }
    sdnn = sqrt(variance / (n-1));

    int countOver50 = 0;
    // RMSSD
    for (int i = 1; i < n; i++) {
        float difference = buffer.get(i) - buffer.get(i - 1);

        sumSq += difference * difference;

        if(abs(difference) > 50) {
            countOver50++;
        }
    }
    
    rmssd = sqrt(sumSq / (n - 1));

    pNN50 = (countOver50 * 100.0) / (n-1);
}

float HRVEngine::getRMSSD() const {
    return rmssd;
}

float HRVEngine::getAverageHR() const {
    return averageHR;
}

float HRVEngine::getSDNN() const {
    return sdnn;
}

float HRVEngine::getPNN50() const {
    return pNN50;
}