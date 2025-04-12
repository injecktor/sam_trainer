#include "sam_trainer.hpp"
#include "offsets.hpp"

extern bool disable_receive_health;
extern bool disable_receive_armor;
extern int hp;
extern int armor;
extern int max_hp;
extern int max_armor;

extern void set_entity_property_by_offset(DWORD offset, PVOID buffer);

extern void GetGroupMover();