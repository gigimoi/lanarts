/* ProjectileInst.h:
 *  Represents a projectile attack's trajectory
 */

#include <cmath>
#include <cstdio>
#include <typeinfo>

extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>
}

#include "../display/colour_constants.h"
#include "../display/sprite_data.h"
#include "../gamestate/GameState.h"

#include "../lua/lua_api.h"

#include "../stats/projectile_data.h"
#include "../stats/stat_formulas.h"

#include "../stats/weapon_data.h"

#include "../util/math_util.h"

#include "enemy/EnemyInst.h"

#include "player/PlayerInst.h"

#include "AnimatedInst.h"
#include "ItemInst.h"
#include "ProjectileInst.h"
#include "collision_filters.h"

ProjectileInst::~ProjectileInst() {
}

void ProjectileInst::draw(GameState* gs) {
	GameView& view = gs->view();
	SpriteEntry& spr = game_sprite_data[sprite()];
	int w = spr.width(), h = spr.height();
	int xx = x - w / 2, yy = y - h / 2;

	if (!view.within_view(xx, yy, w, h))
		return;
	if (!gs->object_visible_test(this))
		return;

	gl_draw_sprite(view, sprite(), xx, yy, vx, vy, 0);
}

void ProjectileInst::deinit(GameState* gs) {
	ProjectileEntry& pentry = projectile.projectile_entry();
	int break_roll = gs->rng().rand(100);
	if (pentry.drop_chance > break_roll) {
		int nx = round_to_multiple(x, TILE_SIZE, true), ny = round_to_multiple(
				y, TILE_SIZE, true);
		ItemInst* item = new ItemInst(projectile.as_item().id, nx, ny, 1,
				origin_id, true /*auto-pickup*/);
		gs->add_instance(item);
	}
}

void ProjectileInst::copy_to(GameInst* inst) const {
	LANARTS_ASSERT(typeid(*this) == typeid(*inst));
	*(ProjectileInst*) inst = *this;
}

ProjectileInst::ProjectileInst(const Projectile& projectile,
		const EffectiveAttackStats& atkstats, obj_id origin_id,
		const Pos& start, const Pos& target, float speed, int range,
		obj_id sole_target, bool bounce, int hits) :
		GameInst(start.x, start.y, projectile.projectile_entry().radius, false, DEPTH), rx(
				start.x), ry(start.y), speed(speed), team(
				0 /*TODO: properly init*/), origin_id(origin_id), sole_target(
				sole_target), projectile(projectile), atkstats(atkstats), range_left(
				range), bounce(bounce), hits(hits), damage_mult(1.0f) {
	direction_towards(start, target, vx, vy, speed);
}

ProjectileInst* ProjectileInst::clone() const {
	return new ProjectileInst(*this);
}

static void lua_hit_callback(lua_State* L, LuaValue& callback,
		const EffectiveAttackStats& atkstats, GameInst* projectile,
		GameInst* target) {
	callback.push(L);
	if (!lua_isnil(L, -1)) {
		lua_push_gameinst(L, projectile);
		lua_push_gameinst(L, target);
		lua_push_effectiveattackstats(L, atkstats);
		lua_call(L, 3, 0);
	} else {
		lua_pop(L, 1);
	}
}

void ProjectileInst::step(GameState* gs) {
	Pos tile_hit;
	int newx = (int) round(rx + vx); //update based on rounding of true float
	int newy = (int) round(ry + vy);
	bool collides = gs->tile_radius_test(newx, newy, radius, true, -1,
			&tile_hit);
	if (bounce) {
		bool hitsx = gs->tile_radius_test(newx, y, radius, true, -1);
		bool hitsy = gs->tile_radius_test(x, newy, radius, true, -1);
		if (hitsy || hitsx || collides) {
			if (hitsx) {
				vx = -vx;
			}
			if (hitsy) {
				vy = -vy;
			}
			if (!hitsy && !hitsx) {
				vx = -vx;
				vy = -vy;
			}
		}
	} else if (collides) {
		gs->remove_instance(this);
	}

	x = (int) round(rx += vx); //update based on rounding of true float
	y = (int) round(ry += vy);

	range_left -= speed;

	GameInst* colobj = NULL;
	CombatGameInst* origin = (CombatGameInst*) gs->get_instance(origin_id);

	if (dynamic_cast<PlayerInst*>(origin)) {
		if (sole_target)
			gs->object_radius_test(this, &colobj, 1, &bullet_target_hit2);
		else
			gs->object_radius_test(this, &colobj, 1, &enemy_colfilter);

		if (colobj) {
			EnemyInst* victim = (EnemyInst*) colobj;
			if (origin) {
				origin->signal_attacked_successfully();
			}

			lua_hit_callback(gs->get_luastate(),
					projectile.projectile_entry().on_hit_func, atkstats, this,
					victim);

			int damage = damage_formula(atkstats, victim->effective_stats());
			damage *= damage_mult;

			char buffstr[32];
			snprintf(buffstr, 32, "%d", damage);
			float rx = vx / speed * .5;
			float ry = vy / speed * .5;
			gs->add_instance(
					new AnimatedInst(
							Pos(victim->x - 5 + rx * 5, victim->y + ry * 5), -1,
							25, Posf(rx, ry), Posf(), AnimatedInst::DEPTH,
							buffstr));

			if (victim->damage(gs, damage)) {
				PlayerInst* p = (PlayerInst*) origin;
				PlayerData& pc = gs->player_data();
				p->signal_killed_enemy();

				int amnt = round(
						double(victim->xpworth()) / pc.all_players().size());
				players_gain_xp(gs, amnt);

				snprintf(buffstr, 32, "%d XP", amnt);
				gs->add_instance(
						new AnimatedInst(victim->pos(), -1, 25, Posf(), Posf(),
								AnimatedInst::DEPTH, buffstr, COL_GOLD));
			}
		}
	} else {
		gs->object_radius_test(this, &colobj, 1, &player_colfilter);
		if (colobj) {
			CombatGameInst* victim = (CombatGameInst*) colobj;
			if (origin) {
				origin->signal_attacked_successfully();
			}

			lua_hit_callback(gs->get_luastate(),
					projectile.projectile_entry().on_hit_func, atkstats, this,
					victim);

			int damage = damage_formula(atkstats, victim->effective_stats());
			damage *= damage_mult;

			if (!gs->game_settings().invincible)
				victim->damage(gs, damage);
			char dmgstr[32];
			snprintf(dmgstr, 32, "%d", damage);
			float rx = vx / speed * .5;
			float ry = vy / speed * .5;
			gs->add_instance(
					new AnimatedInst(
							Pos(colobj->x - 5 + rx * 5, colobj->y + ry * 5), -1,
							25, Posf(rx, ry), Posf(), AnimatedInst::DEPTH,
							dmgstr));
		}
	}
	if (colobj || range_left <= 0) {
		hits--;
		if (hits >= 0 && colobj) {
			MonsterController& mc = gs->monster_controller();
			int mindist = 200;
			if (sole_target == 0)
				damage_mult = 0.5;
			sole_target = NONE; //Clear target
			for (int i = 0; i < mc.monster_ids().size(); i++) {
				obj_id mid = mc.monster_ids()[i];
				GameInst* enemy = gs->get_instance(mid);
				if (enemy && enemy != colobj) {

					float abs = distance_between(Pos(x, y),
							Pos(enemy->x, enemy->y));
					if (abs < 1)
						abs = 1;
					if (abs < mindist) {
						sole_target = mid;
						mindist = abs;
						direction_towards(Pos(x, y), Pos(enemy->x, enemy->y),
								vx, vy, speed);
					}
				}
			}
		}
		if (hits == 0 || sole_target == 0) {
			gs->add_instance(
					new AnimatedInst(pos(), sprite(), 15, Posf(),
							Posf(vx, vy)));
			gs->remove_instance(this);
		}
	}

}

sprite_id ProjectileInst::sprite() const {
	ProjectileEntry& pentry = projectile.projectile_entry();
	return pentry.attack_sprite;
}

void ProjectileInst::serialize(GameState* gs, SerializeBuffer& serializer) {
	GameInst::serialize(gs, serializer);
	SERIALIZE_POD_REGION(serializer, this, rx, sole_target);
	projectile.serialize(serializer);
	SERIALIZE_POD_REGION(serializer, this, atkstats, damage_mult);
}

void ProjectileInst::deserialize(GameState* gs, SerializeBuffer& serializer) {
	GameInst::deserialize(gs, serializer);
	DESERIALIZE_POD_REGION(serializer, this, rx, sole_target);
	projectile.deserialize(serializer);
	DESERIALIZE_POD_REGION(serializer, this, atkstats, damage_mult);
}

bool ProjectileInst::bullet_target_hit2(GameInst* self, GameInst* other) {
	return ((ProjectileInst*) self)->sole_target == other->id;
}

