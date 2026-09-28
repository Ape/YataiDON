#pragma once

class FPSCounter {
private:
    static const int SAMPLE_SIZE = 60;
    float frameTimes[SAMPLE_SIZE] = {};
    int currentFrame = 0;
    double lastTime = 0.0;

public:
    FPSCounter();

    void update();

    float get_fps() const;

    void draw();
};
