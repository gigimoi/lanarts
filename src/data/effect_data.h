
#ifndef EFFECT_DATA_H_
#define EFFECT_DATA_H_

#include <cstdlib>
#include <string>
#include <vector>

#include "../util/LuaValue.h"
#include "../gamestats/stats.h"

struct EffectEntry {
	std::string name;
	LuaValue statmod;
	bool additive_duration;
	EffectEntry(const std::string& name, const std::string& statmod_luaf, bool additive_duration) :
			name(name), statmod(statmod_luaf), additive_duration(additive_duration) {
	}
	void init(lua_State* L) {
		statmod.initialize(L);
	}
};

int get_effect_by_name(const char* name);
extern std::vector<EffectEntry> game_effect_data;

#endif /* EFFECT_DATA_H_ */
