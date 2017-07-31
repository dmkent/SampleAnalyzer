#include "ZeusRfAnalyzer.h"
#include "ZeusRfAnalyzerSettings.h"
#include <AnalyzerChannelData.h>

#include <iostream>

ZeusRfAnalyzer::ZeusRfAnalyzer()
:	Analyzer2(),  
	mSettings( new ZeusRfAnalyzerSettings() ),
	mSimulationInitilized( false )
{
	SetAnalyzerSettings( mSettings.get() );
}

ZeusRfAnalyzer::~ZeusRfAnalyzer()
{
	KillThread();
}

void ZeusRfAnalyzer::SetupResults()
{
	mResults.reset( new ZeusRfAnalyzerResults( this, mSettings.get() ) );
	SetAnalyzerResults( mResults.get() );
	mResults->AddChannelBubblesWillAppearOn( mSettings->mInputChannel );
}

void ZeusRfAnalyzer::GetPairTransitions(U64* pos_start, U64* width_high, U64* width_low) {
	U64 last, current;
	
	if( mSerial->GetBitState() == BIT_LOW ) {
		std::cout << "unexpected BIT_LOW" << std::endl;
	}

	last = mSerial->GetSampleNumber();
	(*pos_start) = last;
	mSerial->AdvanceToNextEdge(); //falling edge -- beginning of the start bit
	current = mSerial->GetSampleNumber();
	(*width_high) = current - last;
	last = current;
	mSerial->AdvanceToNextEdge(); //rising edge -- beginning of the start bit
	current = mSerial->GetSampleNumber();
	(*width_low) = current - last;		
}

void ZeusRfAnalyzer::MarkByte(U64 start, U64 end, U8 data) {
	Frame frame;
	frame.mData1 = data;
	frame.mFlags = 0;
	frame.mStartingSampleInclusive = start;
	frame.mEndingSampleInclusive = end;

	mResults->AddFrame( frame );
	mResults->CommitResults();
	ReportProgress( mSerial->GetSampleNumber() );
}

void ZeusRfAnalyzer::MarkSyncBit(U64 pos) {
	mResults->AddMarker(pos, AnalyzerResults::Dot, mSettings->mInputChannel );
	mResults->CommitResults();
	ReportProgress(pos);
}

void ZeusRfAnalyzer::AdvanceUntilHigh() {
	if( mSerial->GetBitState() == BIT_LOW )
		mSerial->AdvanceToNextEdge();
}

void ZeusRfAnalyzer::WorkerThread()
{
	U64 nhigh, nlow, starting_sample, pos_start,
	    width_high, width_low, exp_width_high, exp_width_low;
	
	mSampleRateHz = GetSampleRate();
	U64 SAMPLES_PREAMBLE_MIN = 13950 * 1E-6 * mSampleRateHz;
	U64 SAMPLES_PAUSE_MIN = 3550 * 1E-6 * mSampleRateHz - 100;
	U64 SAMPLES_PAUSE_MAX = 3770 * 1E-6 * mSampleRateHz + 100;
	U16 MAX_PREAMBLE = 12;
	
	mSerial = GetAnalyzerChannelData( mSettings->mInputChannel );

	AdvanceUntilHigh();
	
	for( ; ; )
	{
		AdvanceUntilHigh();
		
		// Get a single pair of transitions
		// Store width_low and width_high
		GetPairTransitions(&pos_start, &exp_width_high, &exp_width_low);
		
		// Loop over pairs while match previous
		U8 failed = 0, nmatched = 0;
		for( ; ; )
		{
			GetPairTransitions(&pos_start, &width_high, &width_low);

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

			MarkSyncBit(pos_start + width_high + width_low);
		}

		// If not matching then restart
		if (failed > 0) {
			continue;
		}

		// If reach long low pair then into data mode.
		U8 data = 0;
		U8 mask = 1 << 7;
		U8 nbits = 0;
		U8 nbytes = 0;
		starting_sample = pos_start + width_high + width_low;
		for( ;; ) {
			GetPairTransitions(&pos_start, &nhigh, &nlow);
			//std::cout << nhigh << " : " << nlow << "\n";
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
				MarkByte(starting_sample, pos_start + nhigh + nlow, data);
	
				nbits = 0;
				data = 0;
				starting_sample = pos_start + nhigh + nlow;
			}
		}

		mResults->CommitResults();
		ReportProgress( mSerial->GetSampleNumber() );
	}
}

bool ZeusRfAnalyzer::NeedsRerun()
{
	return false;
}

U32 ZeusRfAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels )
{
	if( mSimulationInitilized == false )
	{
		mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
		mSimulationInitilized = true;
	}

	return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 ZeusRfAnalyzer::GetMinimumSampleRateHz()
{
	return 4;//mSettings->mBitRate * 4;
}

const char* ZeusRfAnalyzer::GetAnalyzerName() const
{
	return "Zeus RF";
}

const char* GetAnalyzerName()
{
	return "Zeus RF";
}

Analyzer* CreateAnalyzer()
{
	return new ZeusRfAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
	delete analyzer;
}