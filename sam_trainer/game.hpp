#include "sam_trainer.hpp"

class Entity {
public:
    void __thiscall ReceiveHealth(long health_change, long arg1);
    void __thiscall ReceiveArmor(long armor_change, long arg1);
    void __thiscall GetPlayer(PVOID arg1);
};