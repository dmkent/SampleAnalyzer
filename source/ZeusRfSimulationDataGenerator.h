#ifndef ZEUSRF_SIMULATION_DATA_GENERATOR
#define ZEUSRF_SIMULATION_DATA_GENERATOR

#include <SimulationChannelDescriptor.h>
#include <string>
class ZeusRfAnalyzerSettings;

class ZeusRfSimulationDataGenerator
{
public:
	ZeusRfSimulationDataGenerator();
	~ZeusRfSimulationDataGenerator();

	void Initialize( U32 simulation_sample_rate, ZeusRfAnalyzerSettings* settings );
	U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel );

protected:
	ZeusRfAnalyzerSettings* mSettings;
	U32 mSimulationSampleRateHz;

protected:
	void CreateSerialByte();
	std::string mSerialText;
	U32 mStringIndex;

	SimulationChannelDescriptor mSerialSimulationData;

};
#endif //ZEUSRF_SIMULATION_DATA_GENERATOR