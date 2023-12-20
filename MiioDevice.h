//
// Created by lsk on 12/20/23.
//

#ifndef MIIO_CPP_REWRITE_MIIODEVICE_H
#define MIIO_CPP_REWRITE_MIIODEVICE_H

#include <string>
#include <vector>

using namespace std;

class MiioDevice {
public:
    MiioDevice(string ip, string token);
    ~MiioDevice();
    string sendCommand(string command, vector<string> params);
};


#endif //MIIO_CPP_REWRITE_MIIODEVICE_H
