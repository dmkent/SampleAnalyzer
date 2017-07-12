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
