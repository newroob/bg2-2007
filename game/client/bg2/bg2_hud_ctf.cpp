/*
	This is the hud interface for CTF mode. Essentially a direct copy of Tjoppen's flag HUD... for now. -HairyPotter
*/
#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "c_playerresource.h"
#include "clientmode_hl2mpnormal.h"
#include <vgui_controls/controls.h>
#include <vgui_controls/panel.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <KeyValues.h>
#include "c_baseplayer.h"
#include "c_team.h"
#include "c_ctfflag.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

enum
{
	TEAM_AMERICANS = 2,
	TEAM_BRITISH,
	//BG2 - Tjoppen - NUM_TEAMS is useful
	NUM_TEAMS,	//!! must be last !!
};

#define MAX_FLAGS	8

//==============================================
// CHudFlags
// Displays flag status
//==============================================
class CHudCTFFlags : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudCTFFlags, vgui::Panel );
public:
	CHudCTFFlags( const char *pElementName );
	void Init( void );
	void VidInit( void );
	virtual bool ShouldDraw( void );
	virtual void Paint( void );
	virtual void ApplySchemeSettings( vgui::IScheme *scheme );
	//void MsgFunc_flagstatus( bf_read &msg );

private:

	CHudTexture		*m_pIconBlank,
					*m_pIconRed,
					*m_pIconBlue;

	vgui::Label * m_pLabelFlag[MAX_FLAGS]; 

};

using namespace vgui;

DECLARE_HUDELEMENT( CHudCTFFlags );

//==============================================
// CHudFlags's CHudFlags
// Constructor
//==============================================
CHudCTFFlags::CHudCTFFlags( const char *pElementName ) :
	CHudElement( pElementName ), BaseClass( NULL, "HudFlags" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );
	Color ColourWhite( 255, 255, 255, 255 );
	SetHiddenBits( HIDEHUD_MISCSTATUS );
	SetSize( ScreenWidth(), ScreenHeight() ); //For whatever reason, this has to be set, otherwise the game won't set the dimensions properly. -HairyPotter
	for( int i = 0; i < MAX_FLAGS; i++ )
	{
		m_pLabelFlag[i] = new vgui::Label( this, "RoundState_warmup", "omg r teh lolzors" );
		m_pLabelFlag[i]->SetPaintBackgroundEnabled( false );
		m_pLabelFlag[i]->SetPaintBorderEnabled( false );
		m_pLabelFlag[i]->SizeToContents();
		m_pLabelFlag[i]->SetContentAlignment( vgui::Label::a_west );
		m_pLabelFlag[i]->SetFgColor( ColourWhite );
		m_pLabelFlag[i]->SetVisible(false);
	}

	m_pIconBlank = m_pIconRed = m_pIconBlue;
}
 extern ConVar cl_flagstatus;
//==============================================
// CHudFlags's ApplySchemeSettings
// applies the schemes
//==============================================
void CHudCTFFlags::ApplySchemeSettings( IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );
	vgui::HFont font = scheme->GetFont( "DracoLucidaRawks" );
	for( int i = 0; i < MAX_FLAGS; i++ )
	{
		m_pLabelFlag[i]->SetFont(font);
	}
	SetPaintBackgroundEnabled( false );
}

//==============================================
// CHudFlags's Init
// Inits any vars needed
//==============================================
void CHudCTFFlags::Init( void )
{	
}

//==============================================
// CHudFlags's VidInit
// Inits any textures needed
//==============================================
void CHudCTFFlags::VidInit( void )
{
	m_pIconBlank	= gHUD.GetIcon( "hud_flagicon_blank" );
	m_pIconRed		= gHUD.GetIcon( "hud_flagicon_red" );
	m_pIconBlue		= gHUD.GetIcon( "hud_flagicon_blue" );
}
int m_iFlagCount = g_CtfFlags.Count();
//==============================================
// CHudFlags's ShouldDraw
// whether the panel should be drawing
//==============================================
bool CHudCTFFlags::ShouldDraw( void )
{

	if ( !g_CtfFlags.Count() ) //No flags? Die here. -HairyPotter
		return false;

	return CHudElement::ShouldDraw();
}

//==============================================
// CHudFlags's Paint
// errr... paints the panel
//==============================================
void CHudCTFFlags::Paint()
{
	int m_iFlagCount = g_CtfFlags.Count();

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if( !pPlayer )
		return;

	//char text[512];
	int i;// = 0;
	int x_offset = 0;
	//int y_offset = 5;
	Color ColourWhite( 255, 255, 255, 255 );

	i = 0;
	while (i < MAX_FLAGS)
	{
		m_pLabelFlag[i]->SetVisible(false);
		i++;
	}
	
	//x_offset = ( (ScreenWidth() / 2) - (m_iFlagCount * 74) ); //Always lean to the left from the center. -HairyPotter
	x_offset = 0;

	for( i = 0; i < m_iFlagCount; i++ )
	{

		x_offset += 68;

		float fTimeToCap = gpGlobals->curtime;
		//switch( g_Flags[i]->m_iLastTeam )
		switch( g_CtfFlags[i]->GetTeamNumber() )
		{
		case TEAM_UNASSIGNED:
			switch( g_CtfFlags[i]->m_iForTeam )
			{
			case 0:
				m_pIconBlank->DrawSelf( x_offset, 0, ColourWhite );
				break;
			case 1:
				m_pIconRed->DrawSelf( x_offset, 0, ColourWhite );
				break;
			case 2:
				m_pIconBlue->DrawSelf( x_offset, 0, ColourWhite );
				break;
			}
			break;
		case TEAM_AMERICANS:
			m_pIconBlue->DrawSelf( x_offset, 0, ColourWhite );
			break;
		case TEAM_BRITISH:
			m_pIconRed->DrawSelf( x_offset, 0, ColourWhite );
			break;
		}

		if ( g_CtfFlags[i]->m_bIsCarried )
		{
			int r=0,g=0,b=0;
			
			//Start at blue.
			r = 0;
			g = 0;
			b = 255;
			//End up at red.
			r += 255 * (sin(fTimeToCap*4) + 1)/2;
			//g -= 100 * (sin(gpGlobals->curtime*4) + 1)/2;
			b -= 255 * (sin(fTimeToCap*4) + 1)/2;

			m_pLabelFlag[i]->SetText( "Taken" );
			m_pLabelFlag[i]->SizeToContents();
			//m_pLabelFlag[i]->SetVisible( true );

			//center on icon
			int w,h;
			m_pLabelFlag[i]->GetSize( w, h );
			m_pLabelFlag[i]->SetPos( (x_offset + 32) - w/2, 32 - h/2 );

			m_pLabelFlag[i]->SetFgColor( Color(r,g,b,255) );
			m_pLabelFlag[i]->SetVisible(true);

		}
	}
}