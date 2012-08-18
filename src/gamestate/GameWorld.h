/* GameWorld:
 *  Contains player object information
 *  Contains enemy's seen information
 * 	Contains all the information and functionality of previously generated levels,
 * 	and which levels are currently being generated.
 */

#ifndef GAMEWORLD_H_
#define GAMEWORLD_H_

#include <vector>
#include <cstdlib>

#include "../lanarts_defines.h"
#include "../stats/EnemiesSeen.h"
#include "../objects/TeamRelations.h"

class GameState;
class GameLevelState;
class GeneratedLevel;
class PlayerInst;

class GameWorld {
public:
	GameWorld(GameState* gs);
	~GameWorld();
	void generate_room(GameLevelState* level);
	GameLevelState* get_level(int roomid, bool spawnplayer = false,
			void** player_instances = NULL, size_t nplayers = 0);
	bool pre_step();
	void step();
	void level_move(int id, int x, int y, int roomid1, int roomid2);
	void set_current_level(int roomid);
	void set_current_level_lazy(int roomid);
	void reset(int keep = 0);
	void regen_level(int roomid);
	void place_inst(GeneratedLevel& genlevel, GameInst* inst);
	GameLevelState* get_current_level() {
		return lvl;
	}
	void set_current_level(GameLevelState* level) {
		lvl = level;
	}
	PlayerData& player_data() {
		return _player_data;
	}
	EnemiesSeen& enemies_seen() {
		return _enemies_seen;
	}
	TeamRelations& team_relations() {
		return _team_relations;
	}
private:
	void spawn_players(GeneratedLevel& genlevel, void** player_instances,
			size_t nplayers);
	bool midstep;
	int next_room_id;

	EnemiesSeen _enemies_seen;
<<<<<<< HEAD
	PlayerController _player_controller;
	TeamRelations _team_relations;
=======
	PlayerData _player_data;
>>>>>>> dev

	GameLevelState* lvl;
	GameState* gs;
	std::vector<GameLevelState*> level_states;
};

#endif /* GAMEWORLD_H_ */
