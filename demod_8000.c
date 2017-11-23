// Part of dump1090, a Mode S message decoder for SDR devices.
//
// demod_8000.c: 8MHz Mode S demodulator.
//
// Author: Tim Thorpe, tim.thorpe@islanddsp.com
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

#include "dump1090.h"


void pick_peak (int *match, int *peak_short, int *peak_long); 	
void shift_bytes (unsigned char *msg, int len);

/*
 * Demodulator for 16-bit, 8MHz magnitude array.
 *
 * Basic method: a sliding window of length 56 bits is used to locate
 * data bursts of 56 or 112 bits. The location criterion is the summed
 * magnitude of data transitions spaced 8 samples (1 bit period) apart.
 * The data-block location generally corresponds to the peak of this match
 * value, but to allow for noise, a few bits before and after are also 
 * checked for plausible decoded messages. This search is only triggered
 * when the match exceeds the long-term average noise value by a specified
 * factor.
 *
 * For efficiency, the sliding window is implemented by adding a sample
 * to the leading edge and subtracting one from the trailing edge. This
 * is fragile because any lack of balance would cause the sum to diverge.
 * Therefore, take care if modifying any of the buffering or wrap-around
 * indexing.
 *
 */


void demodulate8000(struct mag_buf *mag)
{
    struct modesMessage     mm;
    int                     message_result;
    unsigned char           msg[MODES_LONG_MSG_BYTES+D8M_SEARCH_BYTES];
    unsigned char           best_msg[MODES_LONG_MSG_BYTES];
    unsigned char           data_byte = 0;

    int                     phase_av, max, best_phase;
    int                     short_msg_offset = 0, long_msg_offset = 0;
    int                     sptr, eptr, dptr;
    int                     *dbuf;
    int                     sum;
    int                     i, j;    

    uint16_t *m = mag->data;
    int mlen = (int) mag->length;

    /* local variables initialized from static storage */

    int phase_av_acc            = Modes.d8m_phase_av_acc;
    int backtrack_phase_av_acc  = Modes.d8m_backtrack_phase_av_acc;
    int window                  = Modes.d8m_window;
    int win_start               = Modes.d8m_win_start;
    int start_phase             = Modes.d8m_start_phase;

    int phase[D8M_NUM_PHASES];
    int backtrack_phase[D8M_NUM_PHASES];
    int match_ar[D8M_WIN_LEN];
    int phase_ar[D8M_WIN_LEN];

    memcpy (phase, Modes.d8m_phase, sizeof(phase));
    memcpy (backtrack_phase, Modes.d8m_backtrack_phase, sizeof(backtrack_phase));
    memcpy (match_ar, Modes.d8m_match_ar, sizeof(match_ar));
    memcpy (phase_ar, Modes.d8m_phase_ar, sizeof(phase_ar));

    memset(&mm, 0, sizeof(mm));


    /* for code below, mlen must be divisible by 8. It should be, but just in case, force it.
       This would discard the last few samples of input.
    */

    mlen &= ~0x7;

	/* first we calculate the 4-sample diff value. This is convenient because both magnitude
       match and decoded data are based on this value, so we avoid recalculation.
    */

    dbuf = &(Modes.d8m_dbuf[D8M_BUF_OVERLAP]);	// point to start of new data

    for (j = 0; j < mlen; j++)
    {
        dbuf[j] = m[j];
        dbuf[j] -= m[j+4];          // +4 OK because there are Modes.trailing_samples extra.
    }

	/* now point to a location which allows the algorithm both some look-back
       and look-ahead in the data.
    */

    dbuf = &(Modes.d8m_dbuf[D8M_LOOK_BACK]);

	/* sliding window start and end points */
    sptr = 0;
    eptr = MODES_SHORT_MSG_BITS * D8M_NUM_PHASES;

	/* loop iterates one bit at a time, but calculates separate matches (phase[n])
       for each phase within the bit-period, 8 at 8MHz sampling. Effectively we have
       8 distict sliding windows to choose between.
    */
    while (sptr < mlen)
    {
		/* update window */
        max = 0;
        for (i = 0; i < D8M_NUM_PHASES; i++)
        {
            phase[i] += abs(dbuf[eptr++]); 
            phase[i] -= abs(dbuf[sptr++]);  
            if (phase[i] > max) max = phase[i];
        }

		/* low pass filter to get long-term S+N (mostly N) value */
        phase_av_acc += phase[0];
        phase_av = phase_av_acc >> 14;		// phase_av is current output
        phase_av_acc -= phase_av;			// phase_av_acc is filter memory

		/* This code first triggers when max exceeds noise by given factor.
		   Once triggered, it continues for 0 <= window < WIN_LEN, ie
           WIN_LEN contiguous bits. On the last bit, the peak-finding
           routine is called to locate the data-block. Then, the values
           are all set back by 56 bits before resuming. This is because,
           for long messages, it is not possible to detect peaks in the
           last 56 locations without more look-ahead.
        */
        if (window || (max * 2 > phase_av * 3))
        {
            // note which of 8 phases gives the greatest match.
			max = best_phase = 0;
            for (i = 0; i < D8M_NUM_PHASES; i++)  
                if (phase[i] > max) {best_phase = i; max = phase[i];}

			// on first bit, record start of match window and best phase
            if (window == 0)
            {
                win_start = sptr;
                start_phase = best_phase;
            }

			// record match value and best phase in arrays
            match_ar[window] = phase[start_phase];  // use same phase consistently, even if not best
            phase_ar[window] = best_phase;          // but record the best for later use


			// save intermediate values 56 before end of match window
            if (window == D8M_WIN_LEN - MODES_SHORT_MSG_BITS)
            {
                memcpy (backtrack_phase, phase, sizeof(phase));
                backtrack_phase_av_acc = phase_av_acc;
            }

			// end of match window, now locate peaks and look for valid messages
            if (++window == D8M_WIN_LEN)
            {
                int best_result = -1; int msg_bytes, msg_type, position;
                
                window = 0; // reset trigger value

                pick_peak (match_ar, &short_msg_offset, &long_msg_offset);

                /* Now we've located the match peak, decode the bits to look for
                   plausible message. We search twice, once around putative short-message
                   peak, then long-message peak.
				*/
				
				msg_bytes = MODES_SHORT_MSG_BYTES;
                dptr = win_start + phase_ar[short_msg_offset] + (short_msg_offset - D8M_SEARCH_BACK) * D8M_NUM_PHASES;
                position = dptr;

                for (msg_type = 0; msg_type < 2; msg_type++)
                {
                    // decode enough bits to search +- a few bits for message
					for (i = 0; i < msg_bytes + D8M_SEARCH_BYTES; i++)
                    {
                        data_byte = 0;

                        for (j = 0; j < 8; j++ )
                        {
                            sum = dbuf[dptr-1] + dbuf[dptr] + dbuf[dptr+1];   
                            sum = (sum >> 31) & 0x1;                        //sign gives data bit
                            data_byte = (data_byte << 1) | sum;
                            dptr += D8M_NUM_PHASES;
                        }

                        msg[i] = ~data_byte; // data was inverted
                    }

                    // search for messages by shifting data one bit and re-testing
					for (i = 0; i < D8M_SEARCH_WIDTH; i++)
                    {
                        message_result = scoreModesMessage(msg, msg_bytes * 8);

                        if (message_result > best_result)
                        {
                            memcpy (best_msg, msg, msg_bytes);	//most plausible message so far
                            best_result = message_result;
                            position = dptr - 64 + i * 8;		//position recorded for mlat
                        }

                        shift_bytes (msg, msg_bytes + D8M_SEARCH_BYTES); // shift by one bit
                    }

                    msg_bytes = MODES_LONG_MSG_BYTES;
                    dptr = win_start + phase_ar[long_msg_offset] + (long_msg_offset - D8M_SEARCH_BACK) * D8M_NUM_PHASES;
                }


                // Decode the received message
                if (best_result >= 0)
                {
                    mm.bFlags = mm.correctedbits  = 0;

                    // Set initial mm structure details
                    mm.timestampMsg = mag->sampleTimestamp + (position - D8M_LOOK_AHEAD) * 12 / 8;

                    // compute message receive time as block-start-time + difference in the 12MHz clock
                    mm.sysTimestampMsg = mag->sysTimestamp; // start of block time
                    mm.sysTimestampMsg.tv_nsec += receiveclock_ns_elapsed(mag->sampleTimestamp, mm.timestampMsg);
                    normalize_timespec(&mm.sysTimestampMsg);

                    message_result = decodeModesMessage(&mm, best_msg);

                    if ((mm.addr) && (message_result >= 0)) useModesMessage(&mm);
                }

                // now backtrack by 56 bits, as we may have missed peaks in this region
                sptr -= (MODES_SHORT_MSG_BITS-1) * D8M_NUM_PHASES;
                eptr -= (MODES_SHORT_MSG_BITS-1) * D8M_NUM_PHASES;
                memcpy (phase, backtrack_phase, sizeof(phase));
                phase_av_acc = backtrack_phase_av_acc;
            }  
        }
    }

    /* copy overlapped part of buffer from end to beginning of array */

    memcpy (&(Modes.d8m_dbuf[0]), &(Modes.d8m_dbuf[mlen]), D8M_BUF_OVERLAP *  sizeof(int));

    /* copy local variables back to static struct */

    Modes.d8m_phase_av_acc = phase_av_acc;
    Modes.d8m_backtrack_phase_av_acc = backtrack_phase_av_acc;
    Modes.d8m_window = window;
    Modes.d8m_win_start = win_start - mlen;
    Modes.d8m_start_phase = start_phase;

    memcpy (Modes.d8m_phase, phase, sizeof(phase));
    memcpy (Modes.d8m_backtrack_phase, backtrack_phase, sizeof(backtrack_phase));
    memcpy (Modes.d8m_match_ar, match_ar, sizeof(match_ar));
    memcpy (Modes.d8m_phase_ar, phase_ar, sizeof(phase_ar));
}

/*
	Find maxima in the match array corresponding to short messages (56 bits)
    and long messages (112 bits).
*/

void pick_peak (int *match, 			// input array of D8M_WIN_LEN match values
                int *peak_short, 		// returned location of peak 56
                int *peak_long) 		// returned location of peak 112
{
    int i, match112;
    int max0 = 0, max1 = 0;

    for (i = 0; i < D8M_WIN_LEN - MODES_SHORT_MSG_BITS; i++)
    {
        if (match[i] >= max0)
        {
            max0 = match[i]; 
            *peak_short = i;
        }

		// synthesise 112-bit match by adding two 56-bit matches
        match112 = match[i] + match[i+MODES_SHORT_MSG_BITS];


        if (match112 >= max1)
        {
            max1 = match112; 
            *peak_long = i;
        }
    }
}

/* 1-bit shift towards MSB[0] in an array of bytes of length len */

void shift_bytes (unsigned char *msg, int len)
{
    int i;
    for (i = 0; i < len - 1; i++)
        msg[i] = (msg[i] << 1) | (msg[i+1] >> 7);
    msg[i] <<= 1;
}

/******************************************************************************************/
