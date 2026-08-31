#include "TimeSeries.h"

TimeSeries::TimeSeries()
{
    count = 0;
}

bool TimeSeries::add(unsigned long timestamp, int rr)
{
    if (count >= MAX_EVENTS)
        return false;

    events[count].timestamp = timestamp;
    events[count].rr = rr;
    events[count].hr = 60000.0 / rr;

    count++;

    return true;
}

BeatEvent TimeSeries::get(int index) const
{
    if (index < 0 || index >= count)
        return {0, 0, 0};

    return events[index];
}

int TimeSeries::size() const
{
    return count;
}

bool TimeSeries::isFull() const
{
    return count >= MAX_EVENTS;
}

void TimeSeries::clear()
{
    count = 0;
}