
#include "circularbuffer.h"

CircularBuffer::CircularBuffer(int s)
{
    size = s;
    buffer = new byte[size];
    in = 0;
    out = 0;
}

CircularBuffer::~CircularBuffer()
{
    if (size > 0) delete buffer;
}

bool CircularBuffer::In(byte * input, int s)
{
    for (int i = 0; i < s; i++)
    {
        if (in == out) return false;
        if (in == size) in = 0;

        buffer[in] = input[i];

        in++;
    }

    return true;
}

bool CircularBuffer::Out(byte * output, int s)
{
    for (int i = 0; i < s; i++)
    {
        if (out == in) return false;
        if (out == size) in = 0;

        output[i] = buffer[out];

        out++;
    }

    return true;
}

int CircularBuffer::GetSize()
{
    return size;
}
