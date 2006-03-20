/*
	The Battle Grounds 2 - A Source modification
	Copyright (C) 206, The Battle Grounds 2 Team and Contributors

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
		Jason "Draco" Houston		iamlorddraco@gmail.com

	You may also contact us via the Battle Grounds website and/or forum at:
		www.bgmod.com
*/
#include "cbase.h"
#include <cdll_client_int.h>
#include <globalvars_base.h>
#include <cdll_util.h>
#include <KeyValues.h>
#include "hud_macros.h"
#include "hud.h"
#include "hudelement.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IPanel.h>
#include <vgui_controls/ImageList.h>
#include <vgui_controls/MenuItem.h>

#include <stdio.h> // _snprintf define

#include <cl_dll/iviewport.h>
#include "commandmenu.h"
#include "hltvcamera.h"

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Menu.h>
#include "c_hl2mp_player.h"
#include "IGameUIFuncs.h" // for key bindings
#include <imapoverview.h>
#include <shareddefs.h>
#include <igameresources.h>
#include "clientmode_hl2mpnormal.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern IGameUIFuncs *gameuifuncs; // for key binding details


#define NUM_HINTS	3

char *pVHints[NUM_HINTS] =
{
	"This is the first hint, hope it helps\n",
	"This is the second hint, hope it really helps\n",
	"This is the third hint, hope it really helps\n"
};

static ConVar cl_hintbox( "cl_hintbox", "1", 0, "0 - Off, 1 - game relevant hints, 2 -  with newbie notices" );

class CHintbox : public CHudElement , public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHintbox, vgui::Panel );
public:
	CHintbox( const char *pElementName );
	void Init( void );	
	void VidInit( void );	
	void Reset();
	void Paint( void );	
	void OnThink(void);
	void MsgFunc_Hintbox( bf_read &msg );
	void SetHint( char* text, int displaytime, int displaymode ); // for custom messages
	void UseHint( int textpreset, int displaytime, int displaymode ); // for predefined messages

private:
	int	m_hint;
	vgui::HFont m_hHintFont;
	wchar_t m_Text[512];
	Vector m_textpos;
	float m_hintshowtime;
	bool m_hidden;
	vgui::HScheme scheme;

protected:
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
};

extern char *pVHints[];

using namespace vgui;

DECLARE_HUDELEMENT( CHintbox );
CHintbox::CHintbox( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "Hintbox" )
{
	DevMsg (2, "CHintbox::CHintbox - constructor sent %s\n", pElementName);
	
	// Here we load up our scheme and set this element to use it. Using a different scheme than ClientScheme doesn't work right off the bat anyways, so... :)
	scheme = vgui::scheme()->LoadSchemeFromFile("resource/ClientScheme.res", "ClientScheme");
	SetScheme(scheme);	
	
	// Our parent is the screen itself.
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );	
	
	SetHiddenBits( HIDEHUD_ALL );	
	SetBgColor(Color( 0,0,0,100 ));
	SetPaintBackgroundEnabled( true );
	SetPaintBackgroundType (2); // Rounded corner box
	SetPos( ScreenWidth() - 300, 300);
	SetSize( 300, 120 );
	SetPaintBackgroundType (2); // Rounded corner box
	
	//hidden = true;
	m_hidden = false;
	m_textpos.x = 10;
	m_textpos.y = 10;
	
	int parent_x, parent_y;
	this->GetPos(parent_x, parent_y);
	m_textpos.x += parent_x;
	m_textpos.y += parent_y;
} 


void CHintbox::Init( void )
{
	//HOOK_HUD_MESSAGE( CHintbox, Hintbox );
	DevMsg (2, "CHintbox::Init\n" );
	Reset();
}

void CHintbox::Reset( void )
{
	DevMsg (2, "CHintbox::Reset (clearing out list)\n" );
	swprintf(m_Text, L"");
	m_hintshowtime = 0;
	UseHint(1, 1 ,1);
}

void CHintbox::VidInit( void )
{
	DevMsg (2, "CHintbox::VidInit\n" );
}

void CHintbox::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	DevMsg (2, "CHintbox::ApplySchemeSettings\n" );

	m_hHintFont = pScheme->GetFont("HudHintTextSmall");

	BaseClass::ApplySchemeSettings( pScheme );
}

void CHintbox::SetHint( char *text, int displaytime, int displaymode )
{
	swprintf(m_Text, L"%s", text);
	m_hintshowtime = gpGlobals->curtime + displaytime;
	m_hidden = false;

	// TODO displaymode usage
}

void CHintbox::UseHint( int textpreset, int displaytime, int displaymode )
{
	m_hint = textpreset;
	if (m_hint > NUM_HINTS)
		return;
	if(pVHints[m_hint] != NULL)
		swprintf(m_Text, L"%s", pVHints[m_hint]);

	m_hintshowtime = gpGlobals->curtime + displaytime;
	m_hidden = false;

	// TODO displaymode usage
}

void CHintbox::MsgFunc_Hintbox( bf_read &msg )
{
	int hint;

	hint = msg.ReadByte();
	DevMsg (2, "CHintbox::MsgFunc_Hintbox - got message hint: %d\n", hint );
	//strcpy(m_Text, pVHints[hint]); FIXME
	m_hintshowtime = gpGlobals->curtime + 5;
	m_hidden = false;
}

void CHintbox::OnThink(void)
{
	//if (hint_showtime < gpGlobals->curtime)
		//hidden = true;
	//SetNextClientThink(gpGlobals->curtime + 0.05);
}

void CHintbox::Paint( void )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return;

	m_textpos.x = 5;
	m_textpos.y = 5;
	vgui::surface()->DrawSetTextColor( 240, 240, 240, 220 );
	vgui::surface()->DrawSetTextFont( m_hHintFont );
	vgui::surface()->DrawSetTextPos(m_textpos.x, m_textpos.y);
	vgui::surface()->DrawPrintText( m_Text, wcslen(m_Text) );
}
