#pragma once

#include <memory>
#include <list>
#include "webServer.h"

#include "PatternEditor.h"

class AnimationRunner;
class PatternConfig;

class PatternList
{
public:
    PatternList(std::shared_ptr<WebServer> webServer, std::shared_ptr<DeviceConfig> deviceConfig, std::shared_ptr<AnimationRunner> animationRunner);

    // Disable copy constructor and assignment operator
    PatternList(const PatternList &) = delete;
    PatternList &operator=(const PatternList &) = delete;
    PatternList(PatternList &&) = delete;
    PatternList &operator=(PatternList &&) = delete;

    ~PatternList() = default;

private:
    int16_t HandlePatternsResponse(char *pcInsert, int iInsertLen, uint16_t tagPart, uint16_t *nextPart);
    int16_t HandlePatternResponse(char *pcInsert, int iInsertLen, uint16_t tagPart, uint16_t *nextPart);
    int16_t HandleAddResponse(char *pcInsert, int iInsertLen, uint16_t tagPart, uint16_t *nextPart);

    bool AddPattern(const CgiParams &params);
    bool BeginEdit(const CgiParams &params);
    bool EndEdit(const CgiParams &params);
    bool SaveGetParams(const CgiParams &params);

    uint16_t _nextId; // Next ID to use for a new pattern
    uint16_t _newPatternId; // ID of the last pattern added

    std::list<SsiSubscription> _ssiHandlers;
    std::list<CgiSubscription> _cgiHandlers;

    const PatternConfig *_getPattern; // Current pattern being read by the API

    std::shared_ptr<WebServer> _webServer;
    std::shared_ptr<DeviceConfig> _deviceConfig;
    std::unique_ptr<PatternEditor> _currentEditor;
    std::shared_ptr<AnimationRunner> _animationRunner;
};
