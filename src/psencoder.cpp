#include "psencoder.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "common.h"
#include "udpsocket.h"
#include "tcpsocket.h"

#define PS_HDR_LEN  14
#define SYS_HDR_LEN 18
#define PSM_HDR_LEN 24
#define PES_HDR_LEN 14
#define RTP_HDR_LEN 12
#define MAX_RTP_PAYLOAD 1400
#define MAX_PES_LENGTH 65000
#define AAC_HEADER_LEN 7

static uint32_t crc32table[256] = {
	0x00000000, 0xB71DC104, 0x6E3B8209, 0xD926430D, 0xDC760413, 0x6B6BC517,
	0xB24D861A, 0x0550471E, 0xB8ED0826, 0x0FF0C922, 0xD6D68A2F, 0x61CB4B2B,
	0x649B0C35, 0xD386CD31, 0x0AA08E3C, 0xBDBD4F38, 0x70DB114C, 0xC7C6D048,
	0x1EE09345, 0xA9FD5241, 0xACAD155F, 0x1BB0D45B, 0xC2969756, 0x758B5652,
	0xC836196A, 0x7F2BD86E, 0xA60D9B63, 0x11105A67, 0x14401D79, 0xA35DDC7D,
	0x7A7B9F70, 0xCD665E74, 0xE0B62398, 0x57ABE29C, 0x8E8DA191, 0x39906095,
	0x3CC0278B, 0x8BDDE68F, 0x52FBA582, 0xE5E66486, 0x585B2BBE, 0xEF46EABA,
	0x3660A9B7, 0x817D68B3, 0x842D2FAD, 0x3330EEA9, 0xEA16ADA4, 0x5D0B6CA0,
	0x906D32D4, 0x2770F3D0, 0xFE56B0DD, 0x494B71D9, 0x4C1B36C7, 0xFB06F7C3,
	0x2220B4CE, 0x953D75CA, 0x28803AF2, 0x9F9DFBF6, 0x46BBB8FB, 0xF1A679FF,
	0xF4F63EE1, 0x43EBFFE5, 0x9ACDBCE8, 0x2DD07DEC, 0x77708634, 0xC06D4730,
	0x194B043D, 0xAE56C539, 0xAB068227, 0x1C1B4323, 0xC53D002E, 0x7220C12A,
	0xCF9D8E12, 0x78804F16, 0xA1A60C1B, 0x16BBCD1F, 0x13EB8A01, 0xA4F64B05,
	0x7DD00808, 0xCACDC90C, 0x07AB9778, 0xB0B6567C, 0x69901571, 0xDE8DD475,
	0xDBDD936B, 0x6CC0526F, 0xB5E61162, 0x02FBD066, 0xBF469F5E, 0x085B5E5A,
	0xD17D1D57, 0x6660DC53, 0x63309B4D, 0xD42D5A49, 0x0D0B1944, 0xBA16D840,
	0x97C6A5AC, 0x20DB64A8, 0xF9FD27A5, 0x4EE0E6A1, 0x4BB0A1BF, 0xFCAD60BB,
	0x258B23B6, 0x9296E2B2, 0x2F2BAD8A, 0x98366C8E, 0x41102F83, 0xF60DEE87,
	0xF35DA999, 0x4440689D, 0x9D662B90, 0x2A7BEA94, 0xE71DB4E0, 0x500075E4,
	0x892636E9, 0x3E3BF7ED, 0x3B6BB0F3, 0x8C7671F7, 0x555032FA, 0xE24DF3FE,
	0x5FF0BCC6, 0xE8ED7DC2, 0x31CB3ECF, 0x86D6FFCB, 0x8386B8D5, 0x349B79D1,
	0xEDBD3ADC, 0x5AA0FBD8, 0xEEE00C69, 0x59FDCD6D, 0x80DB8E60, 0x37C64F64,
	0x3296087A, 0x858BC97E, 0x5CAD8A73, 0xEBB04B77, 0x560D044F, 0xE110C54B,
	0x38368646, 0x8F2B4742, 0x8A7B005C, 0x3D66C158, 0xE4408255, 0x535D4351,
	0x9E3B1D25, 0x2926DC21, 0xF0009F2C, 0x471D5E28, 0x424D1936, 0xF550D832,
	0x2C769B3F, 0x9B6B5A3B, 0x26D61503, 0x91CBD407, 0x48ED970A, 0xFFF0560E,
	0xFAA01110, 0x4DBDD014, 0x949B9319, 0x2386521D, 0x0E562FF1, 0xB94BEEF5,
	0x606DADF8, 0xD7706CFC, 0xD2202BE2, 0x653DEAE6, 0xBC1BA9EB, 0x0B0668EF,
	0xB6BB27D7, 0x01A6E6D3, 0xD880A5DE, 0x6F9D64DA, 0x6ACD23C4, 0xDDD0E2C0,
	0x04F6A1CD, 0xB3EB60C9, 0x7E8D3EBD, 0xC990FFB9, 0x10B6BCB4, 0xA7AB7DB0,
	0xA2FB3AAE, 0x15E6FBAA, 0xCCC0B8A7, 0x7BDD79A3, 0xC660369B, 0x717DF79F,
	0xA85BB492, 0x1F467596, 0x1A163288, 0xAD0BF38C, 0x742DB081, 0xC3307185,
	0x99908A5D, 0x2E8D4B59, 0xF7AB0854, 0x40B6C950, 0x45E68E4E, 0xF2FB4F4A,
	0x2BDD0C47, 0x9CC0CD43, 0x217D827B, 0x9660437F, 0x4F460072, 0xF85BC176,
	0xFD0B8668, 0x4A16476C, 0x93300461, 0x242DC565, 0xE94B9B11, 0x5E565A15,
	0x87701918, 0x306DD81C, 0x353D9F02, 0x82205E06, 0x5B061D0B, 0xEC1BDC0F,
	0x51A69337, 0xE6BB5233, 0x3F9D113E, 0x8880D03A, 0x8DD09724, 0x3ACD5620,
	0xE3EB152D, 0x54F6D429, 0x7926A9C5, 0xCE3B68C1, 0x171D2BCC, 0xA000EAC8,
	0xA550ADD6, 0x124D6CD2, 0xCB6B2FDF, 0x7C76EEDB, 0xC1CBA1E3, 0x76D660E7,
	0xAFF023EA, 0x18EDE2EE, 0x1DBDA5F0, 0xAAA064F4, 0x738627F9, 0xC49BE6FD,
	0x09FDB889, 0xBEE0798D, 0x67C63A80, 0xD0DBFB84, 0xD58BBC9A, 0x62967D9E,
	0xBBB03E93, 0x0CADFF97, 0xB110B0AF, 0x060D71AB, 0xDF2B32A6, 0x6836F3A2,
	0x6D66B4BC, 0xDA7B75B8, 0x035D36B5, 0xB440F7B1
};

uint32_t mpeg_crc32(uint32_t crc, const uint8_t *buffer, uint32_t size)
{  
	unsigned int i;

	for (i = 0; i < size; i++) {
		crc = crc32table[(crc ^ buffer[i]) & 0xff] ^ (crc >> 8);  
	}  
	return crc ;  
}

bool startCode3(uint8_t* data)
{
    if(data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x01)
        return true;
    else
        return false;
}

bool startCode4(uint8_t* data)
{
    if(data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x00 && data[3] == 0x01)
        return true;
    else
        return false;
}

int getNaluSize(uint8_t* data, int len)
{
    for(int i = 4; i < len - 4; ++i)
    {
        if(startCode3(data+i) || startCode4(data+i))
            return i;
    }
    return len;
}

PsEncoder::PsEncoder(SocketProtocol protocol)
{
    if(PROTOCOL_UDP == protocol)
        m_sendSocket.reset(new UdpSocket);
    else
        m_sendSocket.reset(new TcpSocket);
}

PsEncoder::~PsEncoder()
{

}

void PsEncoder::setSsrc(uint32_t ssrc)
{
    m_ssrc = ssrc;
}

int PsEncoder::getLocalPort()
{
    if(m_sendSocket)
        return m_sendSocket->localPort();
    else
        return -1;
}

int PsEncoder::initSocket(string serverIp, int serverPort, int localPort)
{
    if(m_sendSocket)
        return m_sendSocket->init(serverIp, serverPort, localPort);
    else
        return -1;
}

int PsEncoder::sendFrameData(uint8_t* data, int len, FrameType type)
{
    if(FRAME_VIDEO == type)
        return sendVideoData(data, len);
    else if(FRAME_AUDIO == type)
        return sendAudioData(data, len);
}
int PsEncoder::sendVideoData(uint8_t* data, int len)
{
    if(len < 5)
        return -1;
    while (len > 0)
    {
        int naluSize = getNaluSize(data, len);
        int naluType;
        if(startCode3(data))
            naluType = data[3] & 0x1f;
        else
            naluType = data[4] & 0x1f;
        switch (naluType)
        {
        case 1:
            sendPFrame(data, len);
            break;
        case 5:
            sendIdrFrame(data, len);
            break;
        case 6:
            memcpy(m_seiBuff, data, naluSize);
            m_seiLen = naluSize;
            break;
        case 7:
            memcpy(m_spsBuff, data, naluSize);
            m_spsLen = naluSize;
            break;
        case 8:
            memcpy(m_ppsBuff, data, naluSize);
            m_ppsLen = naluSize;
            break;
        default:
            return 0;
        }
        data+=naluSize;
        len-=naluSize;
    }
    return 0;
}

void PsEncoder::sendIdrFrame(uint8_t* data, int len)
{
    int pesNum = len % MAX_PES_LENGTH == 0 ? len / MAX_PES_LENGTH : len / MAX_PES_LENGTH + 1;
    for(int i = 0; i < pesNum; ++i)
    {
        int pesSize = MAX_PES_LENGTH;
        if(i + 1 == pesNum)
            pesSize = len - MAX_PES_LENGTH*i;
        if(i == 0)
        {
            m_idrLen = wrapPsHeader(m_idrBuff, m_timeStamp);
            m_idrLen += wrapSystemHeader(m_idrBuff+m_idrLen);
            m_idrLen += wrapSystemMap(m_idrBuff+m_idrLen);
        }
        wrapIDRFramePes(m_idrBuff, m_idrLen, data+MAX_PES_LENGTH*i, pesSize);
        sendPsData(m_idrBuff, m_idrLen);
        m_idrLen = 0;
        m_spsLen = 0;
        m_ppsLen = 0;
        m_seiLen = 0;
        m_idrBuffInited = true;
    }
    m_timeStamp += m_timeStampIncrement;
    m_idrLen = 0;
}

void PsEncoder:: sendPFrame(uint8_t* data, int len)
{
    m_pLen = wrapPsHeader(m_pBuff, m_timeStamp);
    wrapPFramePes(m_pBuff, m_pLen,data, len);
    sendPsData(m_pBuff, m_pLen);
    m_timeStamp += m_timeStampIncrement;
    m_pLen = 0;
    m_pBuffInited = true;
}

int PsEncoder::wrapPsHeader(uint8_t* data, uint32_t pts)
{
    uint32_t pts_ext = 0;
    uint32_t mux_rate = 19460;
#if 0
    
#else
    // pack_start_code
    data[0] = 0x00;
    data[1] = 0x00;
    data[2] = 0x01;
    data[3] = 0xba;
    // 33-system_clock_reference_base + 9-system_clock_reference_extension
    // '01xxx1xx xxxxxxxx xxxxx1xx xxxxxxxx xxxxx1xx xxxxxxx1'
    data[4] = 0x44 | (((pts>> 30) & 0x07) << 3) | ((pts >> 28) & 0x03);
    data[5] = ((pts >> 20) & 0xFF);
    data[6] = 0x04 | (((pts >> 15) & 0x1F) << 3) | ((pts >> 13) & 0x03);
    data[7] = ((pts >> 5) & 0xFF);
    data[8] = 0x04 | ((pts & 0x1F) << 3) | ((pts_ext >> 7) & 0x03);
    data[9] = 0x01 | ((pts_ext & 0x7F) << 1);

    // program_mux_rate
    // 'xxxxxxxx xxxxxxxx xxxxxx11'
    data[10] = (uint8_t)(mux_rate >> 14);
    data[11] = (uint8_t)(mux_rate >> 6);
    data[12] = (uint8_t)(0x03 | ((mux_rate & 0x3F) << 2));

    // stuffing length
    // '00000xxx'
    data[13] = 0xF8;
#endif
    return 14;
}

int PsEncoder::wrapSystemHeader(uint8_t* data)
{
    int rate_bound = 50000;
    int audio_bound = 1;
    int fixed_flag = 0;
    int csps_flag = 1;
    int sys_audio_lock_flag = 1;
    int sys_video_lock_flag = 1;
    int video_bound = 1;
    int packet_rate_restriction_flag = 0;
    int audio_buff_bound_scale = 0;
    int video_buff_bound_scale = 1;
    int audio_buff_bound_size = 512;
    int video_buff_bound_size = 2048;
#if 0
    
#else
    data[0] = 0x00;
    data[1] = 0x00;
    data[2] = 0x01;
    data[3] = 0xbb;

    data[6] = 0x80 | ((rate_bound >> 15) & 0x7F);
    data[7] = (rate_bound >> 7) & 0xFF;
    data[8] = 0x01 | ((rate_bound & 0x7F) << 1);

    // 6-audio_bound + 1-fixed_flag + 1-CSPS_flag
    data[9] = ((audio_bound & 0x3F) << 2) | ((fixed_flag & 0x01) << 1) | (csps_flag & 0x01);

    // 1-system_audio_lock_flag + 1-system_video_lock_flag + 1-maker + 5-video_bound
    data[10] = 0x20 | ((sys_audio_lock_flag & 0x01) << 7) | ((sys_video_lock_flag & 0x01) << 6) | (video_bound & 0x1F);

    // 1-packet_rate_restriction_flag + 7-reserved
    data[11] = 0x7F | ((packet_rate_restriction_flag & 0x01) << 7);

    data[12] = 0xc0;
    // '11' + 1-P-STD_buffer_bound_scale + 13-P-STD_buffer_size_bound
    // '11xxxxxx xxxxxxxx'
    data[16] = 0xC0 | ((audio_buff_bound_scale & 0x01) << 5) | ((audio_buff_bound_size >> 8) & 0x1F);
    data[17] = audio_buff_bound_size & 0xFF;

    data[12] = 0xe0;
    // '11' + 1-P-STD_buffer_bound_scale + 13-P-STD_buffer_size_bound
    // '11xxxxxx xxxxxxxx'
    data[16] = 0xC0 | ((video_buff_bound_scale & 0x01) << 5) | ((video_buff_bound_size >> 8) & 0x1F);
    data[17] = video_buff_bound_size & 0xFF;
    // header length
    bitsWrite(data + 4, 0, 16, 18 - 6);
    return 18;
#endif
}

int PsEncoder::wrapSystemMap(uint8_t* data)
{
    int psm_ver = 0;
#if 0

#else
    data[0] = 0x00;
    data[1] = 0x00;
    data[2] = 0x01;
	data[3] = 0xbc;

	// program_stream_map_length 16-bits
	bitsWrite(data+4, 0, 16,18);			/*program stream map length*/ 

	// current_next_indicator '1'
	// single_extension_stream_flag '1'
	// reserved '0'
	// program_stream_map_version 'xxxxx'
	data[6] = 0xc0 | (psm_ver & 0x1F);

	// reserved '0000000'
	// marker_bit '1'
	data[7] = 0x01;

	// program_stream_info_length 16-bits program_stream_info_length = 0
	data[8] = 0;
	data[9] = 0;

	// elementary_stream_map_length 16-bits
    bitsWrite(data+10, 0, 16, 8);
	// audio
	data[12] = 0x0f;//AAC
	data[13] = 0xc0;
	// elementary_stream_info_length:16
	data[14] = 0;
	data[15] = 0;
    //video
	data[16] = 0x1b;//H264
	data[17] = 0xe0;
	// elementary_stream_info_length:16
	data[18] = 0;
	data[19] = 0;
    int j = 20;
	// crc32
	uint32_t crc = mpeg_crc32(0xffffffff, data, (uint32_t)j);
	data[j+3] = (uint8_t)((crc >> 24) & 0xFF);
	data[j+2] = (uint8_t)((crc >> 16) & 0xFF);
	data[j+1] = (uint8_t)((crc >> 8) & 0xFF);
	data[j+0] = (uint8_t)(crc & 0xFF);
#endif
    return 24;
}

void PsEncoder::wrapIDRFramePes(uint8_t* buff, int& bufflen, uint8_t* data, int len)
{
    if(m_spsLen > 0)
    {
        makePes(buff+bufflen, 0xe0, m_spsLen, m_timeStamp);
        bufflen += PES_HDR_LEN;
        memcpy(buff+bufflen, m_spsBuff, m_spsLen);
        bufflen+=m_spsLen;
        m_spsLen = 0;
    }
    if(m_ppsLen > 0)
    {
        makePes(buff+bufflen, 0xe0, m_ppsLen, m_timeStamp);
        bufflen += PES_HDR_LEN;
        memcpy(buff + bufflen, m_ppsBuff, m_ppsLen);
        bufflen+=m_ppsLen;
        m_ppsLen = 0;
    }
    if(m_seiLen > 0)
    {
        makePes(buff+bufflen, 0xe0, m_seiLen, m_timeStamp);
        bufflen += PES_HDR_LEN;
        memcpy(buff + bufflen, m_seiBuff, m_seiLen);
        bufflen+=m_seiLen;
        m_seiLen = 0;
    }

    makePes(buff+bufflen, 0xe0, len, m_timeStamp);
    bufflen += PES_HDR_LEN;
    memcpy(buff + bufflen, data, len);
    bufflen += len;
}

void PsEncoder::wrapPFramePes(uint8_t* buff, int& bufflen, uint8_t* data, int len)
{
    makePes(buff+bufflen, 0xe0, len, m_timeStamp);
    bufflen += PES_HDR_LEN;
    memcpy(buff + bufflen, data, len);
    bufflen += len;
}

void PsEncoder::wrapAudioFramePes(uint8_t* buff, int& bufflen, uint8_t* data, int len)
{
    makePes(buff+bufflen, 0xc0, len, m_timeStamp);
    bufflen = PES_HDR_LEN;
    memcpy(buff + bufflen, data, len);
    bufflen += len;
}

void PsEncoder::makePes(uint8_t* buff, uint8_t streamId, int len, uint32_t pts)
{
    bitsWrite(buff, 0, 24, 0x000001 ); // header
	bitsWrite(buff, 24, 8, streamId);
 
	bitsWrite(buff, 32, 16, len+8); //pes_packet_length : es len and the following pes len
	bitsWrite(buff, 48, 8, 0x8c ); //
	bitsWrite(buff, 56, 2, 0x02 ); //第七字节的高两位是PTS和DTS指示位，00表示无PTS无DTS，01禁止使用，10表示PES头部字段会附加PTS结构，11表示PTS和DTS都包括
	bitsWrite(buff, 58, 6, 0x00 ); //
	bitsWrite(buff, 64, 8, 0x05 ); //8
	//UINT64 i_scr = I_SCR(_iFrameIndextemp);
 
    bitsWrite(buff, 72, 4, 2 );                    /*'0010'*/
    bitsWrite(buff, 76, 3, ((pts)>>30)&0x07 );     /*PTS[32..30]*/
    bitsWrite(buff, 79, 1, 1 ); 
    bitsWrite(buff, 80, 15,((pts)>>15)&0x7FFF);    /*PTS[29..15]*/
    bitsWrite(buff, 95, 1, 1 );
    bitsWrite(buff, 96, 15,(pts)&0x7FFF);          /*PTS[14..0]*/
    bitsWrite(buff, 111, 1, 1 );
}

void PsEncoder::makeRtpHeader()
{
    bitsWrite(m_rtpBuff, 0, 16, 0x8060);
    bitsWrite(m_rtpBuff, 16, 16, m_seq);
    bitsWrite(m_rtpBuff, 32, 32, m_timeStamp);
    bitsWrite(m_rtpBuff, 64, 32, m_ssrc);
    m_rtpLen = RTP_HDR_LEN;
    m_seq += 1;
}

void PsEncoder::sendPsData(uint8_t* buff, int len)
{
    int rtpNum = len%MAX_RTP_PAYLOAD==0 ? len/MAX_RTP_PAYLOAD : len/MAX_RTP_PAYLOAD+1;
    for(int i = 0; i < rtpNum; ++i)
    {
        int payloadLen = MAX_RTP_PAYLOAD;
        if(i + 1 >= rtpNum)
            payloadLen = len - MAX_RTP_PAYLOAD * i;
        makeRtpHeader();
        memcpy(m_rtpBuff+m_rtpLen, buff+MAX_RTP_PAYLOAD*i, payloadLen);
        m_rtpLen+=payloadLen;
        sendRtpPacket();
    }
    std::this_thread::sleep_for(std::chrono::microseconds(1));
}

void PsEncoder::sendRtpPacket()
{
    if(m_sendSocket)
        m_sendSocket->sendData(m_rtpBuff, m_rtpLen);
}

int PsEncoder::sendAudioData(uint8_t* data, int len)
{
    uint8_t buff[40960];
    int bufflen = 0;
    makePes(buff, 0xc0, len+AAC_HEADER_LEN, m_timeStamp);
    bufflen+=PES_HDR_LEN;
    bufflen += makeAudioHeader(buff+bufflen, len);
    memcpy(buff+bufflen, data, len);
    bufflen+= len;
    sendPsData(buff, bufflen);
    return 0;
}

int PsEncoder::makeAudioHeader(uint8_t* buff, int audioLen)
{
    bitsWrite(buff, 0, 12, 0xfff);   /* syncword */
    bitsWrite(buff, 12, 1, 1);        /* ID */
    bitsWrite(buff, 13, 2, 0);        /* layer */
    bitsWrite(buff, 15, 1, 1);        /* protection_absent */
    bitsWrite(buff, 16, 2, 01); /* profile_objecttype */
    bitsWrite(buff, 18, 4, m_sampleRateType);
    bitsWrite(buff, 22, 1, 0);        /* private_bit */
    bitsWrite(buff, 23, 3, m_channels); /* channel_configuration */
    bitsWrite(buff, 26, 1, 0);        /* original_copy */
    bitsWrite(buff, 27, 1, 0);        /* home */

    /* adts_variable_header */
    bitsWrite(buff, 28, 1, 0);        /* copyright_identification_bit */
    bitsWrite(buff, 29, 1, 0);        /* copyright_identification_start */
    bitsWrite(buff, 30, 13, audioLen+AAC_HEADER_LEN); /* aac_frame_length */
    bitsWrite(buff, 43, 11, 0x7ff);   /* adts_buffer_fullness */
    bitsWrite(buff, 54, 2, 0);        /* number_of_raw_data_blocks_in_frame */

    return AAC_HEADER_LEN;
}

void PsEncoder::setSampleRate(int rate)
{
    m_sampleRateType = 0;
    for(int i = 0; i < 16; ++i)
    {
        if(rate == samplingFrequencyTable[i])
            m_sampleRateType = i;
    }
}

void PsEncoder::setChannels(int channels)
{
    m_channels = channels;
}