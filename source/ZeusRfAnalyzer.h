#ifndef ZEUSRF_ANALYZER_H
#define ZEUSRF_ANALYZER_H

#include <Analyzer.h>
#include "ZeusRfAnalyzerResults.h"
#include "ZeusRfSimulationDataGenerator.h"

class ZeusRfAnalyzerSettings;
class ANALYZER_EXPORT ZeusRfAnalyzer : public Analyzer2
{
public:
	ZeusRfAnalyzer();
	virtual ~ZeusRfAnalyzer();

	virtual void SetupResults();
	virtual void WorkerThread();

	virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
	virtual U32 GetMinimumSampleRateHz();

	virtual const char* GetAnalyzerName() const;
	virtual bool NeedsRerun();

protected: //vars
	void GetPairTransitions(U64* pos_start, U64* pos_end, U32* width_high, U32* width_low);
	void MarkByte(U64 start, U64 end, U8 data);
	void MarkSyncBit(U64 pos);
	void AdvanceUntilHigh();
	
	std::auto_ptr< ZeusRfAnalyzerSettings > mSettings;
	std::auto_ptr< ZeusRfAnalyzerResults > mResults;
	AnalyzerChannelData* mSerial;

	ZeusRfSimulationDataGenerator mSimulationDataGenerator;
	bool mSimulationInitilized;

	//Serial analysis vars:
	U32 mSampleRateHz;
	U32 mStartOfStopBitOffset;
	U32 mEndOfStopBitOffset;
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif //ZEUSRF_ANALYZER_H
