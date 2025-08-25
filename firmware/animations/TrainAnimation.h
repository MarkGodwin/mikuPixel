#pragma once


#include "IAnimation.h"
#include "NeoPixelBuffer.h"
#include "Miku.h"
#include <queue>

class TrackOccupancyTracker
{
public:
    bool IsOccupied(int trackIndex) const
    {
        return _trackOccupied[trackIndex];
    }

    void Occupy(int trackIndex)
    {
        _trackOccupied[trackIndex] = true;
    }

    void Vacate(int trackIndex)
    {
        _trackOccupied[trackIndex] = false;
    }

private:
    std::array<bool,std::size(MikuTracks)> _trackOccupied = {};
};


class TrainPosition
{
public:
    TrainPosition(int startTrack, int maxLength, neopixel colour, TrackOccupancyTracker *trackOccupied);

    void Drive();
    void Draw(neopixel *buffer);

private:
    bool SelectNewTrack();

    TrackOccupancyTracker *_occupancyTracker;
    neopixel _colour; // Colour of the train part
    std::deque<Connection> _occupiedTracks; // Queue of tracks the train part is occupying
    int _headTrack; // Index of current track part where the the head of the train is
    int _headPosition; // Position of the head of the train in global pixel index
    int _headDirection; // Direction of the head of the train in the current track part
    int _tailTrack; // Index of current track part where the the tail of the train is
    int _tailPosition; // Position of the tail of the train in global pixel index
    int _tailDirection; // Direction of the tail of the train in the tail track part
    int _trainLength; // Number of pixels the train occupies on the track, used to grow train from start
    int _maxLength; // Maximum length of the train in pixels
    int _stuckCount; // Count how many times the train has been stuck
};

class TrainAnimation : public IAnimation
{
public:
    TrainAnimation();

    virtual uint32_t DrawFrame(NeoPixelFrame frame, uint32_t frameCounter) override;

private:
    TrackOccupancyTracker _occupancyTracker;

    TrainPosition _trains[3];
};

