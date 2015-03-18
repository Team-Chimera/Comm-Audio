#ifndef CIRCULARBUFFER
#define CIRCULARBUFFER

#include "client.h"

class CircularBuffer
{
    private:

        byte * buffer; // buffer for data
        int size;
        int in;        // pointer to the current insertion point in the buffer
        int out;       // pointer to the current removal point in the buffer

    public:

        CircularBuffer(int);
        ~CircularBuffer();

        bool In(byte*, int);
        bool Out(byte*, int);

        int GetSize();
};

#endif // CIRCULARBUFFER

