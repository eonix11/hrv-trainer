#include "TimeSeries.h"

TimeSeries::TimeSeries()
{
    head = 0;
    count = 0;
}

// Ring buffer: once full, the newest event overwrites the oldest one.
bool TimeSeries::add(unsigned long timestamp, int rr)
{
    int slot;

    if (count < MAX_EVENTS)
    {
        slot = (head + count) % MAX_EVENTS;
        count++;
    }
    else
    {
        slot = head;
        head = (head + 1) % MAX_EVENTS;
    }

    events[slot].timestamp = timestamp;
    events[slot].rr = rr;
    events[slot].hr = 60000.0 / rr;

    return true;
}

// Index 0 is the oldest event, size() - 1 the newest.
BeatEvent TimeSeries::get(int index) const
{
    if (index < 0 || index >= count)
        return {0, 0, 0};

    return events[(head + index) % MAX_EVENTS];
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
    head = 0;
    count = 0;
}
