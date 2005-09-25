/*
	The Battle Grounds 2 - A Source modification
	Copyright (C) 2005, The Battle Grounds 2 Team and Contributors

	The Battle Grounds 2 free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	The Battle Grounds 2 is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

	Contact information:
		Tomas "Tjoppen" Härdin		tjoppen@gamedev.se

	You may also contact the (future) team via the Battle Grounds website and/or forum at:
		www.bgmod.com

	 Note that because of the sheer volume of files in the Source SDK this
	notice cannot be put in all of them, but merely the ones that have any
	changes from the original SDK.
	 In order to facilitate easy searching, all changes are and must be
	commented on the following form:

	//BG2 - <name of contributer>[ - <small description>]
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
#include "c_flag.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar cl_flagstatus( "cl_flagstatus", "1", 0, "0 - Off, 1 - Text, 2 - Icons" );

enum
{
	TEAM_AMERICANS = 2,
	TEAM_BRITISH,
	//BG2 - Tjoppen - NUM_TEAMS is useful
	NUM_TEAMS,	//!! must be last !!
};

#define MAX_FLAGS	12

/*struct flag
{
	char m_szName[512];
	int m_iOwner;
	int m_iTimeToCap;
	int m_iRequiredPlayers;
	int m_iForTeam;
	int m_iTimeNeeded;
	int m_iCappers;
};*/

//==============================================
// CHudFlags
// Displays flag status
//==============================================
class CHudFlags : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudFlags, vgui::Panel );
public:
	CHudFlags( const char *pElementName );
	void Init( void );
	void VidInit( void );
	virtual bool ShouldDraw( void );
	virtual void Paint( void );
	virtual void ApplySchemeSettings( vgui::IScheme *scheme );
	//void MsgFunc_flagstatus( bf_read &msg );

private:

	CPanelAnimationVarAliasType( float, m_flLineHeight, "LineHeight", "15", "proportional_float" );

	CPanelAnimationVar( float, m_flMaxDeathNotices, "MaxDeathNotices", "4" );

	CPanelAnimationVar( bool, m_bRightJustify, "RightJustify", "1" );

	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "HudNumbersTimer" ); 

	CHudTexture		* m_IconFlag[MAX_FLAGS]; 
	CHudTexture		* m_IconCover[MAX_FLAGS]; 

	vgui::Label * m_pLabelFlag[MAX_FLAGS]; 
	
	//flag myflags[MAX_FLAGS];
	
	//int iNumFlags;
};

using namespace vgui;

DECLARE_HUDELEMENT( CHudFlags );
//DECLARE_HUD_MESSAGE( CHudFlags, flagstatus );

//==============================================
// CHudFlags's CHudFlags
// Constructor
//==============================================
CHudFlags::CHudFlags( const char *pElementName ) :
	CHudElement( pElementName ), BaseClass( NULL, "HudFlags" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );
	Color ColourWhite( 255, 255, 255, 255 );
	SetHiddenBits( HIDEHUD_MISCSTATUS );

	for( int i = 0; i < MAX_FLAGS; i++ )
	{
		m_IconFlag[i] = NULL;
		m_IconCover[i] = NULL;
		m_pLabelFlag[i] = new vgui::Label( this, "RoundState_warmup", "omg r teh lolzors" /*vgui::localize()->Find( "#Clan_warmup_mode" )*/ );
		m_pLabelFlag[i]->SetPaintBackgroundEnabled( false );
		m_pLabelFlag[i]->SetPaintBorderEnabled( false );
		m_pLabelFlag[i]->SizeToContents();
		m_pLabelFlag[i]->SetContentAlignment( vgui::Label::a_west );
		m_pLabelFlag[i]->SetFgColor( ColourWhite );
		m_pLabelFlag[i]->SetVisible(false);
	}
}

//==============================================
// CHudFlags's ApplySchemeSettings
// applies the schemes
//==============================================
void CHudFlags::ApplySchemeSettings( IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );
	SetPaintBackgroundEnabled( false );
}

//==============================================
// CHudFlags's Init
// Inits any vars needed
//==============================================
void CHudFlags::Init( void )
{	
	/*HOOK_HUD_MESSAGE( CHudFlags, flagstatus );
	int i = 0;
	iNumFlags = 0;
	while (i < MAX_FLAGS)
	{
		//STRING( g_Flags[i]->m_sFlagName ) = (char[512])"lolhi";
		myflags[i].m_iOwner = 0;
		myflags[i].m_iTimeToCap = 0;
		g_Flags[i]->m_iCapturePlayers = 0;
		myflags[i].m_iForTeam = 0;
		i++;
	}*/
}

//==============================================
// CHudFlags's VidInit
// Inits any textures needed
//==============================================
void CHudFlags::VidInit( void )
{
	int i = 0;
	while (i < MAX_FLAGS)
	{
		m_IconFlag[i] = gHUD.GetIcon( "draco_test" );
		m_IconCover[i] = gHUD.GetIcon( "draco_test" );
		i++;
	}
}

//==============================================
// CHudFlags's ShouldDraw
// whether the panel should be drawing
//==============================================
bool CHudFlags::ShouldDraw( void )
{
	return CHudElement::ShouldDraw();
}

//==============================================
// CHudFlags's Paint
// errr... paints the panel
//==============================================
void CHudFlags::Paint()
{
	char msg2[512];
	int i;// = 0;
	int yoff = 0;
	Color ColourWhite( 255, 255, 255, 255 );
	Color ColourRed( 255, 0, 0, 255 );
	Color ColourBlue( 0, 0, 255, 255 );
	switch (cl_flagstatus.GetInt())
	{
		case 0:
			for( i = 0; i < MAX_FLAGS; i++ )
				m_pLabelFlag[i]->SetVisible(false);

			break;
		case 1:
			for( i = 0; i < MAX_FLAGS; i++ )
				m_pLabelFlag[i]->SetVisible(false);

			for( i = 0; i < g_Flags.Count() && i < 12; i++ )
			{
				if( !g_Flags[i] )
					break;

				int iTimeToCap = g_Flags[i]->m_flNextCapture - gpGlobals->curtime;

				switch( g_Flags[i]->m_iForTeam )
				{
					case 0:
						if( iTimeToCap > 0 )
						{
							Q_snprintf( msg2, 512, "%s - Requires %i to Capture - Time %i", g_Flags[i]->m_sFlagName, g_Flags[i]->m_iCapturePlayers, iTimeToCap );
						}
						else
						{
							Q_snprintf( msg2, 512, "%s - Requires %i to Capture", g_Flags[i]->m_sFlagName, g_Flags[i]->m_iCapturePlayers );
						}
						break;
					case 1:
						if( iTimeToCap > 0 )
						{
							Q_snprintf( msg2, 512, "%s - American Only Traget - Requires %i to Capture - Time %i", g_Flags[i]->m_sFlagName, g_Flags[i]->m_iCapturePlayers, iTimeToCap );
						}
						else
						{
							Q_snprintf( msg2, 512, "%s - American Only Traget - Requires %i to Capture", g_Flags[i]->m_sFlagName, g_Flags[i]->m_iCapturePlayers);
						}
						break;
					case 2:
						if( iTimeToCap > 0 )
						{
							Q_snprintf( msg2, 512, "%s - British Only Traget - Requires %i to Capture - Time %i", g_Flags[i]->m_sFlagName, g_Flags[i]->m_iCapturePlayers, iTimeToCap );
						}
						else
						{
							Q_snprintf( msg2, 512, "%s - British Only Traget - Requires %i to Capture", g_Flags[i]->m_sFlagName, g_Flags[i]->m_iCapturePlayers);
						}
						break;
				}

				switch( g_Flags[i]->GetTeamNumber() )
				{
					case TEAM_AMERICANS:
						m_pLabelFlag[i]->SetFgColor(ColourBlue);
						break;
					case TEAM_BRITISH:
						m_pLabelFlag[i]->SetFgColor(ColourRed);
						break;
					default:
					case TEAM_UNASSIGNED:
						m_pLabelFlag[i]->SetFgColor(ColourWhite);
						break;
				}

				m_pLabelFlag[i]->SetText( msg2 );
				m_pLabelFlag[i]->SizeToContents();
				m_pLabelFlag[i]->SetVisible( true );
				m_pLabelFlag[i]->SetPos(0,yoff);

				yoff += 20;
			}
			break;
		case 2:
			i = 0;
			while (i < MAX_FLAGS)
			{
				m_pLabelFlag[i]->SetVisible(false);
				i++;
			}
			//i = 0;
			int xoff = 0;
			//while (i < iNumFlags)
			for( i = 0; i < g_Flags.Count(); i++ )
			{
				switch( g_Flags[i]->m_iLastTeam )
				{
				case TEAM_UNASSIGNED:
						m_IconFlag[i]->DrawSelf(xoff,0,ColourWhite);
						Q_snprintf( msg2, 512, "%i", g_Flags[i]->m_iCapturePlayers);
						m_pLabelFlag[i]->SetText( msg2 );
						m_pLabelFlag[i]->SizeToContents();
						m_pLabelFlag[i]->SetVisible( true );
						m_pLabelFlag[i]->SetPos((xoff + 16),32);
						m_pLabelFlag[i]->SetFgColor(ColourWhite);
						m_pLabelFlag[i]->SetVisible(true);
						break;
					case TEAM_AMERICANS:
						m_IconFlag[i]->DrawSelf(xoff,0,ColourBlue);
						Q_snprintf( msg2, 512, "%i", g_Flags[i]->m_iCapturePlayers);
						m_pLabelFlag[i]->SetText( msg2 );
						m_pLabelFlag[i]->SizeToContents();
						m_pLabelFlag[i]->SetVisible( true );
						m_pLabelFlag[i]->SetPos((xoff + 16),32);
						m_pLabelFlag[i]->SetFgColor(ColourBlue);
						m_pLabelFlag[i]->SetVisible(true);
						break;
					case TEAM_BRITISH:
						m_IconFlag[i]->DrawSelf(xoff,0,ColourRed);
						Q_snprintf( msg2, 512, "%i", g_Flags[i]->m_iCapturePlayers);
						m_pLabelFlag[i]->SetText( msg2 );
						m_pLabelFlag[i]->SizeToContents();
						m_pLabelFlag[i]->SetVisible( true );
						m_pLabelFlag[i]->SetPos((xoff + 16),32);
						m_pLabelFlag[i]->SetFgColor(ColourRed);
						m_pLabelFlag[i]->SetVisible(true);
						break;
				}
				int iTimeToCap = g_Flags[i]->m_flNextCapture - gpGlobals->curtime;
				if( iTimeToCap > 0 )
				{
					int coverw = (32 * (iTimeToCap / (int)g_Flags[i]->m_flCaptureTime));
					switch( g_Flags[i]->m_iLastTeam )
					{
						case TEAM_UNASSIGNED:
							m_IconCover[i]->DrawSelfCropped(xoff, 0, xoff, 0, 32 - coverw, 32, ColourWhite);
							break;
						case TEAM_BRITISH:
							m_IconCover[i]->DrawSelfCropped(xoff, 0, xoff, 0, 32 - coverw, 32, ColourRed);
							break;
						case TEAM_AMERICANS:
							m_IconCover[i]->DrawSelfCropped(xoff, 0, xoff, 0, 32 - coverw, 32, ColourBlue);
							break;
					}
				}

				xoff += 40;
			}
			break;
	}
	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*void CHudFlags::MsgFunc_flagstatus( bf_read &msg )
{
	int i = 0;
	iNumFlags = 0;
	iNumFlags = msg.ReadByte();
	if (iNumFlags > MAX_FLAGS)
	{
		iNumFlags = MAX_FLAGS;
	}
	while (i < iNumFlags)
	{
		msg.ReadString(STRING( g_Flags[i]->m_sFlagName ), sizeof(STRING( g_Flags[i]->m_sFlagName )));
		myflags[i].m_iOwner = msg.ReadByte();
		myflags[i].m_iTimeToCap = msg.ReadShort();
		g_Flags[i]->m_iCapturePlayers = msg.ReadByte();
		myflags[i].m_iForTeam = msg.ReadByte();
		myflags[i].m_iTimeNeeded = msg.ReadShort();
		myflags[i].m_iCappers = msg.ReadByte();
		i++;
	}
}*/