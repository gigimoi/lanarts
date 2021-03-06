/*
 * armour_data.h:
 *  Data related to armour
 */

#ifndef ARMOUR_DATA_H_
#define ARMOUR_DATA_H_

#include "../lanarts_defines.h"

#include "item_data.h"

#include "stats.h"

struct ArmourEntry {
	std::string name, description;

	// Cost when appearing in shops (if an item), if (0,0) will not appear in shops.
	Range shop_cost;

	sprite_id item_sprite;
	ItemEntry::equip_type equiptype;
	CoreStatMultiplier resistance, damage_reduction;
	CoreStatMultiplier magic_resistance, magic_reduction;
	float spell_cooldown_multiplier;

	ArmourEntry() :
			item_sprite(0), equiptype(ItemEntry::NONE), spell_cooldown_multiplier(
					1.0f) {
	}
};

int get_armour_by_name(const char* name, bool error_if_not_found = true);

extern std::vector<ArmourEntry> game_armour_data;

inline ArmourEntry& armour_entry(armour_id id) {
	return game_armour_data.at(id);
}

#endif /* ARMOUR_DATA_H_ */
