#ifndef RRBUFFER_H
#define RRBUFFER_H

class RRBuffer
{
private:
    static const int MAX_SIZE = 30;

    int buffer[MAX_SIZE];
    int count;

public:
    RRBuffer();

    bool add(int rr);

    int get(int index) const;

    int size() const;

    bool isFull() const;

    void clear();
};

#endif