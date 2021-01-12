#include "rtspdecoder.h"
#include "psencoder.h"

int main()
{
    string url = "rtsp://admin:S62LLYCU@192.168.31.134:554/Streaming/Channels/101?transportmode=unicast&profile=Profile_1";
    string serverIp = "192.168.31.243";
    int serverPort = 10000;
    SocketProtocol protocol = PROTOCOL_UDP;
    uint32_t ssrc = 1000;
	av_register_all();
	avformat_network_init();
    std::shared_ptr<RtspDecoder> decoder;
    decoder.reset(new RtspDecoder);
    decoder->setStreamUrl(url);
    if(decoder->init() != 0)
        return false;
    std::shared_ptr<PsEncoder> encoder;
    if(PROTOCOL_UDP == protocol)
        encoder.reset(new PsEncoder(PROTOCOL_UDP));
    else
        encoder.reset(new PsEncoder(PROTOCOL_TCP));

    if(encoder->initSocket(serverIp, serverPort, 0) == 0)
    {
        encoder->setSsrc(ssrc);
        decoder->setStreamEncoder(encoder);
        std::thread(&RtspDecoder::executeProcess, decoder).detach();
        return true;
    }
    return 0;
}