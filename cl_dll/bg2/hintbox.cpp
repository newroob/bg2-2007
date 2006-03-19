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

#include "hintbox.h"

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
	"This is the first hint, hope it helps",
	"This is the second hint, hope it really helps",
	"This is the third hint, hope it really helps"
};


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
	vgui::HFont m_hSmallFont, m_hLargeFont;
	vgui::Label	*m_Text;
	float hint_showtime;
	bool hidden;
	vgui::HScheme scheme; 	 	//The Scheme object to hold our scheme info.

protected:
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
};

extern char *pVHints[];

using namespace vgui;

DECLARE_HUDELEMENT( CHintbox );
CHintbox::CHintbox( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "HudFlags" )
{
	DevMsg (2, "CHintbox::CHintbox - constructor sent %s\n", pElementName);
	scheme = vgui::scheme()->LoadSchemeFromFile("resource/ClientScheme.res", "ClientScheme");
	SetScheme(scheme);	// Here we load up our scheme and set this element to use it. Using a different scheme than ClientScheme doesn't work right off the bat anyways, so... :)
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );	// Our parent is the screen itself.
	SetHiddenBits( HIDEHUD_ALL );	
	SetBgColor(Color( 0,0,0,100 ));
	SetPaintBackgroundEnabled( true );
	SetPaintBackgroundType (2); // Rounded corner box
	//hidden = true;
	m_Text = new vgui::Label( this, "Hintbox", "");
	//m_Text->SetWrap(true);
	m_Text->SetPaintBackgroundEnabled( false );
	m_Text->SetPaintBorderEnabled( false );
	m_Text->SetSize(280, 100);
	m_Text->SetContentAlignment( vgui::Label::a_west );
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
	m_Text->SetText("");
	hint_showtime = 0;
	UseHint(1, 1 ,1);
}

void CHintbox::VidInit( void )
{
	DevMsg (2, "CHintbox::VidInit\n" );
}

void CHintbox::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	DevMsg (2, "CHintbox::ApplySchemeSettings\n" );

	m_hSmallFont = pScheme->GetFont( "HudHintTextSmall", true );
	m_hLargeFont = pScheme->GetFont( "HudHintTextLarge", true );

	BaseClass::ApplySchemeSettings( pScheme );
}

void CHintbox::SetHint( char *text, int displaytime, int displaymode )
{
	m_Text->SetText(text);
	hint_showtime = gpGlobals->curtime + displaytime;
	hidden = false;
	// TODO displaymode usage
}

void CHintbox::UseHint( int textpreset, int displaytime, int displaymode )
{
	m_hint = textpreset;
	if (m_hint > NUM_HINTS)
		return;
	if(pVHints[m_hint] != NULL)
		m_Text->SetText(pVHints[m_hint]);
	hint_showtime = gpGlobals->curtime + displaytime;
	hidden = false;
	// TODO displaymode usage
}

void CHintbox::MsgFunc_Hintbox( bf_read &msg )
{
	int hint;

	hint = msg.ReadByte();
	DevMsg (2, "CHintbox::MsgFunc_Hintbox - got message hint: %d\n", hint );
	m_Text->SetText(pVHints[hint]);
	hint_showtime = gpGlobals->curtime + 5;
	hidden = false;
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

	wchar_t unicode[256]; // scratch space for text to print

	// --- Set up default font and get character height for line spacing

	SetPos( ScreenWidth() - 300, 400);
	SetSize( 300, 120 );
	SetPaintBackgroundType (2); // Rounded corner box
	SetPaintBackgroundEnabled( true );

  	vgui::surface()->DrawSetTextColor( 255, 255, 255, 220 ); 
	swprintf(unicode, L"test");

	Color ColourWhite( 255, 255, 255, 255 );
	m_Text->SetFgColor( ColourWhite );
	
	int temp_x, temp_y;
	m_Text->GetParent()->GetPos(temp_x, temp_y);
	m_Text->SetPos(temp_x + 10, temp_y + 10);
	

	//vgui::surface()->DrawPrintText( unicode, wcslen(unicode) ); // print text
	
	//BaseClass::Paint();
}
