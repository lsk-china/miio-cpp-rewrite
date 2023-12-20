//
// Created by lsk on 12/20/23.
//

#include "MiioDevice.h"
#include "Device.h"
#include <cstring>
#include <cstdlib>
#include <algorithm>

Device *device;

std::vector<const char*> strlist(std::vector<std::string> &input) {
    std::vector<const char*> result;

    // remember the nullptr terminator
    result.reserve(input.size()+1);

    std::transform(begin(input), end(input),
                   std::back_inserter(result),
                   [](std::string &s) { return s.data(); }
    );
    result.push_back(nullptr);

    return result;
}

MiioDevice::MiioDevice(string ip, string token) {
    device = new Device(ip.c_str(), token.c_str());
}

string MiioDevice::sendCommand(string command, vector<string> params) {
    int respLength;
    return (device->send(command.c_str(), strlist(params).data(), params.size(), respLength));
}

MiioDevice::~MiioDevice() {
    delete device;
}
