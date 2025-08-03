
#include "debug.h"
#include "PatternList.h"
#include "PatternEditor.h"
#include "deviceConfig.h"
#include "bufferOutput.h"

PatternList::PatternList(std::shared_ptr<WebServer> webServer, std::shared_ptr<DeviceConfig> deviceConfig, std::shared_ptr<AnimationRunner> animationRunner)
:   _webServer(std::move(webServer)),
    _deviceConfig(std::move(deviceConfig)),
    _animationRunner(std::move(animationRunner)),
    _getPattern(nullptr)
{
    _nextId = 0;
    uint32_t count;
    const uint16_t *patternIds = _deviceConfig->GetPatternIds(&count);
    if(count > 0)
        _nextId = *std::max_element(patternIds, patternIds + count) + 1;

    _cgiHandlers.push_back(CgiSubscription(_webServer, "/api/patterns/beginEdit.json", [this](const CgiParams &params) { return BeginEdit(params); }));
    _cgiHandlers.push_back(CgiSubscription(_webServer, "/api/patterns/endEdit.json", [this](const CgiParams &params) { return EndEdit(params); }));
    _cgiHandlers.push_back(CgiSubscription(_webServer, "/api/patterns/get.json", [this](const CgiParams &params) { return SaveGetParams(params);} ));
    _cgiHandlers.push_back(CgiSubscription(_webServer, "/api/patterns/add.json", [this](const CgiParams &params) { return AddPattern(params); }));

    _ssiHandlers.push_back(SsiSubscription(_webServer, "patterns", [this](char *pcInsert, int iInsertLen, uint16_t tagPart, uint16_t *nextPart) { return HandlePatternsResponse(pcInsert, iInsertLen, tagPart, nextPart); }));
    _ssiHandlers.push_back(SsiSubscription(_webServer, "pattern", [this](char *pcInsert, int iInsertLen, uint16_t tagPart, uint16_t *nextPart) { return HandlePatternResponse(pcInsert, iInsertLen, tagPart, nextPart); }));
    _ssiHandlers.push_back(SsiSubscription(_webServer, "newid", [this](char *pcInsert, int iInsertLen, uint16_t tagPart, uint16_t *nextPart) { return HandleAddResponse(pcInsert, iInsertLen, tagPart, nextPart); }));
}

int16_t PatternList::HandlePatternsResponse(char *pcInsert, int iInsertLen, uint16_t tagPart, uint16_t *nextPart)
{
    uint32_t count = 0;
    auto patternIds = _deviceConfig->GetPatternIds(&count);

    if(tagPart >= count)
    {
        return 0; // No more patterns
    }

    auto patternId = patternIds[tagPart];
    auto pattern = _deviceConfig->GetPatternConfig(patternId);
    if(!pattern)
    {
        return 0; // No such pattern
    }

    BufferOutput outputter(pcInsert, iInsertLen);
    outputter.Append("{ \"id\": ");
    outputter.Append((int)patternId);
    outputter.Append(", \"name\": \"");
    outputter.AppendEscaped(pattern->patternName);

    if(tagPart + 1 < count)
    {
        outputter.Append("\" },");
        *nextPart = tagPart + 1;
    }
    else
        outputter.Append("\" }");
    
    return outputter.BytesWritten();
}

int16_t PatternList::HandlePatternResponse(char *pcInsert, int iInsertLen, uint16_t tagPart, uint16_t *nextPart)
{
    if(_getPattern == nullptr)
    {
        return 0; // Pattern being read doesn't exist
    }

    if(tagPart == 0)
    {
        BufferOutput outputter(pcInsert, iInsertLen);
        outputter.Append("{ \"name\": \"");
        outputter.AppendEscaped(_getPattern->patternName);
        outputter.Append("\", \"pixels\": [");
        return outputter.BytesWritten();
    }

    auto pixelIndex = tagPart - 1;
    if(pixelIndex >= PIXEL_COUNT)
        return 0;

    auto &pixel = _getPattern->pixels[pixelIndex];
    BufferOutput outputter(pcInsert, iInsertLen);
    outputter.Append((int)pixel.colour);
    
    if(pixelIndex < PIXEL_COUNT - 1)
    {
        outputter.Append(",");
        *nextPart = tagPart + 1;
    }
    else
    {
        outputter.Append(" }");
        _getPattern = nullptr;
    }

    return outputter.BytesWritten();
}

int16_t PatternList::HandleAddResponse(char *pcInsert, int iInsertLen, uint16_t tagPart, uint16_t *nextPart)
{
    BufferOutput outputter(pcInsert, iInsertLen);
    outputter.Append((int)_newPatternId);
    return outputter.BytesWritten();
}

bool PatternList::AddPattern(const CgiParams &params)
{
    auto patternName = params.find("name");
    if(patternName == params.end())
        return false;

    PatternConfig newPattern;
    strlcpy(newPattern.patternName, patternName->second.c_str(), sizeof(newPattern.patternName));

    // Initialize pixels to black
    for(auto &pixel : newPattern.pixels)
    {
        pixel = neopixel(0, 0, 0);
    }

    // Save the new pattern with a new ID
    uint32_t count = 0;
    _newPatternId = _nextId++; // TODO: Really need to add flexibility for crappy web server framework return value type
    _deviceConfig->SavePatternConfig(_newPatternId, &newPattern);

    return true;
}

bool PatternList::BeginEdit(const CgiParams &params)
{
    auto patternId = params.find("id");
    if(patternId == params.end())
        return false;

    uint16_t id = std::stoul(patternId->second);

    if(_currentEditor && _currentEditor->GetPatternId() == id)
        return true; // Already editing this pattern

    auto pattern = _deviceConfig->GetPatternConfig(id);
    if(!pattern)
        return false; // No such pattern

    _currentEditor = std::make_unique<PatternEditor>(id, pattern->pixels, PIXEL_COUNT, _animationRunner.get(), _webServer);
    return true;
}

bool PatternList::EndEdit(const CgiParams &params)
{
    if(!_currentEditor)
        return false;

    auto patternId = params.find("id");
    if(patternId == params.end())
        return false;
    auto patternName = params.find("name");
    if(patternName == params.end())
        return false;

    uint16_t id = std::stoul(patternId->second);
    if(id != _currentEditor->GetPatternId())
        return false; // Pattern ID mismatch

    PatternConfig newConfig;
    std::copy(_currentEditor->GetPixels().begin(), _currentEditor->GetPixels().end(), newConfig.pixels);
    strlcpy(newConfig.patternName, patternName->second.c_str(), sizeof(newConfig.patternName));
    _deviceConfig->SavePatternConfig(id, &newConfig);

    _currentEditor.reset();
    return true;
}

bool PatternList::SaveGetParams(const CgiParams &params)
{
    auto patternId = params.find("id");
    if(patternId == params.end())
        return false;
    auto getId = (uint16_t)std::stoul(patternId->second);

    _getPattern = _deviceConfig->GetPatternConfig(getId);
    return _getPattern != nullptr; // Return true if pattern exists
}

