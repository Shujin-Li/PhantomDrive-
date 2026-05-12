//将ai行为参数化
#ifndef AIPROFILE_H
#define AIPROFILE_H

namespace PhantomDrive {

struct AIProfile
{
    float maxThrottle = 0.8f;

    float steeringStrength = 0.5f;

    float brakeSensitivity = 0.5f;
};

}

#endif // AIPROFILE_H