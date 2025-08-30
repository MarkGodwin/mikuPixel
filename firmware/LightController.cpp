#include "mikuPixel.h"
#include <algorithm>
#include "LightController.h"

#include "MikuLight.h"

LightController::LightController(
    std::shared_ptr<MikuLight> mikuLight,
    std::shared_ptr<WebServer> webServer,
    std::shared_ptr<MqttClient> mqttClient)
    : _mikuLight(std::move(mikuLight)),
    _webServer(std::move(webServer)),
    _mqttClient(std::move(mqttClient))
{

    // Set up web endpoints
    _cgiHandlers.push_back(MakeCgiSubscription<bool>(_webServer, "/api/setRgb.json", [this](const CgiParams &params) { return OnSetRgbCgi(params); }));
    _cgiHandlers.push_back(MakeCgiSubscription<bool>(_webServer, "/api/setBrightness.json", [this](const CgiParams &params) { return OnSetBrightnessCgi(params); }));
    _cgiHandlers.push_back(MakeCgiSubscription<bool>(_webServer, "/api/activatePattern.json", [this](const CgiParams &params) { return ActivatePatternCgi(params); }));
    _cgiHandlers.push_back(MakeCgiSubscription<bool>(_webServer, "/api/activateAnimation.json", [this](const CgiParams &params) { return ActivateAnimationCgi(params); }));

    _ssiHandlers.push_back(SsiSubscription(_webServer, "anims", [this](char *pcInsert, int iInsertLen, uint16_t tagPart, uint16_t *nextPart) { return HandleAnimationsResponse(pcInsert, iInsertLen, tagPart, nextPart); }));

    DBG_PUT("Registering Light MQTT handlers 1\n");

    _mqttHandlers.push_back(MqttSubscription(_mqttClient, string_format("miku/%s/sw/cmd", macAddress), [this](const uint8_t *payload, uint32_t length)
                {
                    OnSwitchCommand(payload, length);
                }));
    _mqttHandlers.push_back(MqttSubscription(_mqttClient, string_format("miku/%s/br/cmd", macAddress), [this](const uint8_t *payload, uint32_t length)
                {
                    OnBrightnessCommand(payload, length);
                }));
    _mqttHandlers.push_back(MqttSubscription(_mqttClient, string_format("miku/%s/hs/cmd", macAddress), [this](const uint8_t *payload, uint32_t length)
                {
                    OnHsCommand(payload, length);
                }));
    _mqttHandlers.push_back(MqttSubscription(_mqttClient, string_format("miku/%s/wh/cmd", macAddress), [this](const uint8_t *payload, uint32_t length)
                {
                    OnWhiteCommand(payload, length);
                }));
    _mqttHandlers.push_back(MqttSubscription(_mqttClient, string_format("miku/%s/fx/cmd", macAddress), [this](const uint8_t *payload, uint32_t length)
                 { OnEffectCommand(payload, length); })
    );

}

bool LightController::OnSetRgbCgi(const CgiParams &params)
{
    auto rParam = params.find("r");
    auto gParam = params.find("g");
    auto bParam = params.find("b");

    if(rParam != params.end() && gParam != params.end() && bParam != params.end())
    {
        auto r = std::stoi(rParam->second);
        auto g = std::stoi(gParam->second);
        auto b = std::stoi(bParam->second);
        _mikuLight->SetRgb(r, g, b);
        return true;
    }
    return false;
}


bool LightController::OnSetBrightnessCgi(const CgiParams &params)
{
    auto brightnessParam = params.find("value");
    if(brightnessParam == params.end())
        return false;

    int brightness = std::stoi(brightnessParam->second);

    // Set a solid colour animation with the specified brightness
    _mikuLight->SetMikuBrightness(brightness);
    return true;
}

bool LightController::ActivatePatternCgi(const CgiParams &params)
{
    auto patternId = params.find("id");
    if(patternId == params.end())
        return false;

    uint16_t id = std::stoul(patternId->second);
    return _mikuLight->ActivatePattern(id);

}

bool LightController::ActivateAnimationCgi(const CgiParams &params)
{
    auto animationId = 0;
    auto animationIdParam = params.find("id");
    if(animationIdParam != params.end())
        animationId = std::stoi(animationIdParam->second);
    else
    {
        auto animationName = params.find("name");
        if(animationName == params.end())
            return false;

        const auto &name = animationName->second;
        animationId = _mikuLight->GetAnimationByName(name);
        if(animationId < 0)
            return false;
    }

    return _mikuLight->StartAnimation(animationId);
}

void LightController::OnSwitchCommand(const uint8_t *payload, uint32_t length)
{
    // Expecting a string payload like "ON" or "OFF"
    if(!memcmp(payload, "ON", 2) != 0)
    {
        DBG_PUT("Received ON command\n");
        _mikuLight->SwitchOn();
    }
    else if(!memcmp(payload, "OFF", 3) != 0)
    {
        DBG_PUT("Received OFF command\n");
        _mikuLight->SwitchOff();
    }
    else
    {
        DBG_PRINT("Unknown switch command: %.*s\n", length, payload);
        return;
    }
}

void LightController::OnBrightnessCommand(const uint8_t *payload, uint32_t length)
{
    // Expecting a non-terminated string payload with a number between 0 and 255
    auto value = std::string((const char *)payload, length);
    DBG_PRINT("Received Brightness command: %s\n", value.c_str());
    int brightness = std::stoi(value);

    _mikuLight->SetBrightness(brightness);
}

// Part of an HSB command set. General brightness is set through white command
void LightController::OnHsCommand(const uint8_t *payload, uint32_t length)
{
    // Expecting a non-terminated string payload like "255,0,127"
    std::string rgbStr((const char *)payload, length);
    DBG_PRINT("Received HS command: %s\n", rgbStr.c_str());
    auto parts = std::count(rgbStr.begin(), rgbStr.end(), ',');
    if(parts != 1)
    {
        DBG_PRINT("Invalid HS command (wrong number of commas): %s\n", rgbStr.c_str());
        return;
    }

    auto firstComma = rgbStr.find(',');
    if(firstComma == std::string::npos)
    {
        DBG_PRINT("Invalid HS command (missing commas): %s\n", rgbStr.c_str());
        return;
    }

    float hue = std::clamp(std::stof(rgbStr.substr(0, firstComma)), 0.0f, 360.0f);
    float sat = std::clamp(std::stof(rgbStr.substr(firstComma + 1)), 0.0f, 100.0f);

    _mikuLight->SetHueAndSaturation(hue, sat);
}

// Revert the light to "white" (pattern mode)
void LightController::OnWhiteCommand(const uint8_t *payload, uint32_t length)
{
    // Expecting a non-terminated string payload with a number between 0 and 255
    auto value = std::string((const char *)payload, length);
    DBG_PRINT("Received White command: %s\n", value.c_str());
    int brightness = std::stoi(value);

    _mikuLight->SetMikuBrightness(brightness);
}

void LightController::OnEffectCommand(const uint8_t *payload, uint32_t length)
{
    // Expecting a string payload like "solid"
    std::string effectName;
    effectName.resize(length);
    memcpy(effectName.data(), payload, length);

    auto animationId = _mikuLight->GetAnimationByName(effectName);
    if(animationId >= 0)
        _mikuLight->StartAnimation(animationId);
}


int16_t LightController::HandleAnimationsResponse(char *pcInsert, int iInsertLen, uint16_t tagPart, uint16_t *nextPart)
{
    const auto &animations = _mikuLight->GetAvailableAnimations();

    if(tagPart == 0)
    {
        DBG_PRINT("Returning %d patterns\n", animations.size());
    }

    if(tagPart >= animations.size())
    {
        return 0; // No more patterns
    }

    const auto &[shortName, animationName] = animations[tagPart];

    BufferOutput outputter(pcInsert, iInsertLen);
    outputter.Append("{ \"id\": ");
    outputter.Append((int)tagPart);
    outputter.Append(", \"name\": \"");
    outputter.AppendEscaped(animationName.c_str());
    outputter.Append("\" ");

    if(tagPart + 1 < animations.size())
    {
        outputter.Append(" },");
        *nextPart = tagPart + 1;
    }
    else
        outputter.Append(" }");
    
    return outputter.BytesWritten();
}


