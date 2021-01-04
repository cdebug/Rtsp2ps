#ifndef RTSPDECODER_H
#define RTSPDECODER_H
#include "common.h"
#include "psencoder.h"
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}


class RtspDecoder
{
public:
    RtspDecoder();
    ~RtspDecoder();
    int init();
    void executeProcess();
    void setStreamUrl(std::string);
    void setStreamEncoder(std::shared_ptr<PsEncoder>);
    void removeStreamEncoder();
private:
    void sendFrameData(uint8_t*, int, FrameType);

    AVFormatContext *m_pFormatCtx;
	AVCodecContext  *m_pVideoCodecCtx = nullptr;
	AVCodecContext  *m_pAudioCodecCtx = nullptr;
    int m_videoindex, m_audioindex;
    std::string m_streamUrl;
    std::weak_ptr<PsEncoder> m_encoder;
};

#endif //RTSPDECODER_H