// Copyright (c) 2025 Mark Godwin.
// SPDX-License-Identifier: MIT

#pragma once

#include "blockStorage.h"
#include "NeoPixel.h"
#include "Miku.h"

#define SAVE_DELAY 120000

struct WifiConfig
{
    char ssid[36];
    char password[64];
};

struct MqttConfig
{
    char brokerAddress[16];
    uint16_t port;
    char username[32];
    char password[64];
    char topic[128];
};

struct PatternConfig
{
    char patternName[48];
    neopixel pixels[PIXEL_COUNT];
    int32_t nextFrameId; // ID of the next frame in the animation, -1 if no next frame
    int32_t frameTime; // Duration of this frame in milliseconds
    int32_t transitionTime;
};


bool operator==(const PatternConfig &left, const PatternConfig &right);

class DeviceConfig
{
    public:
        DeviceConfig(uint32_t storageSize, uint32_t blockSize);

        const WifiConfig *GetWifiConfig();
        void SaveWifiConfig(const WifiConfig *wifiConfig);

        const MqttConfig *GetMqttConfig();
        void SaveMqttConfig(const MqttConfig *mqttConfig);

        /// @brief Get the IDs of all the registered patterns
        /// @param count [out] number of patterns registered
        const uint16_t *GetPatternIds(uint32_t *count);
        void SavePatternIds(const uint16_t *patternIds, uint32_t count);

        const PatternConfig *GetPatternConfig(uint16_t patternId);
        void SavePatternConfig(uint16_t patternId, const PatternConfig *patternConfig);
        void DeletePatternConfig(uint16_t patternId);

        void HardReset();

    private:
        DeviceConfig(const DeviceConfig &) = delete;

        void SaveIdList(uint32_t header, const uint16_t *ids, uint32_t count);
        const uint16_t *GetIdList(uint32_t header, uint32_t *count);
        void SaveIdList32(uint32_t header, const uint32_t *ids, uint32_t count);
        const uint32_t *GetIdList32(uint32_t header, uint32_t *count);

        BlockStorage _storage;        
};
