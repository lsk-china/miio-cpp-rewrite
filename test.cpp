//
// Created by lsk on 12/4/23.
//
#include "Device.h"
#include <iostream>

using namespace std;

class YeeLight : public Device {
public:
    YeeLight(const char *ip, const char *token) : Device(ip, token) {}
    void printStatus() {
        int respLength;
        const char *params[] = {"name", "lan_ctrl", "save_state", "delayoff", "music_on", "power", "bright", "color_mode", "rgb", "hue", "sat", "ct", "flowing", "flow_params", "active_mode", "nl_br", "bg_power", "bg_bright", "bg_lmode", "bg_rgb", "bg_hue", "bg_sat", "bg_ct", "bg_flowing", "bg_flow_params"};
        cout << this->send("get_prop", params, 25, respLength) << endl;
    }
};

int main() {
    YeeLight yeeLight("192.168.1.6", "7576e70479144b341cf0e4a44aa2766b");
    yeeLight.printStatus();
}