#include "RRBuffer.h"

RRBuffer::RRBuffer()
{
    count = 0;
}

bool RRBuffer::add(int rr)
{
    if (count >= MAX_SIZE)
        return false;

    buffer[count] = rr;
    count++;

    return true;
}

int RRBuffer::get(int index) const
{
    if (index < 0 || index >= count)
        return -1;

    return buffer[index];
}

int RRBuffer::size() const
{
    return count;
}

bool RRBuffer::isFull() const
{
    return count >= MAX_SIZE;
}

void RRBuffer::clear()
{
    count = 0;
}