/*
 * This file is part of dmrshark.
 *
 * dmrshark is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * dmrshark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with dmrshark.  If not, see <http://www.gnu.org/licenses/>.
**/

#ifndef TTYCONSOLE_H_
#define TTYCONSOLE_H_

void ttyconsole_send(char *buffer, unsigned int buffer_length);
void ttyconsole_print(const char *format, ...);

void ttyconsole_process(void);
void ttyconsole_init(void);
void ttyconsole_deinit(void);

#endif
