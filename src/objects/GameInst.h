/*
 *GameInst.h:
 * Base object of the game object inheritance heirarchy
 * Life cycle of GameInst initialization:
 * ->Constructor
 * ->GameState::add_instance(obj) called
 * 		->GameInstSet::add_instance(obj) called from GameState::add_instance, sets id
 * 		->GameInst::init(GameState) called from GameState::add_instance, does further initialization
 * ->
 */

#ifndef GAMEINST_H_
#define GAMEINST_H_

#include <cassert>
#include "../lua/LuaValue.h"
#include "../lanarts_defines.h"

struct lua_State;
class GameState;
class SerializeBuffer;
//Base class for game instances

class GameInst {
public:
	GameInst(int x, int y, int radius, bool solid = true, int depth = 0) :
			reference_count(0), id(0), last_x(x), last_y(y), x(x), y(y), radius(
					radius), target_radius(radius), depth(depth), solid(solid), destroyed(
					false), current_level(-1) {
		if (this->radius > 14)
			this->radius = 14;
	}
	virtual ~GameInst();
	/* Initialize the object further, 'id' will be set*/
	virtual void init(GameState* gs);
	/* Deinitialize the object, removing eg child instances*/
	virtual void deinit(GameState* gs);
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);

	virtual void copy_to(GameInst* inst) const;

	virtual void serialize(GameState* gs, SerializeBuffer& serializer);
	virtual void deserialize(GameState* gs, SerializeBuffer& serializer);
	virtual GameInst* clone() const;
	//Used for integrity checking
	virtual unsigned int integrity_hash();
	virtual void update_position(float newx, float newy);

	BBox bbox() {
		return BBox(x - radius, y - radius, x + radius, y + radius);
	}

	Pos pos() {
		return Pos(x, y);
	}
	//Used for keeping object from being deleted arbitrarily
	//Important for lua code
	int reference_count;
	/* Reference counting functions
	 * NOTE: free_reference employs 'delete this;' */
	void retain_reference();
	void free_reference();

	/*Should probably keep these public, many functions operate on these*/
	obj_id id;
	int last_x, last_y;
	int x, y, radius, target_radius;
	int depth;
	bool solid, destroyed;
	level_id current_level;
	LuaValue lua_variables;
};

typedef bool (*col_filterf)(GameInst* o1, GameInst* o2);

#endif /* GAMEINST_H_ */
