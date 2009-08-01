#include "cbase.h"
#include "bg2_maptriggers.h"
void CMapTrigger::Spawn( void )
{
	BaseClass::Spawn( );
}
void CMapTrigger::AmericanRoundWin( void )
{
	m_OnAmericanWinRound.FireOutput( this, this );
}
void CMapTrigger::AmericanMapWin( void )
{
	m_OnAmericanWinMap.FireOutput( this, this );
}
void CMapTrigger::BritishRoundWin( void )
{
	m_OnBritishWinRound.FireOutput( this, this );
}
void CMapTrigger::BritishMapWin( void )
{
	m_OnBritishWinMap.FireOutput( this, this );
}
void CMapTrigger::Draw( void )
{
	m_OnDraw.FireOutput( this, this );
}

BEGIN_DATADESC( CMapTrigger )

	DEFINE_OUTPUT( m_OnAmericanWinRound, "OnAmericanWinRound" ),
	DEFINE_OUTPUT( m_OnBritishWinRound, "OnBritishWinRound" ),
	DEFINE_OUTPUT( m_OnAmericanWinMap, "OnAmericanWinMap" ),
	DEFINE_OUTPUT( m_OnBritishWinMap, "OnBritishWinMap" ),
	DEFINE_OUTPUT( m_OnDraw, "OnDraw" ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( bg2_maptrigger, CMapTrigger);