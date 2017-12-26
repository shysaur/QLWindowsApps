/* log.h - Messaging routines.
 *
 * Copyright (C) 2017 Daniele Cattaneo
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LOG_H
#define LOG_H

#define LOG_LEVEL_DEBUG (0)
#define LOG_LEVEL_CRITICAL (1)

void log_msg(int level, const char *msg, ...);

#define dbg_log(...) log_msg(LOG_LEVEL_DEBUG, __VA_ARGS__)
#define crit_log(...) log_msg(LOG_LEVEL_NORMAL, __VA_ARGS__)

#endif
