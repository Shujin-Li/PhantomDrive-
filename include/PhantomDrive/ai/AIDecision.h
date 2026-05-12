#ifndef AIDECISION_H
#define AIDECISION_H

namespace PhantomDrive {

struct AIDecision
{
    float throttle = 0.0f;   //油门
    float brake = 0.0f;     //刹车
    float steering = 0.0f;  //转向
};

}

#endif // AIDECISION_H