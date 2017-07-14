#include "ZeusRfAnalyzerSettings.h"
#include <AnalyzerHelpers.h>


ZeusRfAnalyzerSettings::ZeusRfAnalyzerSettings()
:	mInputChannel( UNDEFINED_CHANNEL )
{
	mInputChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mInputChannelInterface->SetTitleAndTooltip( "Serial", "Standard Zeus RF" );
	mInputChannelInterface->SetChannel( mInputChannel );

	AddInterface( mInputChannelInterface.get() );
	
	AddExportOption( 0, "Export as text/csv file" );
	AddExportExtension( 0, "text", "txt" );
	AddExportExtension( 0, "csv", "csv" );

	ClearChannels();
	AddChannel( mInputChannel, "Serial", false );
}

ZeusRfAnalyzerSettings::~ZeusRfAnalyzerSettings()
{
}

bool ZeusRfAnalyzerSettings::SetSettingsFromInterfaces()
{
	mInputChannel = mInputChannelInterface->GetChannel();
	
	ClearChannels();
	AddChannel( mInputChannel, "Zeus RF", true );

	return true;
}

void ZeusRfAnalyzerSettings::UpdateInterfacesFromSettings()
{
	mInputChannelInterface->SetChannel( mInputChannel );
}

void ZeusRfAnalyzerSettings::LoadSettings( const char* settings )
{
	SimpleArchive text_archive;
	text_archive.SetString( settings );

	text_archive >> mInputChannel;
	
	ClearChannels();
	AddChannel( mInputChannel, "Zeus RF", true );

	UpdateInterfacesFromSettings();
}

const char* ZeusRfAnalyzerSettings::SaveSettings()
{
	SimpleArchive text_archive;

	text_archive << mInputChannel;
	
	return SetReturnString( text_archive.GetString() );
}
