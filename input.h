#pragma once

struct InputState {
    bool left;
    bool right;
    bool jump;
};

class Input {
public:
    virtual InputState poll() = 0;
    virtual ~Input() = default;
};
