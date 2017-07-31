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

#include <functional>
#include <Analyzer.h>

#define SAMPLES_PREAMBLE_MIN 13950
#define SAMPLES_PAUSE_MIN 3550
#define SAMPLES_PAUSE_MAX 3800
#define MAX_PREAMBLE 12

U64 block_until_data(
    std::function<void()> AdvanceUntilHigh,
    std::function<void(U64*, U64*, U32*, U32*)> GetPairTransitions,
	std::function<void(U64)> MarkSyncBit,
    std::function<void(U64, U64, U8)> MarkByte
 );

void receive_and_process_data(
    U64 data_start,
    std::function<void()> AdvanceUntilHigh,
    std::function<void(U64*, U64*, U32*, U32*)> GetPairTransitions,
	std::function<void(U64)> MarkSyncBit,
    std::function<void(U64, U64, U8)> MarkByte 
);