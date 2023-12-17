//
// Created by lsk on 12/4/23.
//

#include "Device.h"
#include <iostream>
using namespace std;

Device::Device(const char *ip, const char *token) {
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(54321);
    unsigned char token_buf[16];
    Hex2Ascii(token, token_buf);
    if (udp_client_.OpenUdpClient(&addr) == 0)
    {
        throw runtime_error("Failed to create UDP Client");
    }
    //set token
    memcpy(token_, token_buf, 16);
    //set aes ctx
    Token2AesCtx(token_, &ctx_, iv_);
    //handshake
    unsigned char handshake_buf[32] = { 0x21, 0x31, 0, 0x20, 0xff, 0xff, 0xff, 0xff,
                                        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    UdpSendSync(handshake_buf, 32, handshake_buf, 32);
    Msg msg;
    UnpackHandshakeMsg((unsigned char*)handshake_buf, &msg);
    //set did
    memcpy(did_, msg.did, 4);
    //set root_stamp
    time_t seconds;
    time(&seconds);
    root_stamp_ = (uint32_t)seconds - msg.stamp;
}

const char *Device::send(const char *command, const char** args, int argsCount, int &respLength) {
    // 1. Create Payload
    StringBuffer stringBuffer;
    Writer<StringBuffer> writer(stringBuffer);
    writer.StartObject();
    writer.Key("id");       writer.Int(GetPacketIdPlus1());
    writer.Key("method");   writer.String(command);
    writer.Key("params");   writer.StartArray();
    for(int i = 0; i < argsCount; i++) {
        writer.String(args[i]);
    }
    writer.EndArray();
    writer.EndObject();
    string payload = stringBuffer.GetString();
    cout << payload << endl;
    // 2. encrypt and send
    MakePayloads16Multiple(&payload);
    uint8_t* buf = new uint8_t[2000];
    uint16_t payloads_len = (uint16_t)payload.length();
    payload.copy((char*)&(buf[32]), payloads_len, 0);
    Msg msg;
    msg.length = uint16_t(32 + payloads_len);
    memcpy(msg.did, did_,4);
    time_t seconds;
    time(&seconds);
    msg.stamp = (uint32_t)seconds - root_stamp_;
    memcpy(msg.md5_checksum, token_, 16);
    PackMsgHeader(&msg, buf);
    EncryptPayloads(&(buf[32]), payloads_len);
    FillMd5Checksum(buf, msg.length);
    cout << buf << endl;
    int ret = UdpSendSync(buf, msg.length, buf, 2000);
    if (ret < 0)
    {
        delete[] buf;
        throw runtime_error("Failed to send command: " + to_string(ret));
    }
    //do NOT need msg struct for received msg
    UnpackMsgHeader(buf, &msg);
    payloads_len = (uint16_t)(ret - 32);
    DecryptPayloads(&(buf[32]), payloads_len);
    GetPayloadsActualLength(&(buf[32]), &payloads_len);
    payload.clear();
    payload.append((char*)&(buf[32]), payloads_len);
    delete[] buf;
    return payload.c_str();
}

void Device::Token2AesCtx(const unsigned char *token, AES_ctx *ctx_with_key, uint8_t *iv) {
    string token_str;
    token_str.append((char*)token, 16);
    MD5* md5;
    md5 = new MD5(token_str);
    const byte* key_byte = md5->getDigest();
    string key_str;
    key_str.append((char*)key_byte, 16);
    AES_init_ctx(ctx_with_key, key_byte);//AES_init_ctx only will use key_byte[0] to key_byte[15]
    delete md5;
    md5 = new MD5(key_str + token_str);
    const byte* iv_byte = md5->getDigest();
    memcpy(iv, iv_byte, 16);
    //AES_init_ctx_iv(ctx, key_byte, iv_byte);
    delete md5;
}

void Device::UnpackHandshakeMsg(const unsigned char *msg_buf, Device::Msg *msg) {
    memcpy(&(msg->did), &(msg_buf[8]), 4);
    CharToUint32BigEndian(&(msg_buf[12]), &(msg->stamp));
}

void Device::UnpackMsgHeader(const uint8_t *msg_buf, Device::Msg *msg) {
    CharToUint16BigEndian(&(msg_buf[2]), &(msg->length));
    memcpy(&(msg->did), &(msg_buf[8]), 4);
    CharToUint32BigEndian(&(msg_buf[12]), &(msg->stamp));
    memcpy(&(msg->md5_checksum), &(msg_buf[16]), 16);
}

void Device::DecryptPayloads(uint8_t *payloads, uint16_t payloads_len) {
    AES_ctx_set_iv(&ctx_, iv_);
    AES_CBC_decrypt_buffer(&ctx_, payloads, payloads_len);
}

void Device::EncryptPayloads(uint8_t *payloads, uint16_t payloads_len) {
    AES_ctx_set_iv(&ctx_, iv_);
    AES_CBC_encrypt_buffer(&ctx_, payloads, payloads_len);
}

void Device::PackMsgHeader(const Device::Msg *msg, uint8_t *msg_buf) {
    msg_buf[0] = 0x21;
    msg_buf[1] = 0x31;
    Uint16ToCharBigEndian(&(msg->length), &(msg_buf[2]));
    memset(&(msg_buf[4]), 0, 4);
    memcpy(&(msg_buf[8]), &(msg->did), 4);
    Uint32ToCharBigEndian(&(msg->stamp), &(msg_buf[12]));
    memcpy(&(msg_buf[16]), &(msg->md5_checksum), 16);
}

void Device::FillMd5Checksum(unsigned char *msg_buf, u_int32_t packet_length) {
    string md5_checksum_str;
    md5_checksum_str.append((char*)msg_buf, packet_length);
    MD5* md5;
    md5 = new MD5(md5_checksum_str);
    //const unsigned char* md5_checksum_byte = md5->getDigest();
    memcpy(&(msg_buf[16]), md5->getDigest(), 16);
    delete md5;
}

void Device::MakePayloads16Multiple(string *payloads_send) {
    const char empty[15] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    u_int32_t len = (u_int32_t)payloads_send->length();
    int remain = len % 16;
    if (remain != 0)
    {
        remain = 16 - remain;
        payloads_send->append(empty,remain);
    }
}

void Device::GetPayloadsActualLength(uint8_t *payloads, uint16_t *payloads_len) {
    while (payloads[*payloads_len - 1] != 0x7d && payloads[*payloads_len - 1] != 0)
    {
        (*payloads_len)--;
    }
}
