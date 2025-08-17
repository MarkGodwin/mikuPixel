
#include "debug.h"

#include "PatternEditor.h"
#include "EditableImage.h"
#include "AnimationRunner.h"

PatternEditor::PatternEditor(uint16_t patternId, const neopixel *pixels, uint32_t pixelCount, AnimationRunner *animationRunner, std::shared_ptr<WebServer> webServer)
:   _patternId(patternId),
    _editableImage(std::make_shared<EditableImage>(pixels, pixelCount)),
    _setLedSubscription(MakeCgiSubscription<bool>(webServer, "/api/patterns/setLed.json", [this](const CgiParams &params) { return OnSetLed(params);}))
{
    animationRunner->SetAnimation(_editableImage);
}

bool PatternEditor::OnSetLed(const CgiParams &params)
{
    auto pixelIndex = params.find("index");
    if(pixelIndex == params.end())
        return false;

    auto rparam = params.find("r");
    auto gparam = params.find("g");
    auto bparam = params.find("b");
    if(rparam == params.end() || gparam == params.end() || bparam == params.end())
        return false;

    int index = std::stoul(pixelIndex->second);
    auto rgb = neopixel(std::stoul(rparam->second), std::stoul(gparam->second), std::stoul(bparam->second));

    return _editableImage->SetPixel(index, rgb);
}