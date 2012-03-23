/*
 * pathfind.cpp
 *
 *  Created on: Feb 6, 2012
 *      Author: 100397561
 */

#include <algorithm>
#include <cmath>
#include "pathfind.h"
#include "../data/tile_data.h"
#include "../world/GameState.h"
#include "../util/draw_util.h"

using namespace std;

void floodfill(PathingNode* path, int w, int h, int sx, int sy) {
	PathCoord* heap = new PathCoord[w * h];
	PathCoord* heap_end = heap + 1;

	heap[0] = PathCoord(sx, sy, 0); //Start coordinate
	path[sy * w + sx] = PathingNode(false, false, 0, 0, 0);
	while (heap != heap_end) {
		PathCoord curr = *heap;
		PathCoord next;
		pop_heap(heap, heap_end--);
		for (int dy = -1; dy <= +1; dy++) {
			for (int dx = -1; dx <= +1; dx++) {
				int nx = curr.x + dx, ny = curr.y + dy;
				if (nx < 0 || nx >= w || ny < 0 || ny >= h)
					continue;
				int coord = ny * w + nx;
				bool is_diag = (abs(dx) == abs(dy));
				int dist = curr.distance + (is_diag ? 140 : 100);
				PathingNode* p = &path[coord];
				if (p->open && !p->solid) {
					bool cant_cross = is_diag
							&& (path[curr.y * w + nx].solid
									|| path[ny * w + curr.x].solid);
					if (!cant_cross) {
						p->open = false;
						p->dx = -dx, p->dy = -dy;
						p->distance = dist;
						*(heap_end++) = PathCoord(nx, ny, dist);
						push_heap(heap, heap_end);
					}
				}
			}

		}
	}
//	make_heap(heap, );

	delete[] heap;
}

PathInfo::PathInfo() {
	path = NULL;
	w = 0, h = 0;
	start_x = 0, start_y = 0;
}
PathInfo::~PathInfo() {
	delete[] path;
}

void PathInfo::calculate_path(GameState* gs, int ox, int oy, int radius) {
	int min_tilex, min_tiley;
	int max_tilex, max_tiley;

	GameTiles& tile = gs->tile_grid();

	//Use a temporary 'GameView' object to make use of its helper methods
	GameView view(0, 0, radius * 2, radius * 2, gs->width(), gs->height());
	view.sharp_center_on(ox, oy);

	view.min_tile_within(min_tilex, min_tiley);
	view.max_tile_within(max_tilex, max_tiley);

	start_x = min_tilex, start_y = min_tiley;

	int ww = max_tilex - min_tilex, hh = max_tiley - min_tiley;
	if (!path || w != ww || h != hh) {
		w = ww, h = hh;
		path = new PathingNode[w * h];
	}
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			PathingNode* node = &path[y * w + x];
			node->solid = tile.get(x + min_tilex, y + min_tiley)
					<= TILE_STONE_WALL;
			node->open = true;
			node->dx = 0;
			node->dy = 0;
			node->marked = false;
			node->distance = 0;
		}
	}
	int tx = ox / TILE_SIZE - min_tilex, ty = oy / TILE_SIZE - min_tiley;
	floodfill(path, w, h, tx, ty);
	path_x = tx, path_y = ty;
}

static int squish(int a, int b, int c) {
	return std::min(std::max(a, b), c - 1);
}

/*Used to make sure all interpolated directions are possible*/
bool PathInfo::can_head(int sx, int sy, int ex, int ey, int speed, int dx,
		int dy) {
	bool is_diag = (abs(dx) == abs(dy));
	int xx, yy;
	for (int y = sy; y <= ey + TILE_SIZE; y += TILE_SIZE) {
		for (int x = sx; x <= ex + TILE_SIZE; x += TILE_SIZE) {
			xx = squish(x, sx, ex + 1);
			yy = squish(y, sy, ey + 1);
			int gx = (xx + dx * speed) / TILE_SIZE - start_x;
			int gy = (yy + dy * speed) / TILE_SIZE - start_y;
			if (get(gx,gy)->solid)
				return false;
			if (is_diag) {
				if (get(xx / TILE_SIZE - start_x,
						(yy + dy * speed) / TILE_SIZE - start_y)->solid)
					return false;
				if (get((xx + dx * speed) / TILE_SIZE - start_x,
						yy / TILE_SIZE - start_y)->solid)
					return false;
			}
		}
	}
	return true;
}

void PathInfo::interpolated_direction(int x, int y, int w, int h, float speed,
		float& vx, float& vy) {
	if (!path) {
		vx = 0, vy = 0;
		return;
	}
	int ispeed = (int) ceil(speed);
	int area = w * h;
	int mx = x + w, my = y + h;
	//Set up coordinate min and max
	int mingrid_x = x / TILE_SIZE, mingrid_y = y / TILE_SIZE;
	int maxgrid_x = mx / TILE_SIZE, maxgrid_y = my / TILE_SIZE;
	//Make sure coordinates do not go out of bounds
	int minx = squish(mingrid_x, start_x, start_x + width());
	int miny = squish(mingrid_y, start_y, start_y + height());
	int maxx = squish(maxgrid_x, start_x, start_x + width());
	int maxy = squish(maxgrid_y, start_y, start_y + height());
	//Set up accumulators for x and y (later normalized)
	int acc_x = 0, acc_y = 0;

	for (int yy = miny; yy <= maxy; yy++) {
		for (int xx = minx; xx <= maxx; xx++) {
			int sx = max(xx * TILE_SIZE, x), sy = max(yy * TILE_SIZE, y);
			int ex = min((xx + 1) * TILE_SIZE, mx), ey = min(
					(yy + 1) * TILE_SIZE, my);
			int px = xx - start_x, py = yy - start_y;
			PathingNode* p = get(px, py);
			if (!p->solid) {
				int sub_area = (ex - sx) * (ey - sy) + 1;
				/*Make sure all interpolated directions are possible*/
				if (can_head(x, y, mx, my, ispeed, p->dx, p->dy)) {
					acc_x += p->dx * sub_area;
					acc_y += p->dy * sub_area;
				}
			}
		}
	}
	float mag = sqrt(acc_x * acc_x + acc_y * acc_y);
	if (mag == 0) {
		vx = 0, vy = 0;
	} else {
		vx = speed * float(acc_x) / mag;
		vy = speed * float(acc_y) / mag;
	}
}

const int HUGE_DISTANCE = 1000000;

void PathInfo::point_to_local_min(int sx, int sy) {
	PathingNode* fixed_node = get(sx, sy);

	int minx = squish(sx - 1, 0, width()), miny = squish(sy - 1, 0, height());
	int maxx = squish(sx + 1, 0, width()), maxy = squish(sy + 1, 0, height());

	int dx = 0, dy = 0;
	int min_distance = HUGE_DISTANCE;

	if (!fixed_node->marked) {
		for (int yy = miny; yy <= maxy; yy++) {
			for (int xx = minx; xx <= maxx; xx++) {
				if (sx == xx && sy == yy)
					continue;
				PathingNode* p = get(xx, yy);
				if (p->solid)
					continue;
				int dist = p->distance
						+ (abs(xx - sx) == abs(yy - sy) ? 140 : 100);
				if (dist < min_distance) {
					dx = xx - sx, dy = yy - sy;
					min_distance = dist;
				}
			}
		}
	}
	fixed_node->dx = dx;
	fixed_node->dy = dy;
//	if (dx == 0 && dy == 0) {
//		fixed_node->distance = HUGE_DISTANCE;
//	} else {
//		fixed_node->distance = min_distance + (abs(dx) == abs(dy) ? 140 : 100);
//	}
}

void PathInfo::fix_distances(int sx, int sy) {
	PathingNode* fixed_node = get(sx, sy);

	int minx = squish(sx - 1, 0, width()), miny = squish(sy - 1, 0, height());
	int maxx = squish(sx + 1, 0, width()), maxy = squish(sy + 1, 0, height());

	int min_distance = fixed_node->distance;
	if (fixed_node->marked)
		return;

	for (int yy = miny; yy <= maxy; yy++) {
		for (int xx = minx; xx <= maxx; xx++) {
			if (sx == xx && sy == yy)
				continue;
			PathingNode* p = get(xx, yy);
			if (p->solid)
				continue;
			int dist = p->distance + (abs(xx - sx) == abs(yy - sy) ? 140 : 100);
			if (dist < min_distance) {
				min_distance = dist;
			}
		}
	}
	fixed_node->distance = min_distance;
}
void PathInfo::adjust_for_claims(int x, int y) {
	x = x / TILE_SIZE - start_x, y = y / TILE_SIZE - start_y;
	if (x < 0 || x >= width() || y < 0 || y >= height())
		return;
	int minx = squish(x - 1, 0, width()), miny = squish(y - 1, 0, height());
	int maxx = squish(x + 1, 0, width()), maxy = squish(y + 1, 0, height());
	for (int yy = miny; yy <= maxy; yy++) {
		for (int xx = minx; xx <= maxx; xx++) {
			this->point_to_local_min(xx, yy);
		}
	}
}
void PathInfo::stake_claim(int x, int y) {
	x = x / TILE_SIZE - start_x, y = y / TILE_SIZE - start_y;
	if (x < 0 || x >= width() || y < 0 || y >= height())
		return;

	PathingNode* start_node = get(x, y);
	x += start_node->dx, y += start_node->dy;
	//Don't claim the player's square
	if (x == path_x && y == path_y) {
		//Backtrack, and stake our current square
		x -= start_node->dx, y -= start_node->dy;
	}
	PathingNode* stake_node = get(x, y);

	//Make distance some arbitrarily large number
	stake_node->distance = HUGE_DISTANCE;
	stake_node->marked = true;
//	stake_node->solid = true;

	int minx = squish(x - 1, 0, width()), miny = squish(y - 1, 0, height());
	int maxx = squish(x + 1, 0, width()), maxy = squish(y + 1, 0, height());
	for (int yy = miny; yy <= maxy; yy++) {
		for (int xx = minx; xx <= maxx; xx++) {
			this->fix_distances(xx, yy);
		}
	}
	for (int yy = miny; yy <= maxy; yy++) {
		for (int xx = minx; xx <= maxx; xx++) {
			this->point_to_local_min(xx, yy);
		}
	}

//	stake_node->solid = false;
}

void PathInfo::draw(GameState* gs) {
	GameView& view = gs->window_view();
	int min_tilex, min_tiley;
	int max_tilex, max_tiley;

	view.min_tile_within(min_tilex, min_tiley);
	view.max_tile_within(max_tilex, max_tiley);

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			PathingNode* node = &path[y * w + x];
			if (false && !node->solid)
				gl_printf(gs->primary_font(), Colour(255, 255, 255),
						(x + start_x) * TILE_SIZE - view.x,
						(y + start_y) * TILE_SIZE - view.y, "%d,%d", node->dx,
						node->dy);
			if (!node->solid)
				gl_printf(gs->primary_font(), Colour(255, 255, 255),
						(x + start_x) * TILE_SIZE - view.x,
						(y + start_y) * TILE_SIZE - view.y, "%d",
						node->distance);
		}
	}
}