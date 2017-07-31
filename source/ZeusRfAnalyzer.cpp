#include "ZeusRfAnalyzer.h"
#include "ZeusRfAnalyzerSettings.h"
#include <AnalyzerChannelData.h>

#include <iostream>
#include <functional>

#include "ZeusRfDecode.h"

using namespace std::placeholders;


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

void ZeusRfAnalyzer::GetPairTransitions(U64* pos_start, U64* pos_end, 
										U32* width_high, U32* width_low) {
	U64 last, current;
	
	if( mSerial->GetBitState() == BIT_LOW ) {
		std::cout << "unexpected BIT_LOW" << std::endl;
	}

	last = mSerial->GetSampleNumber();
	(*pos_start) = last;
	mSerial->AdvanceToNextEdge(); //falling edge -- beginning of low part of bit
	current = mSerial->GetSampleNumber();
	(*width_high) = (current - last) * 1E6 / mSampleRateHz;
	last = current;
	mSerial->AdvanceToNextEdge(); //rising edge -- beginning of the next bit
	current = mSerial->GetSampleNumber();
	(*width_low) = (current - last) * 1E6 / mSampleRateHz;	
	(*pos_end) = current;
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
	U64 data_start;
	mSampleRateHz = GetSampleRate();
	mSerial = GetAnalyzerChannelData( mSettings->mInputChannel );
	AdvanceUntilHigh();

	for( ; ; )
	{
		data_start = block_until_data(
			std::bind(&ZeusRfAnalyzer::AdvanceUntilHigh, this),
			std::bind(&ZeusRfAnalyzer::GetPairTransitions, this, _1, _2, _3, _4),
			std::bind(&ZeusRfAnalyzer::MarkSyncBit, this, _1),
			std::bind(&ZeusRfAnalyzer::MarkByte, this, _1, _2, _3)
		);
		
		// If reach long low pair then into data mode.
		receive_and_process_data(
			data_start,
			std::bind(&ZeusRfAnalyzer::AdvanceUntilHigh, this),
			std::bind(&ZeusRfAnalyzer::GetPairTransitions, this, _1, _2, _3, _4),
			std::bind(&ZeusRfAnalyzer::MarkSyncBit, this, _1),
			std::bind(&ZeusRfAnalyzer::MarkByte, this, _1, _2, _3)
		);
		
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