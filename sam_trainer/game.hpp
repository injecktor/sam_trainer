#include "sam_trainer.hpp"

extern void __fastcall ReceiveHealth(PVOID entity, long health_change, long arg1);
extern void __fastcall ReceiveArmor(PVOID entity, long armor_change, long arg1);

extern void GetGroupMover();