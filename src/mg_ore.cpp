/*
Minetest
Copyright (C) 2010-2014 kwolekr, Ryan Kwolek <kwolekr@minetest.net>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "mg_ore.h"
#include "mapgen.h"
#include "util/numeric.h"
#include "map.h"
#include "log.h"


FlagDesc flagdesc_ore[] = {
	{"absheight",            OREFLAG_ABSHEIGHT},
	{"scatter_noisedensity", OREFLAG_DENSITY},
	{"claylike_nodeisnt",    OREFLAG_NODEISNT},
	{NULL,                   0}
};

///////////////////////////////////////////////////////////////////////////////


Ore *createOre(OreType type)
{
	switch (type) {
	case ORE_SCATTER:
		return new OreScatter;
	case ORE_SHEET:
		return new OreSheet;
	//case ORE_CLAYLIKE: //TODO: implement this!
	//	return new OreClaylike;
	default:
		return NULL;
	}
}


Ore::~Ore()
{
	delete np;
	delete noise;
}


void Ore::placeOre(Mapgen *mg, u32 blockseed, v3s16 nmin, v3s16 nmax)
{
	int in_range = 0;

	in_range |= (nmin.Y <= height_max && nmax.Y >= height_min);
	if (flags & OREFLAG_ABSHEIGHT)
		in_range |= (nmin.Y >= -height_max && nmax.Y <= -height_min) << 1;
	if (!in_range)
		return;

	int ymin, ymax;
	if (in_range & ORE_RANGE_MIRROR) {
		ymin = MYMAX(nmin.Y, -height_max);
		ymax = MYMIN(nmax.Y, -height_min);
	} else {
		ymin = MYMAX(nmin.Y, height_min);
		ymax = MYMIN(nmax.Y, height_max);
	}
	if (clust_size >= ymax - ymin + 1)
		return;

	nmin.Y = ymin;
	nmax.Y = ymax;
	generate(mg->vm, mg->seed, blockseed, nmin, nmax);
}


void OreScatter::generate(ManualMapVoxelManipulator *vm, int seed,
	u32 blockseed, v3s16 nmin, v3s16 nmax)
{
	PseudoRandom pr(blockseed);
	MapNode n_ore(c_ore, 0, ore_param2);

	int volume = (nmax.X - nmin.X + 1) *
				 (nmax.Y - nmin.Y + 1) *
				 (nmax.Z - nmin.Z + 1);
	int csize     = clust_size;
	int orechance = (csize * csize * csize) / clust_num_ores;
	int nclusters = volume / clust_scarcity;

	for (int i = 0; i != nclusters; i++) {
		int x0 = pr.range(nmin.X, nmax.X - csize + 1);
		int y0 = pr.range(nmin.Y, nmax.Y - csize + 1);
		int z0 = pr.range(nmin.Z, nmax.Z - csize + 1);

		if (np && (NoisePerlin3D(np, x0, y0, z0, seed) < nthresh))
			continue;

		for (int z1 = 0; z1 != csize; z1++)
		for (int y1 = 0; y1 != csize; y1++)
		for (int x1 = 0; x1 != csize; x1++) {
			if (pr.range(1, orechance) != 1)
				continue;

			u32 i = vm->m_area.index(x0 + x1, y0 + y1, z0 + z1);
			if (!CONTAINS(c_wherein, vm->m_data[i].getContent()))
				continue;

			vm->m_data[i] = n_ore;
		}
	}
}


void OreSheet::generate(ManualMapVoxelManipulator *vm, int seed,
	u32 blockseed, v3s16 nmin, v3s16 nmax)
{
	PseudoRandom pr(blockseed + 4234);
	MapNode n_ore(c_ore, 0, ore_param2);

	int max_height = clust_size;
	int y_start = pr.range(nmin.Y, nmax.Y - max_height);

	if (!noise) {
		int sx = nmax.X - nmin.X + 1;
		int sz = nmax.Z - nmin.Z + 1;
		noise = new Noise(np, 0, sx, sz);
	}
	noise->seed = seed + y_start;
	noise->perlinMap2D(nmin.X, nmin.Z);

	int index = 0;
	for (int z = nmin.Z; z <= nmax.Z; z++)
	for (int x = nmin.X; x <= nmax.X; x++) {
		float noiseval = noise->result[index++];
		if (noiseval < nthresh)
			continue;

		int height = max_height * (1. / pr.range(1, 3));
		int y0 = y_start + np->scale * noiseval; //pr.range(1, 3) - 1;
		int y1 = y0 + height;
		for (int y = y0; y != y1; y++) {
			u32 i = vm->m_area.index(x, y, z);
			if (!vm->m_area.contains(i))
				continue;
			if (!CONTAINS(c_wherein, vm->m_data[i].getContent()))
				continue;

			vm->m_data[i] = n_ore;
		}
	}
}

