#ifndef ZEUSRF_ANALYZER_SETTINGS
#define ZEUSRF_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class ZeusRfAnalyzerSettings : public AnalyzerSettings
{
public:
	ZeusRfAnalyzerSettings();
	virtual ~ZeusRfAnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	void UpdateInterfacesFromSettings();
	virtual void LoadSettings( const char* settings );
	virtual const char* SaveSettings();

	
	Channel mInputChannel;
	U32 mBitRate;

protected:
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mInputChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceInteger >	mBitRateInterface;
};

#endif //ZEUSRF_ANALYZER_SETTINGS
