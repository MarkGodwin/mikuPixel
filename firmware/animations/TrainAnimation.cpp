#include "TrainAnimation.h"
#include "debug.h"
#include <algorithm>

TrainPosition::TrainPosition(int startTrack, int maxLength, neopixel colour, TrackOccupancyTracker *occupancyTracker)
    : _occupancyTracker(occupancyTracker),
    _colour(colour),
    _headTrack(startTrack),
    _headPosition(MikuTracks[startTrack].start),
    _headDirection(1),
    _tailTrack(startTrack),
    _tailPosition(MikuTracks[startTrack].start),
    _tailDirection(0),
    _trainLength(0),
    _maxLength(maxLength),
    _stuckCount(0)
{
    _occupancyTracker->Occupy(startTrack);
}

void TrainPosition::Drive()
{
    auto &currentTrack = MikuTracks[_headTrack];
    if(currentTrack.IsAtEnd(_headPosition, _headDirection))
    {
        if(!SelectNewTrack())
        {
            _stuckCount++;
            if(_stuckCount > 60)
            {
                // Reverse the train
                DBG_PRINT("Reversing train at track %d pos %d dir %d\n", _headTrack, _headPosition, _headDirection);
                std::swap(_headTrack, _tailTrack);
                std::swap(_headPosition, _tailPosition);
                std::swap(_headDirection, _tailDirection);
                std::reverse(_occupiedTracks.begin(), _occupiedTracks.end());
                // By convention, the tail is not on the occupied tracks list, so remove it and add the new head
                if(!_occupiedTracks.empty())
                {
                    _occupiedTracks.pop_front();
                    _occupiedTracks.push_back(Connection{_headTrack, _headDirection > 0});
                }
                _headDirection = -_headDirection;
                _tailDirection = -_tailDirection;
                for(auto &track : _occupiedTracks)
                {
                    track.start = !track.start;
                }
                _stuckCount = 0;
            }
            return; // Stuck, can't move
        }
    }
    else
    {
        // Move forward on the current track part
       _headPosition += _headDirection;
       _stuckCount = 0;
    }

    if(_tailDirection == 0)
    {
        // Train is still growing
        _trainLength++;
        if(_trainLength == _maxLength)
            _tailDirection = 1;
    }
    else
    {
        auto &tailTrack = MikuTracks[_tailTrack];
        if(tailTrack.IsAtEnd(_tailPosition, _tailDirection))
        {
            //DBG_PRINT("Vacating track %d pos %d dir %d\n", _tailTrack, _tailPosition, _tailDirection);
            // Finished with this track part, vacate it
            _occupancyTracker->Vacate(_tailTrack);
            auto &next = _occupiedTracks.front();
            _tailTrack = next.track;
            _tailPosition = next.start ? MikuTracks[_tailTrack].start : MikuTracks[_tailTrack].end;
            _tailDirection = next.start ? 1 : -1;

            _occupiedTracks.pop_front();
            //DBG_PRINT("Tail now on track %d pos %d dir %d (%d occupied)\n", _tailTrack, _tailPosition, _tailDirection, _occupiedTracks.size());
        }
        else
            _tailPosition += _tailDirection;
    }
}

bool TrainPosition::SelectNewTrack()
{

    // At the end of the current track part, need to switch to a connected track
    std::vector<Connection> validConnections;
    auto possibleConnections = MikuTracks[_headTrack].GetConnections(_headDirection > 0);
    while(possibleConnections->track >= 0)
    {
        if(!_occupancyTracker->IsOccupied(possibleConnections->track))
            validConnections.push_back(*possibleConnections);
        possibleConnections++;
    }

    //DBG_PRINT("At end of track %d pos %d dir %d, %d valid connections\n", _headTrack, _headPosition, _headDirection, validConnections.size());

    if(validConnections.size() == 0)
    {
        // No valid connections, stop the train for now
        //DBG_PRINT("Stuck train at track %d pos %d dir %d\n", _headTrack, _headPosition, _headDirection);
        return false;
    } 

    // Pick a random valid connection
    auto connectionIndex = rand() % validConnections.size();

    // Move the head to the connected track part
    _occupiedTracks.push_back(validConnections[connectionIndex]);
    _headTrack = validConnections[connectionIndex].track;
    _headPosition = validConnections[connectionIndex].start ? MikuTracks[_headTrack].start : MikuTracks[_headTrack].end;
    _headDirection = validConnections[connectionIndex].start ? 1 : -1;

    // Occupy the new track part
    //DBG_PRINT("Occupied track %d pos %d dir %d (%d occupied)\n", _headTrack, _headPosition, _headDirection, _occupiedTracks.size());
    _occupancyTracker->Occupy(_headTrack);
    return true;
}

void TrainPosition::Draw(neopixel *buffer)
{
    // Draw the train on the buffer from tail to head
    int pos = _tailPosition;
    int dir = _tailDirection;
    if(dir == 0)
        dir = 1; // Still growing, just draw forward
    int track = _tailTrack;
    int remainingLength = _trainLength;

    auto nextTrack = _occupiedTracks.begin();
    auto fade = 64; // Rear of train is dimmer

    while(remainingLength > 0)
    {
        auto col = _colour.fade(fade);
        if(remainingLength < 2)
            col = col.blend(neopixel(255,255,255), 128);
        buffer[pos] = col.gammaCorrected();
        fade += 16;
        if(fade > 255)
            fade = 255;

        if(MikuTracks[track].IsAtEnd(pos, dir))
        {
            if(nextTrack == _occupiedTracks.end())
                break; // Shouldn't happen, but just in case

            track = nextTrack->track;
            pos = nextTrack->start ? MikuTracks[track].start : MikuTracks[track].end;
            dir = nextTrack->start ? 1 : -1;
            nextTrack++;
        }
        else
        {
            pos += dir;
        }

        remainingLength--;
    }
}


TrainAnimation::TrainAnimation()
: _trains{
        TrainPosition(0, 18, neopixel(128, 0, 128), &_occupancyTracker),
        TrainPosition(5, 12, neopixel(0, 128, 128), &_occupancyTracker),
        TrainPosition(10, 8, neopixel(128, 128, 0), &_occupancyTracker)
    }
{
}

uint32_t TrainAnimation::DrawFrame(NeoPixelFrame frame, uint32_t frameCounter)
{
    // Clear the frame
    frame.Clear();

    for(auto &train : _trains)
    {
        train.Drive();
    }

    for(auto &train : _trains)
    {
        train.Draw(frame.GetBuffer());
    }

    return 1000 / 30;
}

