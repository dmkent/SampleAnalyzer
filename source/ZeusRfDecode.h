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
#pragma once

/* Types */
typedef unsigned char U8;
typedef unsigned int U32;
typedef unsigned long long int U64;


#ifdef ARDUINO
typedef void (*AdvanceHighFuncType)();
typedef U64 (*PairTransFuncType)(U64*, U64*, U32*, U32*);
typedef void(*MarkByteFuncType)(U64, U64, U8);
typedef void(*MarkSyncBitFuncType)(U64);    
#else
#include <functional>
typedef std::function<void()> AdvanceHighFuncType;
typedef std::function<void(U64*, U64*, U32*, U32*)> PairTransFuncType;
typedef std::function<void(U64, U64, U8)> MarkByteFuncType;
typedef std::function<void(U64)> MarkSyncBitFuncType;
#endif

/* Used to flag end of data (in micro seconds) */
#define SAMPLES_PREAMBLE_MIN 13950
/* Used to flag end of sync/start of data */
#define SAMPLES_PAUSE_MIN 3550
#define SAMPLES_PAUSE_MAX 3800
/* Max sync pairs expected */
#define MAX_PREAMBLE 12

/**
  * Progressivly work through the stream of data until we detect initial sync signal.
  *
  * This works by looking for pairs of transitions (one high pulse, one low) and comparing
  * their width to the previous pairs. If we get a series of pairs with same widths then
  * we know we have a signal. When we then get a long low pulse we know data is about to
  * start.
  *
  * Callbacks:
  *   AdvanceUntilHigh - should skip read pulses until current one is HIGH.
  *   GetPairTransitions - should get/wait for next pair of pulses. The values of
  *                        the four pointers should be set to:
  *                            1. Position of pair start (as position count)
  *                            2. Position of pair end (as position count)
  *                            3. Width of high pulse (in micro seconds)
  *                            4. Width of low pulse (in micro seconds)
  *   MarkSyncBit - called when a complete sync pair is detected giving the
  *                 end position count.
  *
  * Returns:
  *   The current position index for start of data.
  */
U64 block_until_data(
    AdvanceHighFuncType AdvanceUntilHigh,
    PairTransFuncType GetPairTransitions,
	MarkSyncBitFuncType MarkSyncBit
);

/**
  * Progressivly work through the stream of data returning decoded byte data.
  *
  * This applies on-off-keying (OOK) to decode data from pulses.
  *
  * Takes the initial position index as an argument.
  *
  * Callbacks:
  *   GetPairTransitions - should get/wait for next pair of pulses. The values of
  *                        the four pointers should be set to:
  *                            1. Position of pair start (as position count)
  *                            2. Position of pair end (as position count)
  *                            3. Width of high pulse (in micro seconds)
  *                            4. Width of low pulse (in micro seconds)
  *   MarkByte - called when a complete byte has been decoded. Arguments will be:
  *                  1. Starting position index of byte
  *                  2. Ending position index of byte
  *                  3. Byte value
  */
void receive_and_process_data(
    U64 data_start,
    PairTransFuncType GetPairTransitions,
	MarkByteFuncType MarkByte 
);