//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "hudelement.h"
#include "hud_numericdisplay.h"
#include <vgui_controls/Panel.h>
#include "cbase.h"
#include "hud.h"
#include "hud_suitpower.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include <vgui_controls/AnimationController.h>
#include <vgui/ISurface.h>
#include "c_basehlplayer.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
/* //BG2 - No flashlights in BG2 -HairyPotter
//-----------------------------------------------------------------------------
// Purpose: Shows the flashlight icon
//-----------------------------------------------------------------------------
class CHudFlashlight : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudFlashlight, vgui::Panel );

public:
	CHudFlashlight( const char *pElementName );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

protected:
	virtual void Paint();

private:
	void SetFlashlightState( bool flashlightOn );
	void Reset( void );
	
	bool	m_bFlashlightOn;
	CPanelAnimationVar( vgui::HFont, m_hFont, "Font", "WeaponIconsSmall" );
	CPanelAnimationVarAliasType( float, m_IconX, "icon_xpos", "4", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_IconY, "icon_ypos", "4", "proportional_float" );
	
	CPanelAnimationVarAliasType( float, m_flBarInsetX, "BarInsetX", "2", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarInsetY, "BarInsetY", "18", "proportional_float" );

	CPanelAnimationVarAliasType( float, m_flBarWidth, "BarWidth", "28", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarHeight, "BarHeight", "2", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarChunkWidth, "BarChunkWidth", "2", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarChunkGap, "BarChunkGap", "2", "proportional_float" );
};	

using namespace vgui;

#ifdef HL2_EPISODIC
DECLARE_HUDELEMENT( CHudFlashlight );
#endif // HL2_EPISODIC

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudFlashlight::CHudFlashlight( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "HudFlashlight" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pScheme - 
//-----------------------------------------------------------------------------
void CHudFlashlight::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings(pScheme);
}

//-----------------------------------------------------------------------------
// Purpose: Start with our background off
//-----------------------------------------------------------------------------
void CHudFlashlight::Reset( void )
{
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "SuitFlashlightOn" ); 
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
void CHudFlashlight::SetFlashlightState( bool flashlightOn )
{
	if ( m_bFlashlightOn == flashlightOn )
		return;

	m_bFlashlightOn = flashlightOn;
}

#define WCHAR_FLASHLIGHT_ON  (L'©')
#define WCHAR_FLASHLIGHT_OFF (L'®')

//-----------------------------------------------------------------------------
// Purpose: draws the flashlight icon
//-----------------------------------------------------------------------------
void CHudFlashlight::Paint()
{
}*/
