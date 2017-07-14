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

void ZeusRfAnalyzer::WorkerThread()
{
	mSampleRateHz = GetSampleRate();

	mSerial = GetAnalyzerChannelData( mSettings->mInputChannel );

	if( mSerial->GetBitState() == BIT_LOW )
		mSerial->AdvanceToNextEdge();

	U64 nhigh, nlow, starting_sample;
	U64 last = mSerial->GetSampleNumber();
	U64 SAMPLES_PREAMBLE_MIN = 14100 * 1E-6 * mSampleRateHz;
	U64 SAMPLES_PREAMBLE_INIT_FLAG = 15000 * 1E-6 * mSampleRateHz;
	U64 SAMPLES_PREAMBLE_MAX = 16450 * 1E-6 * mSampleRateHz + 100;
	U64 SAMPLES_PAUSE_MIN = 1210 * 1E-6 * mSampleRateHz - 40;
	U64 SAMPLES_PAUSE_MAX = 1210 * 1E-6 * mSampleRateHz + 40;
	U64 SAMPLES_PRE_HIGH_MIN = 275 * 1E-6 * mSampleRateHz;
	U64 SAMPLES_PRE_HIGH_MAX = 540 * 1E-6 * mSampleRateHz;
	U64 SAMPLES_PRE_LOW_MIN = 235 * 1E-6 * mSampleRateHz;
	U64 SAMPLES_PRE_LOW_MAX = 500 * 1E-6 * mSampleRateHz;

	for( ; ; )
	{
		mSerial->AdvanceToNextEdge(); //falling edge -- beginning of the start bit

		U64 current = mSerial->GetSampleNumber();
		U64 width = current - last;

		//std::cout << width << " - " << SAMPLES_INTRO << "\n";
		if ((SAMPLES_PREAMBLE_MIN < width) && (width < SAMPLES_PREAMBLE_MAX)) {
			//std::cout << width << " - " << SAMPLES_INTRO << " - HIT\n";
			mResults->AddMarker(mSerial->GetSampleNumber(), AnalyzerResults::Square, mSettings->mInputChannel );
		} else {
			last = current;
			continue;
		}

		if (width > SAMPLES_PREAMBLE_INIT_FLAG) {
			last = current;
			mSerial->AdvanceToNextEdge();
			current = mSerial->GetSampleNumber();
			width = current - last;
			//std::cout << width << " - " << SAMPLES_PAUSE << "\n";
			if ((SAMPLES_PAUSE_MIN < width) && (width < SAMPLES_PAUSE_MAX)) {
				mResults->AddMarker(mSerial->GetSampleNumber(), AnalyzerResults::Dot, mSettings->mInputChannel );
			} else {
				last = current;
				continue;
			}
		}
		last = current;

		mResults->CommitResults();
		ReportProgress( current );

		U8 failed = 0;
		for(int i = 0; i < 11; i++) {
			mSerial->AdvanceToNextEdge();
			current = mSerial->GetSampleNumber();
			width = current - last;
			//std::cout << current << " - " << i << " - " << width << " - " << SAMPLES_PRE_HIGH << "\n";
			if ((SAMPLES_PRE_HIGH_MIN < width) && (width < SAMPLES_PRE_HIGH_MAX)) {
				mResults->AddMarker(mSerial->GetSampleNumber(), AnalyzerResults::Dot, mSettings->mInputChannel );
			} else {
				last = current;
				failed = 1;
				break;
			}
			last = current;

			mSerial->AdvanceToNextEdge();
			current = mSerial->GetSampleNumber();
			width = current - last;
			//std::cout << current << " - " << i << " - " << width << " - " << SAMPLES_PRE_LOW << "\n";
			if ((SAMPLES_PRE_LOW_MIN < width) && (width < SAMPLES_PRE_LOW_MAX)) {
				mResults->AddMarker(mSerial->GetSampleNumber(), AnalyzerResults::Dot, mSettings->mInputChannel );
			} else {
				last = current;
				failed = 1;
				break;
			}
			last = current;

		}

		mSerial->AdvanceToNextEdge();
		current = mSerial->GetSampleNumber();
		width = current - last;
		//std::cout << current << " - " << width << " - " << SAMPLES_PRE_HIGH << "\n";
		if ((SAMPLES_PRE_HIGH_MIN < width) && (width < SAMPLES_PRE_HIGH_MAX)) {
			mResults->AddMarker(mSerial->GetSampleNumber(), AnalyzerResults::Dot, mSettings->mInputChannel );
		} else {
			last = current;
			failed = 1;
			break;
		}
		last = current;

		mResults->CommitResults();
		ReportProgress( mSerial->GetSampleNumber() );

		mSerial->AdvanceToNextEdge();
		current = mSerial->GetSampleNumber();
		last = current;

		if (failed > 0) {
			continue;
		}

		U8 data = 0;
		U8 mask = 1 << 7;
		U8 nbits = 0;
		U8 nbytes = 0;
		starting_sample = current;
		for( ;; ) {
			mSerial->AdvanceToNextEdge();
			current = mSerial->GetSampleNumber();
			nhigh = current - last;
			last = current;
			current = mSerial->GetSampleOfNextEdge();
			nlow = current - last;
			//std::cout << nhigh << " : " << nlow << "\n";
			if (nlow > SAMPLES_PREAMBLE_MIN) {
				Frame frame;
				frame.mData1 = data;
				frame.mFlags = 0;
				frame.mStartingSampleInclusive = starting_sample;
				frame.mEndingSampleInclusive = last;

				mResults->AddFrame( frame );
				mResults->CommitResults();
				ReportProgress( mSerial->GetSampleNumber() );
				break;
			}

			mSerial->AdvanceToNextEdge();
			last = current;

			data <<= 1;
			nbits++;
			if (nhigh < nlow) {
				data |= 1;
			}

			if (nbits == 8) {
				//we have a byte to save. 
				Frame frame;
				frame.mData1 = data;
				frame.mFlags = 0;
				frame.mStartingSampleInclusive = starting_sample;
				frame.mEndingSampleInclusive = mSerial->GetSampleOfNextEdge();

				mResults->AddFrame( frame );
				mResults->CommitResults();
				ReportProgress( mSerial->GetSampleNumber() );

				nbits = 0;
				data = 0;
				starting_sample = current;
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