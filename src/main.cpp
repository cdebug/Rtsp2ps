#include "common.h"
#include "rtspdecoder.h"
#include "psencoder.h"

int main()
{
    av_register_all();
	avformat_network_init();

    SocketProtocol protocol = PROTOCOL_UDP;
    std::string url = "rtsp://admin:S62LLYCU@192.168.31.134:554/Streaming/Channels/101?transportmode=unicast&profile=Profile_1";
    std::string serverIp = "192.168.31.14";
    int serverPort = 20000, localPort = 20000;
    uint32_t ssrc = 1111;
    std::shared_ptr<RtspDecoder> decoder(new RtspDecoder);
    std::shared_ptr<PsEncoder> encoder(new PsEncoder(protocol));
    decoder->setStreamUrl(url);
    if(decoder->init() != 0)
        return -1;

    if(encoder->initSocket(serverIp, serverPort, localPort) != 0)
        return -1;

    encoder->setSsrc(ssrc);
    decoder->setStreamEncoder(encoder);
    std::thread th(&RtspDecoder::executeProcess, decoder);
    th.join();

    return 0;
}