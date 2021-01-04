#ifndef PSENCODER_H
#define PSENCODER_H
#include "mysocket.h"
#include "common.h"

class PsEncoder
{
public:
    PsEncoder(SocketProtocol);
    ~PsEncoder();
    int initSocket(std::string, int, int);
    int sendFrameData(uint8_t*, int, FrameType);
    int sendVideoData(uint8_t*, int);
    int sendAudioData(uint8_t*, int);
    void setSsrc(uint32_t);
    int getLocalPort();
    void setSampleRate(int);
    void setChannels(int);

private:
    void sendIdrFrame(uint8_t*, int);
    void sendPFrame(uint8_t*, int);
    int wrapPsHeader(uint8_t*, uint32_t);
    int wrapSystemHeader(uint8_t*);
    int wrapSystemMap(uint8_t*);
    void wrapIDRFramePes(uint8_t*, int&, uint8_t*, int);
    void wrapPFramePes(uint8_t*, int&, uint8_t*, int);
    void wrapAudioFramePes(uint8_t*, int&, uint8_t*, int);
    int makeAudioHeader(uint8_t*, int);
    void makePes(uint8_t*, uint8_t, int, uint32_t);
    void makeRtpHeader();
    void sendPsData(uint8_t*, int);
    void sendRtpPacket();

    uint32_t m_ssrc;
    uint8_t m_spsBuff[1000];
    int m_spsLen;
    uint8_t m_ppsBuff[1000];
    int m_ppsLen;
    uint8_t m_seiBuff[1000];
    int m_seiLen;
    uint8_t m_idrBuff[409600];
    int m_idrLen;
    uint8_t m_pBuff[409600];
    int m_pLen;
    uint8_t m_rtpBuff[1600];
    int m_rtpLen;
    uint32_t m_timeStamp = 0;
    uint32_t m_timeStampIncrement = 3600;
    uint16_t m_seq = 0;

    bool m_idrBuffInited = false;
    bool m_pBuffInited = false;

    std::shared_ptr<MySocket> m_sendSocket;

    uint8_t m_frameData[409600];
    int m_frameLen;
    int m_sampleRateType;
    int m_channels;
};

#endif //PSENCODER_H