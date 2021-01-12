#include "rtspdecoder.h"
#include "common.h"

RtspDecoder::RtspDecoder()
{

}

RtspDecoder::~RtspDecoder()
{

}

int RtspDecoder::init()
{
	int             i;
	AVCodec         *pVideoCodec, *pAudioCodec;
	struct SwsContext *img_convert_ctx;
	m_pFormatCtx = avformat_alloc_context();
	AVDictionary* opts = NULL;
	av_dict_set(&opts, "rtsp_transport", "tcp", 0); //设置tcp or udp，默认一般优先tcp再尝试udp
	av_dict_set(&opts, "stimeout", "9000000", 0);//设置超时3秒
	if (avformat_open_input(&m_pFormatCtx, m_streamUrl.c_str(), NULL, &opts) != 0)
	{
		return -1;
	}
	if (avformat_find_stream_info(m_pFormatCtx, NULL)<0)
	{
		return -1;
	}
	m_videoindex = -1;
	m_audioindex = -1;
	for (i = 0; i<m_pFormatCtx->nb_streams; i++)
	{
		AVCodecContext* codec = m_pFormatCtx->streams[i]->codec;
		if (codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			if(codec->codec_id != AV_CODEC_ID_H264)
				{}
			else
				m_videoindex = i;
		}
		if (codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			if(codec->codec_id != AV_CODEC_ID_AAC)
				{}
			else
				m_audioindex = i;
		}
	}
	if (m_videoindex == -1 && m_audioindex == -1)
	{
		return -1;
	}
	//Open Video Ctx
	if(m_videoindex >= 0)
	{
		m_pVideoCodecCtx = m_pFormatCtx->streams[m_videoindex]->codec;
		pVideoCodec = avcodec_find_decoder(m_pVideoCodecCtx->codec_id);
		if (pVideoCodec == NULL)
		{
			return -1;
		}
		if (avcodec_open2(m_pVideoCodecCtx, pVideoCodec, NULL)<0)
		{
			return -1;
		}
	}
	//Open Audio Ctx
	if(m_audioindex >= 0)
	{
		m_pAudioCodecCtx = m_pFormatCtx->streams[m_audioindex]->codec;
		pAudioCodec = avcodec_find_decoder(m_pAudioCodecCtx->codec_id);
		if (pAudioCodec == NULL)
		{
			return -1;
		}
		if (avcodec_open2(m_pAudioCodecCtx, pAudioCodec, NULL)<0)
		{
			return -1;
		}
	}
	else
	//Output Info---输出一些文件（RTSP）信息  
	// av_dump_format(m_pFormatCtx, 0, m_streamUrl.c_str(), 0);
    return 0;
}

void RtspDecoder::executeProcess()
{
	shared_ptr<PsEncoder> encoder = m_encoder.lock();
	if(encoder)
	{
		encoder->setSampleRate(m_pFormatCtx->streams[m_audioindex]->codec->sample_rate);
		encoder->setChannels(m_pFormatCtx->streams[m_audioindex]->codec->channels);
	}
	AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	for (;;)
	{
		//------------------------------  
		if (av_read_frame(m_pFormatCtx, packet) >= 0)
		{
			if (packet->stream_index == m_videoindex)
				sendFrameData(packet->data, packet->size, FRAME_VIDEO);
			else if (packet->stream_index == m_audioindex)
				sendFrameData(packet->data, packet->size, FRAME_AUDIO);
			av_free_packet(packet);
		}
	}
	avcodec_close(m_pVideoCodecCtx);
	avcodec_close(m_pAudioCodecCtx);
	avformat_close_input(&m_pFormatCtx);
}

void RtspDecoder::sendFrameData(uint8_t* data, int len, FrameType type)
{
	shared_ptr<PsEncoder> encoder = m_encoder.lock();
	if(encoder)
		encoder->sendFrameData(data, len, type);
}

void RtspDecoder::setStreamUrl(std::string url)
{
    m_streamUrl = url;
}

void RtspDecoder::setStreamEncoder(shared_ptr<PsEncoder> encoder)
{
    m_encoder = encoder;
}

void RtspDecoder::removeStreamEncoder()
{
    m_encoder.reset();
}