/*
 * Class to receive and decode signals from wireless sensors of an old
 * security system.
 *
 * See the README.md for more details.
 *
 * Copyright 2017 David M Kent.
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

#include "ZeusRfDecode.h"


INTPOS block_until_data(
    AdvanceHighFuncType AdvanceUntilHigh,
    PairTransFuncType GetPairTransitions,
	MarkSyncBitFuncType MarkSyncBit
) {
    INTPOS pos_start, pos_end;
    INTWID width_high, width_low, exp_width_high, exp_width_low;

    for( ; ; ) {
        
        AdvanceUntilHigh();
		
		// Get a single pair of transitions
		// Store width_low and width_high
		GetPairTransitions(&pos_start, &pos_end, &exp_width_high, &exp_width_low);
		
		// Loop over pairs while match previous
		BYTE failed = 0, nmatched = 0;
		for( ; ; )
		{
			GetPairTransitions(&pos_start, &pos_end, &width_high, &width_low);
            if (((exp_width_high * 0.9) <= width_high) &&
			    (width_high <= (exp_width_high * 1.1))) {
				// Matched on HIGH pulse.
				if (((exp_width_low * 0.9) <= width_low) &&
			        (width_low <= (exp_width_low * 1.1))) {
					// Matched on low as well
					nmatched++;
				} else if ((SAMPLES_PAUSE_MIN <= width_low) && (width_low <= SAMPLES_PAUSE_MAX)) {
					nmatched++;
					// Break without failure - into data mode
					break;
				} else {
					failed = 1;
					break;
				}
			} else {
				failed = 1;
				break;
			}

			if (nmatched > MAX_PREAMBLE) {
				failed = 1;
				break;
			}

			MarkSyncBit(pos_end);
		}

		// If not matching then restart
		if (failed > 0) {
			continue;
		}

        // Fell through here without failure == SUCESS!
        // return control read for data
        return pos_end;
    }
}

void receive_and_process_data(
    INTPOS data_start,
    PairTransFuncType GetPairTransitions,
	MarkByteFuncType MarkByte
){
    INTWID nhigh, nlow;
    INTPOS starting_sample, pos_start, pos_end;
    BYTE data = 0;
    BYTE mask = 1 << 7;
    BYTE nbits = 0;
    BYTE nbytes = 0;
    starting_sample = data_start;
    for( ;; ) {
        GetPairTransitions(&pos_start, &pos_end, &nhigh, &nlow);
        if (nlow > SAMPLES_PREAMBLE_MIN) {
            // Mark to start of this pair (we ignore this one)
            MarkByte(starting_sample, pos_start, data);
            break;
        }

        data <<= 1;
        nbits++;
        if (nhigh < nlow) {
            data |= 1;
        }

        if (nbits == 8) {
            //we have a byte to save. 
            MarkByte(starting_sample, pos_end, data);

            nbits = 0;
            data = 0;
            starting_sample = pos_end;
        }
    }
}