//第一个ai-FSM：Finite State Machine有限状态机
#ifndef AIOPPONENTFSM_H
#define AIOPPONENTFSM_H

#include <PhantomDrive/ai/AIOpponent.h>
#include <PhantomDrive/ai/AIProfile.h>

namespace PhantomDrive {

class AIOpponentFSM : public AIOpponent
{
   // Q_OBJECT

public:
    explicit AIOpponentFSM(QObject* parent = nullptr);

    void update(float deltaTime) override;

    AIDecision makeDecision() override;

private:
    AIDecision m_currentDecision;

protected:
    int currentWaypointIndex = 0;

    float fakePositionX = 0.0f;

    AIProfile m_profile;

};
}
#endif // AIOPPONENTFSM_H