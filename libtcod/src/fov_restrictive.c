/*
* libtcod 1.5.2
* Copyright (c) 2008,2009,2010,2012 Jice & Mingos
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * The name of Jice or Mingos may not be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY JICE AND MINGOS ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL JICE OR MINGOS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
* Mingos' Restrictive Precise Angle Shadowcasting (MRPAS), v1.1
* This file was written by Dominik "Mingos" Marczuk.
* Original implementation: http://umbrarumregnum.110mb.com/downloads/MRPAS.zip
*/

#include <stdlib.h> /* for NULL in VS */
#include "libtcod.h"
#include "libtcod_int.h"

/* angle ranges */
double * start_angle = NULL;
double * end_angle = NULL;
/* number of allocated angle pairs */
int allocated = 0;

void TCOD_map_compute_fov_restrictive_shadowcasting_quadrant (map_t *m, int player_x, int player_y, int max_radius, bool light_walls, int dx, int dy) {
	/*octant: vertical edge */
	{
		int iteration = 1; /*iteration of the algo for this octant */
		bool done = false;
		int total_obstacles = 0;
		int obstacles_in_last_line = 0;
		double min_angle = 0.0;
		int x,y;

		/*do while there are unblocked slopes left and the algo is within the map's boundaries
		  scan progressive lines/columns from the PC outwards */
		y = player_y+dy; /*the outer slope's coordinates (first processed line) */
		if (y < 0 || y >= m->height) done = true;
		while(!done) {
			/*process cells in the line */
			double slopes_per_cell = 1.0 / (double)(iteration);
			double half_slopes = slopes_per_cell * 0.5;
			int processed_cell = (int)((min_angle + half_slopes) / slopes_per_cell);
			int minx = MAX(0,player_x-iteration), maxx = MIN(m->width-1,player_x+iteration);
			done = true;
			for (x = player_x + (processed_cell * dx); x >= minx && x <= maxx; x+=dx) {
				int c = x + (y * m->width);
				/*calculate slopes per cell */
				bool visible = true;
				bool extended = false;
				double centre_slope = (double)processed_cell * slopes_per_cell;
				double start_slope = centre_slope - half_slopes;
				double end_slope = centre_slope + half_slopes;
				if (obstacles_in_last_line > 0 && m->cells[c].fov == false) {
					int idx = 0;
					if (visible && (m->cells[c-(m->width*dy)].fov == false || !m->cells[c-(m->width*dy)].transparent) && (x-dx >= 0 && x-dx < m->width && (m->cells[c-(m->width*dy)-dx].fov == false || !m->cells[c-(m->width*dy)-dx].transparent))) visible = false;
					else while(visible && idx < obstacles_in_last_line) {
						if (start_angle[idx] > end_slope || end_angle[idx] < start_slope) {
							++idx;
						}
            else {
              if (m->cells[c].transparent) {
  							if (centre_slope > start_angle[idx] && centre_slope < end_angle[idx])
  								visible = false;
  						}
  						else {
  							if (start_slope >= start_angle[idx] && end_slope <= end_angle[idx])
  								visible = false;
  							else {
									start_angle[idx] = MIN(start_angle[idx],start_slope);
									end_angle[idx] = MAX(end_angle[idx],end_slope);
									extended = true;
								}
  						}
  						++idx;
						}
					}
				}
				if (visible) {
					m->cells[c].fov = true;
					done = false;
					/*if the cell is opaque, block the adjacent slopes */
					if (!m->cells[c].transparent) {
						if (min_angle >= start_slope) {
							min_angle = end_slope;
							/* if min_angle is applied to the last cell in line, nothing more
							   needs to be checked. */
							if (processed_cell == iteration) done = true;
						}
						else if (!extended) {
							start_angle[total_obstacles] = start_slope;
							end_angle[total_obstacles++] = end_slope;
						}
						if (!light_walls) m->cells[c].fov = false;
					}
				}
				processed_cell++;
			}
			if (iteration == max_radius) done = true;
			iteration++;
			obstacles_in_last_line = total_obstacles;
			y += dy;
			if (y < 0 || y >= m->height) done = true;
		}
	}
	/*octant: horizontal edge */
	{
		int iteration = 1; /*iteration of the algo for this octant */
		bool done = false;
		int total_obstacles = 0;
		int obstacles_in_last_line = 0;
		double min_angle = 0.0;
		int x,y;

		/*do while there are unblocked slopes left and the algo is within the map's boundaries
		 scan progressive lines/columns from the PC outwards */
		x = player_x+dx; /*the outer slope's coordinates (first processed line) */
		if (x < 0 || x >= m->width) done = true;
		while(!done) {
			/*process cells in the line */
			double slopes_per_cell = 1.0 / (double)(iteration);
			double half_slopes = slopes_per_cell * 0.5;
			int processed_cell = (int)((min_angle + half_slopes) / slopes_per_cell);
			int miny = MAX(0,player_y-iteration), maxy = MIN(m->height-1,player_y+iteration);
			done = true;
			for (y = player_y + (processed_cell * dy); y >= miny && y <= maxy; y+=dy) {
				int c = x + (y * m->width);
				/*calculate slopes per cell */
				bool visible = true;
				bool extended = false;
				double centre_slope = (double)processed_cell * slopes_per_cell;
				double start_slope = centre_slope - half_slopes;
				double end_slope = centre_slope + half_slopes;
				if (obstacles_in_last_line > 0 && m->cells[c].fov == false) {
					int idx = 0;
					if (visible && (m->cells[c-dx].fov == false || !m->cells[c-dx].transparent) && (y-dy >= 0 && y-dy < m->height && (m->cells[c-(m->width*dy)-dx].fov == false || !m->cells[c-(m->width*dy)-dx].transparent))) visible = false;
					else while(visible && idx < obstacles_in_last_line) {
						if (start_angle[idx] > end_slope || end_angle[idx] < start_slope) {
							++idx;
						}
						else {
              if (m->cells[c].transparent) {
  							if (centre_slope > start_angle[idx] && centre_slope < end_angle[idx])
  								visible = false;
  						}
  						else {
  							if (start_slope >= start_angle[idx] && end_slope <= end_angle[idx])
  								visible = false;
  							else {
									start_angle[idx] = MIN(start_angle[idx],start_slope);
									end_angle[idx] = MAX(end_angle[idx],end_slope);
									extended = true;
								}
  						}
  						++idx;
						}
					}
				}
				if (visible) {
					m->cells[c].fov = true;
					done = false;
					/*if the cell is opaque, block the adjacent slopes */
					if (!m->cells[c].transparent) {
						if (min_angle >= start_slope) {
						  min_angle = end_slope;
						  /* if min_angle is applied to the last cell in line, nothing more
							   needs to be checked. */
							if (processed_cell == iteration) done = true;
						}
						else if (!extended) {
							start_angle[total_obstacles] = start_slope;
							end_angle[total_obstacles++] = end_slope;
						}
						if (!light_walls) m->cells[c].fov = false;
					}
				}
				processed_cell++;
			}
			if (iteration == max_radius) done = true;
			iteration++;
			obstacles_in_last_line = total_obstacles;
			x += dx;
			if (x < 0 || x >= m->width) done = true;
		}
	}
}

void TCOD_map_compute_fov_restrictive_shadowcasting(TCOD_map_t map, int player_x, int player_y, int max_radius, bool light_walls) {
	map_t *m = (map_t *)map;
	int c;
	int max_obstacles;
	/*first, zero the FOV map */
	for(c = m->nbcells - 1; c >= 0; c--) m->cells[c].fov = false;

	/*calculate an approximated (excessive, just in case) maximum number of obstacles per octant */
	max_obstacles = m->nbcells / 7;

	/* check memory for angles */
	if (max_obstacles > allocated) {
		allocated = max_obstacles;
		if (start_angle != NULL) free(start_angle);
		if (end_angle != NULL) free(end_angle);
		start_angle = (double*)calloc(max_obstacles, sizeof(double));
		end_angle = (double*)calloc(max_obstacles, sizeof(double));
	}

	/*set PC's position as visible */
	m->cells[player_x+(player_y*m->width)].fov = true;

	/*compute the 4 quadrants of the map */
	TCOD_map_compute_fov_restrictive_shadowcasting_quadrant (m, player_x, player_y, max_radius, light_walls, 1, 1);
	TCOD_map_compute_fov_restrictive_shadowcasting_quadrant (m, player_x, player_y, max_radius, light_walls, 1, -1);
	TCOD_map_compute_fov_restrictive_shadowcasting_quadrant (m, player_x, player_y, max_radius, light_walls, -1, 1);
	TCOD_map_compute_fov_restrictive_shadowcasting_quadrant (m, player_x, player_y, max_radius, light_walls, -1, -1);
}
