// Part of dump1090, a Mode S message decoder for RTLSDR devices.
//
// demod_8000.h: 8MHz Mode S demodulator prototypes.
//
// This file is free software: you may copy, redistribute and/or modify it  
// under the terms of the GNU General Public License as published by the
// Free Software Foundation, either version 2 of the License, or (at your  
// option) any later version.  
//
// This file is distributed in the hope that it will be useful, but  
// WITHOUT ANY WARRANTY; without even the implied warranty of  
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License  
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef DUMP1090_DEMOD_8000_H
#define DUMP1090_DEMOD_8000_H

#include <stdint.h>



#define D8M_NUM_PHASES      8                                                   // samples per bit

#define D8M_WIN_LEN         (MODES_SHORT_MSG_BITS + MODES_LONG_MSG_BITS)        // match window to search for peak correlation

#define D8M_SEARCH_BACK     4                                                   // bits to search back relative to peak
#define D8M_SEARCH_AHEAD    12                                                  // bits to search ahead relative to peak
#define D8M_SEARCH_WIDTH    (D8M_SEARCH_BACK+D8M_SEARCH_AHEAD)                  // total search width in bits
#define D8M_SEARCH_BYTES    ((D8M_SEARCH_WIDTH+7)/8)                            // same in bytes, rounded up

#define D8M_LOOK_BACK       ((D8M_WIN_LEN+D8M_SEARCH_BACK+1)*D8M_NUM_PHASES)            // buffer look-back required for algorithm
#define D8M_LOOK_AHEAD      ((MODES_SHORT_MSG_BITS+D8M_SEARCH_AHEAD)*D8M_NUM_PHASES)    // buffer look-ahead required for algorithm

#define D8M_BUF_OVERLAP     (D8M_LOOK_BACK+D8M_LOOK_AHEAD)                              // total extra buffer compared to frame of data



struct mag_buf;

void demodulate8000(struct mag_buf *mag);

#endif
