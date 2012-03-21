#ifndef ENEMYINST_H_
#define ENEMYINST_H_

#include "GameInst.h"
#include "../../gamestats/Stats.h"
#include "../../data/enemy_data.h"

struct EnemyBehaviour {
	enum Action {
		WANDERING,
		CHASING_PLAYER
	};
	Action current_action;
	float speed, vx, vy;
	EnemyBehaviour(float speed) :
		current_action(WANDERING), speed(speed), vx(0), vy(0){
	}
};

class EnemyInst : public GameInst {
public:
	EnemyInst(EnemyType* type, int x, int y) :
		GameInst(x,y, type->radius), eb(type->basestats.movespeed), type(type), rx(x), ry(y), stat(type->basestats) {
	}
	virtual ~EnemyInst();
	virtual void init(GameState* gs);
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	void attack(GameState* gs, GameInst* inst, bool ranged);
	Stats& stats() { return stat; }
	EnemyBehaviour& behaviour() { return eb; }
protected:
	EnemyBehaviour eb;
	EnemyType* type;
	float rx, ry;
    Stats stat;
};


#endif /* ENEMYINST_H_ */
