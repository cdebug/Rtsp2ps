#ifndef COMMON_H
#define COMMON_H
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <list>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
using namespace std;

#define SOCK_PORT_MIN 20000
#define SOCK_PORT_MAX 30000

enum SocketProtocol{
    PROTOCOL_UDP,
    PROTOCOL_TCP,
};

enum FrameType{
    FRAME_VIDEO,
    FRAME_AUDIO,
};

static unsigned const samplingFrequencyTable[16] = { 96000, 88200, 64000, 48000, 44100, 32000, 
                24000, 22050, 16000, 12000, 11025, 8000, 7350, 0, 0, 0 };

static void bitsWrite(uint8_t* buff, int pos, int size, uint32_t value)
{
    uint32_t index[] = 
    {
        0x80000000, 0x40000000, 0x20000000, 0x10000000, 0x8000000, 0x4000000, 0x2000000, 0x1000000,
        0x800000,   0x400000,   0x200000,   0x100000,   0x80000,   0x40000,   0x20000,   0x10000,
        0x8000,     0x4000,     0x2000,     0x1000,     0x800,     0x400,     0x200,     0x100,
        0x80,       0x40,       0x20,       0x10,       0x8,       0x4,       0x2,       0x1,
    };
    for(int i = 0; i < size; ++i)
    {
        int curBytePos = (pos+i)/8;
        int byteLeft = (pos+i)%8;
        buff[curBytePos] = buff[curBytePos] & ~(index[byteLeft] >> 24);
        uint32_t curBit = index[32-size+i]&value;
        uint8_t bitValue;
        int moveBits = pos+size - (curBytePos+1)*8;
        if(moveBits >=0)
            bitValue = curBit >> moveBits;
        else
            bitValue = curBit << -moveBits;
        buff[curBytePos] = buff[curBytePos] | bitValue;
    }
}

#endif //COMMON_H