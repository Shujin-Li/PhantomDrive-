//第一个ai-FSM：Finite State Machine有限状态机
#ifndef AIOPPONENTFSM_H
#define AIOPPONENTFSM_H

#include <PhantomDrive/ai/AIOpponent.h>

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
};

}

#endif // AIOPPONENTFSM_H