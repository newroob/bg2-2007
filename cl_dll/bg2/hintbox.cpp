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

// our hint def file
#include "hintdefs.h"

//memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern IGameUIFuncs *gameuifuncs; // for key binding details

class CHint
{
public:
	CHint(char *input);
	~CHint();
	char* GetText() { return m_text; };
	bool Shown() { return m_shown; };
	void SetShown(bool shown) { m_shown = shown; };
private:
	bool m_shown;
	char m_text[512];
};

CHint::CHint(char *input)
{
	strcpy(m_text, input);
	m_shown = false;
}

CHint::~CHint()
{
	delete m_text;
	m_shown = NULL;
}

// not implemented yet
static ConVar cl_hintbox( "cl_hintbox", "1", 0, "0 - Off, 1 - game relevant hints, 2 -  with newbie notices" );

class CHintbox : public CHudElement , public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHintbox, vgui::Panel );
public:
	CHintbox( const char *pElementName );
	virtual void Init( void );	
	virtual void VidInit( void );	
	void Reset();
	virtual void OnThink(void);
	void MsgFunc_Hintbox( bf_read &msg );
	void SetHint( char* text, int displaytime, int displaymode ); // for custom messages
	void UseHint( int textpreset, int displaytime, int displaymode ); // for predefined messages

private:
	vgui::HFont m_hHintFont;
	CHint *m_hint[NUM_HINTS];
	Vector m_textpos;
	float m_hintshowtime;
	bool m_hidden;
	vgui::HScheme scheme;
	vgui::Label * m_label; 

protected:
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
};

extern char *pVHints[];

using namespace vgui;

DECLARE_HUDELEMENT( CHintbox );
DECLARE_HUD_MESSAGE( CHintbox, Hintbox );

CHintbox::CHintbox( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "Hintbox" )
{
	DevMsg (2, "CHintbox::CHintbox - constructor sent %s\n", pElementName);
	
	// Here we load up our scheme and set this element to use it. Using a different scheme than ClientScheme doesn't work right off the bat anyways, so... :)
	scheme = vgui::scheme()->LoadSchemeFromFile("resource/ClientScheme.res", "ClientScheme");
	SetScheme(scheme);	
	
	// Our parent is the screen itself.
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );
	
	for (int i=0; i < NUM_HINTS; i++)
	{
		m_hint[i] = new CHint(pVHints[i]);
	}
	
	SetHiddenBits( HIDEHUD_ALL );	
	SetBgColor(Color( 0,0,0,100 ));
	SetPaintBackgroundEnabled( true );
	SetPos( ScreenWidth() - 300, 300);
	SetSize( 300, 120 );
	SetPaintBackgroundType (2); // Rounded corner box
	SetAlpha(255);
	
	m_hidden = true;
	m_textpos.x = 5;
	m_textpos.y = 5;
	
	m_label = new vgui::Label( this, "HintLabel", "hint label" );
	m_label->SetPaintBackgroundEnabled( false );
	m_label->SetPaintBorderEnabled( false );
	m_label->SetContentAlignment( vgui::Label::a_west );
	m_label->SetVisible(false);
	m_label->SetPos(m_textpos.x, m_textpos.y);
} 


void CHintbox::Init( void )
{
	//HOOK_HUD_MESSAGE( CHintbox, Hintbox ); 
	DevMsg (2, "CHintbox::Init\n" );
	Reset();
}

void CHintbox::Reset( void )
{
	m_hintshowtime = 0;
	m_hidden = true;

	m_label->SetText("");
	m_label->SetVisible(false);
}

void CHintbox::VidInit( void )
{
	DevMsg (2, "CHintbox::VidInit\n" );
}

void CHintbox::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	DevMsg (2, "CHintbox::ApplySchemeSettings\n" );
	BaseClass::ApplySchemeSettings( pScheme );
	
	m_hHintFont = pScheme->GetFont("HudHintTextSmall", true);
	m_label->SetFont(m_hHintFont);
}


void CHintbox::SetHint( char *text, int displaytime, int displaymode )
{
	m_label->SetText(text);

	m_hintshowtime = gpGlobals->curtime + displaytime;
	m_hidden = false;
	SetAlpha(255);

	m_label->SetVisible(true);
	m_label->SizeToContents();

	// TODO displaymode usage
}


void CHintbox::UseHint( int textpreset, int displaytime, int displaymode )
{
	if (textpreset >= NUM_HINTS)
		return;
		
	//check wether we are already displaying a different hint
	if (!m_hidden)
		return;

	if (m_hint == NULL)
		return;

	//we already showed this hint once
	if (m_hint[textpreset]->Shown())
		return;

	Color ColourWhite( 255, 255, 255, 230 );
	m_label->SetFgColor( ColourWhite );

	m_hintshowtime = gpGlobals->curtime + displaytime;
	m_hidden = false;
	SetAlpha(255);

	m_hint[textpreset]->SetShown(true);

	wchar_t	printtext[512];
	vgui::localize()->ConvertANSIToUnicode(m_hint[textpreset]->GetText(), printtext, sizeof(printtext));
	m_label->SetText(printtext);
	m_label->SetVisible(true);
	m_label->SizeToContents();

	// TODO displaymode usage
}


void CHintbox::MsgFunc_Hintbox( bf_read &msg )
{
	int hint;
	hint = msg.ReadByte();
	DevMsg (2, "CHintbox::MsgFunc_Hintbox - got message hint: %d\n", hint );
	
	UseHint( hint, 5, 1 );
}
void CHintbox::OnThink(void)
{
	if (m_hintshowtime < gpGlobals->curtime)
	{
		SetVisible(false);
		m_hidden = true;
	}
	else
		SetVisible(true);

	//fade out
	if (m_hintshowtime - 1 < gpGlobals->curtime && !m_hidden)
	{
		float alpha = (m_hintshowtime - gpGlobals->curtime) * 255;
		if (alpha < 1)
			alpha = 0;
		SetAlpha((int)alpha);
	}
}

static void hintbox_f( void )
{
	if( engine->Cmd_Argc() == 2 )
	{
		int hint = atoi( engine->Cmd_Argv( 1 ) );
		CHintbox *hintbox = (CHintbox *)GET_HUDELEMENT( CHintbox );
		int displaytime = 5;
		int displaymode = 1;
		hintbox->UseHint(hint, displaytime, displaymode);
	}
}
static ConCommand hintbox( "hintbox", hintbox_f );