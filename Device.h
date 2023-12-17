//
// Created by lsk on 12/4/23.
//

#ifndef MIIO_CPP_REWRITE_DEVICE_H
#define MIIO_CPP_REWRITE_DEVICE_H

#include <vector>
#include <cstdint>
#include <netinet/in.h>
#include <cstdlib>
#include <cstring>
#include <string>
#include <ctime>
#include <arpa/inet.h>
#include <exception>
#include <stdexcept>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

#include "aes.h"
#include "socket_class.h"
#include "md5.h"
#include "type_tools.h"

using namespace std;
using namespace rapidjson;

class Device {
public:
    Device(const char* ip, const char* token);
    const char* send(const char* command, const char** params, int argsCount, int &respLength);
private:
    struct Msg
    {
        uint16_t length;
        unsigned char did[4];
        uint32_t stamp;
        unsigned char md5_checksum[16];
    };
    struct sockaddr_in* addr;
    int inited_ = 0;
    unsigned char token_[16];
    AES_ctx ctx_;
    uint8_t iv_[16];
    UdpClient udp_client_;
    unsigned char did_[4];
    uint32_t root_stamp_;
    inline int UdpSendSync(void* send_buf, u_int32_t send_buf_size, void* recv_buf, u_int32_t recv_buf_size)
    {
        return udp_client_.SendSync((char*)send_buf, send_buf_size, (char*)recv_buf, recv_buf_size);
    }
    static void Token2AesCtx(const unsigned char* token, AES_ctx* ctx, uint8_t* iv);
    static void UnpackHandshakeMsg(const unsigned char* msg_buf, Msg* msg);
    static void UnpackMsgHeader(const uint8_t* msg_buf, Msg* msg);
    void DecryptPayloads(uint8_t* payloads, uint16_t payloads_len);
    void EncryptPayloads(uint8_t* payloads, uint16_t payloads_len);
    static void PackMsgHeader(const Msg* msg, uint8_t* msg_buf);
    static void FillMd5Checksum(unsigned char* msg_buf, u_int32_t packet_length);
    static void MakePayloads16Multiple(string* payloads_send);
    static void GetPayloadsActualLength(uint8_t* payloads, uint16_t* payloads_len);
    uint32_t packet_id_ = 0;
    inline int GetPacketIdPlus1()
    {
        return ++packet_id_;
    }
};


#endif //MIIO_CPP_REWRITE_DEVICE_H
