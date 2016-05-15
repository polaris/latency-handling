#ifndef _Clock_H
#define _Clock_H

#include <chrono>

class Clock {
public:
    Clock();

    void update();

    float getTime() const;

    float getElapsed() const;

    float getGameTime() const;

    float getFrameTime() const;

private:
    std::chrono::high_resolution_clock::time_point startTime;

    std::chrono::high_resolution_clock::time_point lastTime;

    std::chrono::high_resolution_clock::time_point currentTime;

    float elapsed;
};

#endif  // _Clock_H
