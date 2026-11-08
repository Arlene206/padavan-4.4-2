/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _nvram_x_h_
#define _nvram_x_h_

#define MAX_GROUP_COUNT		128

extern char *nvram_get_list_x(const char *name, int index);
extern int  nvram_match_list_x(const char *name, char *match, int index);
extern int nvram_del_group_map_x(const char * const *names, int name_count,\n\t\t\t\t const char *count_name, int group_count, int *delMap);


#endif /* _nvram_x_h_ */
