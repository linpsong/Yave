/*******************************
Copyright (C) 2009-2010 grégoire ANGERAND

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************/

#ifndef N_DEFINES
#define N_DEFINES

namespace n {
void fatal(const char *msg, const char *file = 0, int line = 0);
}

/* defines stuffs here */

#define N_PI 3.1415926535897932384626433832795028841971693993751058

#define nUnused(var) (void)(var)

#ifdef N_DEBUG
#define nError(msg) n::fatal((msg), __FILE__, __LINE__)
#else
#define nError(msg) n::fatal((msg))
#endif

#ifndef __GNUC__
#define N_NO_FORCE_INLINE
#endif

#ifdef N_NO_FORCE_INLINE
#define N_FORCE_INLINE /*nothing*/
#else
#define N_FORCE_INLINE __attribute__((always_inline))
#endif



/****************** COMPILATION DEFINES BELOW ******************/


#define N_AUTO_TEST

#endif //N_DEFINES
