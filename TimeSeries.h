#ifndef TIMESERIES_H
#define TIMESERIES_H

struct BeatEvent
{
    unsigned long timestamp;
    int rr;
    float hr;
};

class TimeSeries
{
private:
    static const int MAX_EVENTS = 120;

    BeatEvent events[MAX_EVENTS];
    int head;   // index of the oldest event
    int count;

public:
    TimeSeries();

    bool add(unsigned long timestamp, int rr);

    BeatEvent get(int index) const;

    int size() const;

    bool isFull() const;

    void clear();
};

#endif