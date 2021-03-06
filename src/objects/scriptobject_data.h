/*
 * scriptobject_data.h:
 *  Describes an object whose behaviour is entirely determined by lua callbacks.
 */

#ifndef SCRIPTOBJECT_DATA_H_
#define SCRIPTOBJECT_DATA_H_
#include <cstdlib>
#include <vector>
#include <string>

#include "../lua/LuaValue.h"
#include "../lanarts_defines.h"

#include "GameInst.h"

struct ScriptObjectEntry {
	enum equip_type {
		NONE, WEAPON, PROJECTILE, HELMET, SHIELD, ARMOUR, BOOTS
	};
	std::string name;
	int sprite, radius;
	LuaValue init_event, step_event;

	void init(lua_State* L) {
		init_event.initialize(L);
		step_event.initialize(L);
	}
};

scriptobj_id get_scriptobject_by_name(const char* name,
		bool error_if_not_found = true);

extern std::vector<ScriptObjectEntry> game_scriptobject_data;

#endif /* ITEM_DATA_H_ */
