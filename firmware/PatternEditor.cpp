
#include "debug.h"

#include "PatternEditor.h"
#include "EditableImage.h"
#include "AnimationRunner.h"

PatternEditor::PatternEditor(uint16_t patternId, const neopixel *pixels, uint32_t pixelCount, AnimationRunner *animationRunner, std::shared_ptr<WebServer> webServer)
:   _patternId(patternId),
    _editableImage(std::make_shared<EditableImage>(pixels, pixelCount)),
    _setLedSubscription(webServer, "/api/setled.json", [this](const CgiParams &params) { return OnSetLed(params);})
{
    animationRunner->SetAnimation(_editableImage);
}

bool PatternEditor::OnSetLed(const CgiParams &params)
{
    auto pixelIndex = params.find("index");
    if(pixelIndex == params.end())
        return false;

    auto colour = params.find("colour");
    if(colour == params.end())
        return false;

    int index = std::stoul(pixelIndex->second);

    auto rgb = neopixel(std::stoul(colour->second, 0, 16));

    return _editableImage->SetPixel(index, rgb);
}