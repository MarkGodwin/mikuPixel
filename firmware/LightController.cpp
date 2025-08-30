#include "mikuPixel.h"
#include "LightController.h"

#include "animations/SolidMikuAnimation.h"
#include "animations/SolidColourAnimation.h"
#include <algorithm>

LightController::LightController(
    std::shared_ptr<WebServer> webServer,
    std::shared_ptr<MqttClient> mqttClient,
    std::shared_ptr<AnimationRunner> animationRunner)
    : _webServer(std::move(webServer)),
    _mqttClient(std::move(mqttClient)),
    _animationRunner(std::move(animationRunner))
{

    // Set up web endpoints
    _cgiHandlers.push_back(MakeCgiSubscription<bool>(_webServer, "/api/setRgb.json", [this](const CgiParams &params) { return OnSetRgbCgi(params); }));
    _cgiHandlers.push_back(MakeCgiSubscription<bool>(_webServer, "/api/setBrightness.json", [this](const CgiParams &params) { return OnSetBrightnessCgi(params); }));

    //_ssiHandlers.push_back(SsiSubscription(_webServer, "anims", [this](char *pcInsert, int iInsertLen, uint16_t tagPart, uint16_t *nextPart) { return HandleAnimationsResponse(pcInsert, iInsertLen, tagPart, nextPart); }));

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
        SetRgb(r, g, b);
        return true;
    }
    return false;
}

void LightController::SetRgb(int r, int g, int b)
{
    r = std::clamp(r, 0, 255);
    g = std::clamp(g, 0, 255);
    b = std::clamp(b, 0, 255);
    _animationRunner->SetAnimation(std::make_shared<SolidColourAnimation>(neopixel(r, g, b).gammaCorrected()));

    auto [h, s, br] = RGBtoHSB(r, g, b);

    auto hs = string_format("%.1f,%.1f", h, s);
    _hue = std::round(h * 10.0f) / 10.0f;
    _saturation = std::round(s * 10.0f) / 10.0f;
    _brightness = br;
    auto brval = std::to_string(br);
    _mqttClient->Publish(string_format("miku/%s/hs/state", macAddress).c_str(), (const uint8_t *)hs.data(), hs.length(), true);
    _mqttClient->Publish(string_format("miku/%s/br/state", macAddress).c_str(), (const uint8_t *)brval.data(), brval.length(), true);
    _mqttClient->Publish(string_format("miku/%s/fx/state", macAddress).c_str(), (const uint8_t *)"solid", 5, true);

    if(r > 0 || g > 0 || b > 0)
    {
        _on = true;
        _mqttClient->Publish(string_format("miku/%s/sw/state", macAddress).c_str(), (const uint8_t *)"ON", 2, true);
    }
    else
    {
        _on = false;
        _mqttClient->Publish(string_format("miku/%s/sw/state", macAddress).c_str(), (const uint8_t *)"OFF", 3, true);
    }
}

bool LightController::OnSetBrightnessCgi(const CgiParams &params)
{
    auto brightnessParam = params.find("value");
    if(brightnessParam == params.end())
        return false;

    int brightness = std::stoi(brightnessParam->second);

    // Set a solid colour animation with the specified brightness
    SetPatternBrightness(brightness);
    return true;
}

void LightController::SetPatternBrightness(int brightness)
{
    brightness = std::clamp(brightness, 0, 255);
    _animationRunner->SetAnimation(std::make_shared<SolidMikuAnimation>(brightness));

    if(brightness > 0)
    {
        _brightness = brightness;
        auto br = std::to_string(brightness);
        auto hs = string_format("%.0f,0", _hue);
        _saturation = 0.0f;
        _mqttClient->Publish(string_format("miku/%s/br/state", macAddress).c_str(), (const uint8_t *)br.data(), br.length(), true);
        _mqttClient->Publish(string_format("miku/%s/hs/state", macAddress).c_str(), (const uint8_t *)hs.data(), hs.length(), true);
        _mqttClient->Publish(string_format("miku/%s/fx/state", macAddress).c_str(), (const uint8_t *)"solid", 5, true);

        _on = true;
        _mqttClient->Publish(string_format("miku/%s/sw/state", macAddress).c_str(), (const uint8_t *)"ON", 2, true);
    }
    else
    {
        _on = false;
        _mqttClient->Publish(string_format("miku/%s/fx/state", macAddress).c_str(), nullptr, 0, true);
        _mqttClient->Publish(string_format("miku/%s/sw/state", macAddress).c_str(), (const uint8_t *)"OFF", 3, true);
    }

}

void LightController::OnSwitchCommand(const uint8_t *payload, uint32_t length)
{
    // Expecting a string payload like "ON" or "OFF"
    if(!memcmp(payload, "ON", 2) != 0)
    {
        DBG_PUT("Received ON command\n");
        if(!_on)
        {
            // Restore the previous colour
            if(_saturation == 0.0f)
                SetPatternBrightness(_brightness);
            else
            {
                auto [r, g, b] = HSBtoRGB(_hue, _saturation, _brightness);
                SetRgb(r, g, b);
            }
        }

    }
    else if(!memcmp(payload, "OFF", 3) != 0)
    {
        DBG_PUT("Received OFF command\n");
        SetPatternBrightness(0);
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

    if(_saturation == 0.0f)
    {
        SetPatternBrightness(brightness);
    }
    else
    {
        auto [r, g, b] = HSBtoRGB(_hue, _saturation, brightness);
        SetRgb(r, g, b);
    }
}

// Part of an HSB command set. General brightness is set through white command
void LightController::OnHsCommand(const uint8_t *payload, uint32_t length)
{
    // Expecting a non-terminated string payload like "255,0,127"
    std::string rgbStr((const char *)payload, length);
    DBG_PRINT("Received RGB command: %s\n", rgbStr.c_str());
    auto parts = std::count(rgbStr.begin(), rgbStr.end(), ',');
    if(parts != 1)
    {
        DBG_PRINT("Invalid RGB command (wrong number of commas): %s\n", rgbStr.c_str());
        return;
    }

    auto firstComma = rgbStr.find(',');
    if(firstComma == std::string::npos)
    {
        DBG_PRINT("Invalid RGB command (missing commas): %s\n", rgbStr.c_str());
        return;
    }

    float hue = std::clamp(std::stof(rgbStr.substr(0, firstComma)), 0.0f, 360.0f);
    float sat = std::clamp(std::stof(rgbStr.substr(firstComma + 1)), 0.0f, 100.0f);

    if(sat == 0.0f)
    {
        SetPatternBrightness(_brightness);
    }
    else
    {
        auto [r, g, b] = HSBtoRGB(hue, sat, _brightness);
        SetRgb(r, g, b);
    }
}

// Revert the light to "white" (pattern mode)
void LightController::OnWhiteCommand(const uint8_t *payload, uint32_t length)
{
    // Expecting a non-terminated string payload with a number between 0 and 255
    auto value = std::string((const char *)payload, length);
    DBG_PRINT("Received White command: %s\n", value.c_str());
    int brightness = std::stoi(value);

    SetPatternBrightness(brightness);
}


// HA messages work better with HSB, as RGB is treated like a hue scaled to full brightness
std::tuple<int,int,int> LightController::HSBtoRGB(float hue, float sat, int brightness)
{
    float s = std::clamp(sat / 100.0f, 0.0f, 1.0f);
    float v = std::clamp(brightness / 255.0f, 0.0f, 1.0f);

    float C = v * s;                           // chroma
    float X = C * (1 - fabsf(fmodf(hue / 60.0f, 2.0f) - 1));
    float m = v - C;

    float r=0, g=0, b=0;
    if(hue < 60)      { r = C; g = X; b = 0; }
    else if(hue < 120){ r = X; g = C; b = 0; }
    else if(hue < 180){ r = 0; g = C; b = X; }
    else if(hue < 240){ r = 0; g = X; b = C; }
    else if(hue < 300){ r = X; g = 0; b = C; }
    else            { r = C; g = 0; b = X; }

    int R = static_cast<int>(std::round((r + m) * 255));
    int G = static_cast<int>(std::round((g + m) * 255));
    int B_out = static_cast<int>(std::round((b + m) * 255));

    return {R, G, B_out};
}

// Convert RGB -> HSB
inline std::tuple<float,float,int> LightController::RGBtoHSB(int red, int green, int blue)
{
    float r = red / 255.0f;
    float g = green / 255.0f;
    float b = blue / 255.0f;

    float maxC = std::max({r,g,b});
    float minC = std::min({r,g,b});
    float delta = maxC - minC;

    float hue = 0.0f;
    if(delta != 0.0f) {
        if(maxC == r)      hue = 60.0f * fmodf(((g - b) / delta), 6.0f);
        else if(maxC == g) hue = 60.0f * (((b - r) / delta) + 2.0f);
        else if(maxC == b) hue = 60.0f * (((r - g) / delta) + 4.0f);
    }
    if(hue < 0) hue += 360.0f;

    float sat = (maxC == 0 ? 0 : delta / maxC);
    float brightness = maxC;

    return {hue, sat * 100.0f, static_cast<int>(std::round(brightness * 255))};
}