//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: module for gathering performance stats for upload so that we can
//  monitor performance regressions and improvements
//
//=====================================================================================//


#include "cbase.h"
//#include "statgather.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/*
#define STATS_WINDOW_SIZE ( 60 * 10 )						// # of records to hold
#define STATS_RECORD_INTERVAL 1								// # of seconds between data grabs. 2 * 300 = every 10 minutes


struct StatsBufferRecord_t
{
	float m_flFrameRate;									// fps

};

class CStatsRecorder
{

	StatsBufferRecord_t m_StatsBuffer[STATS_WINDOW_SIZE];
	bool m_bBufferFull;
	float m_flLastRealTime;
	float m_flLastSampleTime;

	template<class T> T AverageStat( T StatsBufferRecord_t::*field ) const
	{
		T sum = 0;
		for( int i = 0; i < STATS_WINDOW_SIZE; i++ )
			sum += m_StatsBuffer[i].*field;
		return sum / STATS_WINDOW_SIZE;
	}

	template<class T> T MaxStat( T StatsBufferRecord_t::*field ) const
	{
		T maxsofar = -16000000;
		for( int i = 0; i < STATS_WINDOW_SIZE; i++ )
			maxsofar = max( maxsofar, m_StatsBuffer[i].*field );
		return maxsofar;
	}

	template<class T> T MinStat( T StatsBufferRecord_t::*field ) const
	{
		T minsofar = 16000000;
		for( int i = 0; i < STATS_WINDOW_SIZE; i++ )
			minsofar = min( minsofar, m_StatsBuffer[i].*field );
		return minsofar;
	}

	inline void AdvanceIndex( void )
	{
		m_nWriteIndex++;
		if ( m_nWriteIndex == STATS_WINDOW_SIZE )
		{
			m_nWriteIndex = 0;
			m_bBufferFull = true;
		}
	}

public:
	int m_nWriteIndex;
	CStatsRecorder( void )
	{
		m_bBufferFull = false;
		m_nWriteIndex = 0;
		m_flLastRealTime = -1;
		m_flLastSampleTime = -1;
	}

	char const *GetPerfStatsString( void );
	void UpdatePerfStats( void );
};

char s_cPerfString[2048];

static inline char const *SafeString( char const *pStr )
{
	return ( pStr ) ? pStr : "?";
}

char const *CStatsRecorder::GetPerfStatsString( void )
{
	if ( ! m_bBufferFull )
		return NULL;

	float flAverageFrameRate = AverageStat( &StatsBufferRecord_t::m_flFrameRate );
	float flMinFrameRate = MinStat( &StatsBufferRecord_t::m_flFrameRate );
	float flMaxFrameRate = MaxStat( &StatsBufferRecord_t::m_flFrameRate );

	const CPUInformation &cpu = GetCPUInformation();
	MaterialAdapterInfo_t gpu;
	materials->GetDisplayAdapterInfo( materials->GetCurrentAdapter(), gpu );

	CMatRenderContextPtr pRenderContext( materials );
	int dest_width,dest_height;
	pRenderContext->GetRenderTargetDimensions( dest_width, dest_height );

	V_snprintf( s_cPerfString, sizeof( s_cPerfString ), 
				"PERFDATA:AvgFps=%4.2f;MinFps=%4.2f;MaxFps=%4.2f;CPUID=\"%s\";CPUGhz=%2.2f;"
				"NumCores=%d;GPUDrv=\"%s\";"
				"GPUVendor=%d;GPUDeviceID=%d;"
				"GPUDriverVersion=\"%d.%d\";DxLvl=%d;"
				"Width=%d;Height=%d",
				flAverageFrameRate,
				flMinFrameRate,
				flMaxFrameRate,
				cpu.m_szProcessorID,
				cpu.m_Speed * ( 1.0 / 1.0e9 ),
				cpu.m_nPhysicalProcessors,
				SafeString( gpu.m_pDriverName ),
				gpu.m_VendorID,
				gpu.m_DeviceID,
				gpu.m_nDriverVersionHigh,
				gpu.m_nDriverVersionLow,
				g_pMaterialSystemHardwareConfig->GetDXSupportLevel(),
				dest_width, dest_height
		);
	// get rid of chars that we hate in vendor strings
	for( char *i = s_cPerfString; *i; i++ )
	{
		if ( ( i[0]=='\n' ) || ( i[0]=='\r' )  || ( i[0]==';' ) )
			i[0]=' ';
	}

	// clear buffer
	m_nWriteIndex = 0;
	m_bBufferFull = false;
	
	return s_cPerfString;
}

void CStatsRecorder::UpdatePerfStats( void )
{
	float flCurTime = Plat_FloatTime();
	if (
		( m_flLastSampleTime == -1 ) || 
		( flCurTime - m_flLastSampleTime >= STATS_RECORD_INTERVAL ) )
	{
		if ( ( m_flLastRealTime > 0 ) && ( flCurTime > m_flLastRealTime ) )
		{
			float flFrameRate = 1.0 / ( flCurTime - m_flLastRealTime );
			StatsBufferRecord_t &stat = m_StatsBuffer[m_nWriteIndex];
			stat.m_flFrameRate = flFrameRate;
			AdvanceIndex();
			m_flLastSampleTime = flCurTime;
		}
	}
	m_flLastRealTime = flCurTime;
}

static CStatsRecorder s_StatsRecorder;

char const *GetPerfStatsString( void )
{
	return s_StatsRecorder.GetPerfStatsString();
}

void UpdatePerfStats( void )
{
	s_StatsRecorder.UpdatePerfStats();
}

static void ShowPerfStats( void )
{
	char const *pStr = GetPerfStatsString();
	if ( pStr )
		Warning( "%s\n", pStr );
	else
		Warning( "%d records stored. buffer not full.\n", s_StatsRecorder.m_nWriteIndex );
}

static ConCommand perfstats( "cl_perfstats", ShowPerfStats, "Dump the perf monitoring string" );


void UploadPerfData( void )
{
	if( g_pClientGameStatsUploader )
	{
		char const *pPerfData = GetPerfStatsString();

		if ( pPerfData )
		{
			g_pClientGameStatsUploader->UploadGameStats( "",
														 1,
														 1 + strlen( pPerfData ),
														 pPerfData );
		}
	}	
}*/
