/*
 * Equipment.cpp
 *  Represents all the possessions and equipped items of a player
 */

#include "Equipment.h"

#include "../data/item_data.h"
#include "../data/weapon_data.h"

bool Equipment::valid_to_use_projectile(projectile_id pid) {
	WeaponEntry& wentry = game_weapon_data[weapon];
	if (pid == -1)
		return true;
	ProjectileEntry& pentry = game_projectile_data[pid];
	if (pentry.weapon_class == "unarmed")
		return true;
	if (pentry.weapon_class == wentry.weapon_class)
		return true;
	return false;
}

void Equipment::deequip_projectiles() {
	if (projectile != -1) {
		ProjectileEntry& pentry = game_projectile_data[projectile];
		item_id item = get_item_by_name(pentry.name.c_str());
		inventory.add(item, projectile_amnt);
		projectile = -1;
		projectile_amnt = 0;
	}
}

void Equipment::deequip_weapon() {
	if (weapon != 0) {
		WeaponEntry& wentry = game_weapon_data[weapon];
		item_id item = get_item_by_name(wentry.name.c_str());
		inventory.add(item, 1);
		weapon = 0;

		if (!valid_to_use_projectile(projectile)) {
			deequip_projectiles();
		}
	}
}

void Equipment::deequip(int equipment_type) {
	switch (equipment_type) {
	case ItemEntry::PROJECTILE:
		deequip_projectiles();
		break;
	case ItemEntry::WEAPON:
		deequip_weapon();
		break;
	}
}
bool Equipment::valid_to_use(item_id item) {
	ItemEntry& entry = game_item_data[item];
	int eqid = entry.equipment_id;

	switch (entry.equipment_type) {
	case ItemEntry::PROJECTILE:
		return valid_to_use_projectile(eqid);
	}
	return true;
}
void Equipment::equip(item_id item, int amnt) {
	ItemEntry& entry = game_item_data[item];
	WeaponEntry& wentry = game_weapon_data[weapon];
	int eqid = entry.equipment_id;

	switch (entry.equipment_type) {
	case ItemEntry::WEAPON:
		deequip_projectiles();
		if (weapon != 0) {
			item_id item = get_item_by_name(wentry.name.c_str());
			inventory.add(item, 1);
		}
		weapon = eqid;
		break;
	case ItemEntry::PROJECTILE:
		if (eqid != projectile)
			deequip_projectiles();
		projectile = eqid;
		projectile_amnt += amnt;
		break;
	}
}

void Equipment::use_ammo(int amnt) {
	projectile_amnt -= amnt;
	if (projectile_amnt <= 0) {
		projectile = -1;
		projectile_amnt = 0;
	}
}

