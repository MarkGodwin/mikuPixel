// Copyright (c) 2023 Mark Godwin.
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "debug.h"
#include "pico/flash.h"
#include "pico/malloc.h"

#include "deviceConfig.h"
#include "blockStorage.h"

static const uint32_t wifiConfigMagic = 0x19841984;
static const uint32_t mqttConfigMagic = 0x19841985;
static const uint32_t patternsConfigMagic = 0xDEADBEEF;
static const uint32_t patternConfigMagic = 0xBEEF0000;

DeviceConfig::DeviceConfig(uint32_t storageSize, uint32_t blockSize)
:   _storage(PICO_FLASH_SIZE_BYTES - storageSize, storageSize, blockSize)
{
}

const WifiConfig * DeviceConfig::GetWifiConfig()
{
    auto creds = (const WifiConfig *)_storage.GetBlock(wifiConfigMagic);
    return creds;
}

void DeviceConfig::SaveWifiConfig(const WifiConfig *config)
{
    _storage.SaveBlock(wifiConfigMagic, (const uint8_t *)config, sizeof(WifiConfig));
}

const MqttConfig *DeviceConfig::GetMqttConfig()
{
    auto creds = (const MqttConfig *)_storage.GetBlock(mqttConfigMagic);
    return creds;
}

void DeviceConfig::SaveMqttConfig(const MqttConfig *mqttConfig)
{
    _storage.SaveBlock(mqttConfigMagic, (const uint8_t *)mqttConfig, sizeof(MqttConfig));
}

void DeviceConfig::SavePatternIds(const uint16_t *blindIds, uint32_t count)
{
    SaveIdList(patternsConfigMagic, blindIds, count);
}

const uint16_t *DeviceConfig::GetPatternIds(uint32_t *count)
{
    return GetIdList(patternsConfigMagic, count);
}

const PatternConfig *DeviceConfig::GetPatternConfig(uint16_t patternId)
{
    return (PatternConfig *)_storage.GetBlock(patternConfigMagic | patternId);
}

void DeviceConfig::SavePatternConfig(uint16_t patternId, const PatternConfig *patternConfig)
{
    // Don't save if there are no changes, to avoid flash wear
    auto existingCfg = GetPatternConfig(patternId);
    if(existingCfg &&
        *patternConfig == *existingCfg)
    {
        DBG_PUT("No changes to save");
        return;
    }
    DBG_PRINT("Saving pattern config for %d\n", patternId);
    _storage.SaveBlock(patternConfigMagic | patternId, (const uint8_t *)patternConfig, sizeof(*patternConfig));
    DBG_PUT("Pattern config saved\n");
}

void DeviceConfig::DeletePatternConfig(uint16_t patternId)
{
    _storage.ClearBlock(patternConfigMagic | patternId);
}

void DeviceConfig::HardReset()
{
        // The flash storage has no wifi config. Format it to ensure it is empty, and
        // store default device config
        _storage.Format();

        WifiConfig cfg;
        memset(&cfg, 0, sizeof(WifiConfig));
        DBG_PUT("Saving WiFi config\n");
        SaveWifiConfig(&cfg);

        MqttConfig mcfg;
        memset(&mcfg, 0, sizeof(mcfg));
        strcpy(mcfg.brokerAddress, "");
        mcfg.port = 1883;
        DBG_PUT("Saving Mqtt config\n");
        SaveMqttConfig(&mcfg);

}

void DeviceConfig::SaveIdList(uint32_t header, const uint16_t *ids, uint32_t count)
{
    DBG_PRINT("Saving %d IDs to header %08x\n", count, header);

    size_t bytes = sizeof(count) + sizeof(uint16_t) * count;
    if(bytes > _storage.BlockSize())
        DBG_PUT("Too many ids to fit in the block. FAIL\n");

    auto buf = (uint8_t *)malloc(bytes);
    if(buf == nullptr)
    {
        DBG_PRINT("Failed to allocate a buffer of %d bytes to save ID list\n", bytes);
        return;
    }
    memcpy(buf, &count, sizeof(count));
    memcpy(buf + sizeof(count), ids, sizeof(uint16_t) * count);
    _storage.SaveBlock(header, buf, bytes);
    free(buf);
}

void DeviceConfig::SaveIdList32(uint32_t header, const uint32_t *ids, uint32_t count)
{
    size_t bytes = sizeof(count) + sizeof(uint32_t) * count;
    if(bytes > _storage.BlockSize())
        DBG_PUT("Too many ids to fit in the block. FAIL\n");

    auto buf = (uint8_t *)malloc(bytes);
    if(buf == nullptr)
    {
        DBG_PRINT("Failed to allocate a buffer of %d bytes to save ID list\n", bytes);
        return;
    }
    memcpy(buf, &count, sizeof(count));
    memcpy(buf + sizeof(count), ids, sizeof(uint32_t) * count);
    _storage.SaveBlock(header, buf, bytes);
    free(buf);
}

const uint16_t *DeviceConfig::GetIdList(uint32_t header, uint32_t *count)
{
    auto block = (const uint32_t *)_storage.GetBlock(header);
    if(block == nullptr)
    {
        *count = 0;
        return nullptr;
    }
    *count = *block;
    return (const uint16_t *)(block + 1);
}

const uint32_t *DeviceConfig::GetIdList32(uint32_t header, uint32_t *count)
{
    auto block = (const uint32_t *)_storage.GetBlock(header);
    if(block == nullptr)
    {
        *count = 0;
        return nullptr;
    }
    *count = *block;
    return (block + 1);
}

bool operator==(const PatternConfig &left, const PatternConfig &right)
{
    return !strcmp(left.patternName, right.patternName) &&
        !memcmp(left.pixels, right.pixels, sizeof(left.pixels)) &&
        left.nextFrameId == right.nextFrameId &&
        left.frameTime == right.frameTime &&
        left.transitionTime == right.transitionTime;
}
