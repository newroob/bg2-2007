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
		Jason "Draco" Houston		iamlorddraco@gmail.com

	You may also contact us via the Battle Grounds website and/or forum at:
		www.bgmod.com
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
#include "c_hl2mp_player.h"
#include "hl2mp_gamerules.h"

// hintbox header
#include "hintbox.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//==============================================
// CHudBG2
// Displays flag status
//==============================================
class CHudBG2 : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudBG2, vgui::Panel );
public:
	CHudBG2( const char *pElementName );
	void Init( void );
	void VidInit( void );
	virtual bool ShouldDraw( void );
	virtual void Paint( void );
	virtual void ApplySchemeSettings( vgui::IScheme *scheme );
	virtual void OnThink();

private:

	CHudTexture		* m_Base; 
	CHudTexture		* m_Straps; 
	CHudTexture		* m_Stamina;
	CHudTexture		* m_Health;

	vgui::Label * m_pLabelBScore; 
	vgui::Label * m_pLabelAScore; 
	vgui::Label * m_pLabelAmmo; 
	vgui::Label * m_pLabelWaveTime; 
	vgui::Label * m_pLabelBGVersion; 
};

using namespace vgui;

DECLARE_HUDELEMENT( CHudBG2 );

//==============================================
// CHudBG2's CHudFlags
// Constructor
//==============================================
CHudBG2::CHudBG2( const char *pElementName ) :
	CHudElement( pElementName ), BaseClass( NULL, "HudBG2" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_ALL );//HIDEHUD_MISCSTATUS );

	m_Base = NULL; 
	m_Stamina = NULL;
	m_Health = NULL;

	Color ColourWhite( 255, 255, 255, 255 );

	m_pLabelBScore = new vgui::Label( this, "RoundState_warmup", "test label");
	m_pLabelBScore->SetPaintBackgroundEnabled( false );
	m_pLabelBScore->SetPaintBorderEnabled( false );
	m_pLabelBScore->SizeToContents();
	m_pLabelBScore->SetContentAlignment( vgui::Label::a_west );
	m_pLabelBScore->SetFgColor( ColourWhite );

	m_pLabelAScore = new vgui::Label( this, "RoundState_warmup", "test label");
	m_pLabelAScore->SetPaintBackgroundEnabled( false );
	m_pLabelAScore->SetPaintBorderEnabled( false );
	m_pLabelAScore->SizeToContents();
	m_pLabelAScore->SetContentAlignment( vgui::Label::a_west );
	m_pLabelAScore->SetFgColor( ColourWhite );

	m_pLabelAmmo = new vgui::Label( this, "RoundState_warmup", "test label");
	m_pLabelAmmo->SetPaintBackgroundEnabled( false );
	m_pLabelAmmo->SetPaintBorderEnabled( false );
	m_pLabelAmmo->SizeToContents();
	m_pLabelAmmo->SetContentAlignment( vgui::Label::a_west );
	m_pLabelAmmo->SetFgColor( ColourWhite );

	m_pLabelWaveTime = new vgui::Label( this, "RoundState_warmup", "test label");
	m_pLabelWaveTime->SetPaintBackgroundEnabled( false );
	m_pLabelWaveTime->SetPaintBorderEnabled( false );
	m_pLabelWaveTime->SizeToContents();
	m_pLabelWaveTime->SetContentAlignment( vgui::Label::a_west );
	m_pLabelWaveTime->SetFgColor( ColourWhite );

	m_pLabelBGVersion = new vgui::Label( this, "RoundState_warmup", "test label");
	m_pLabelBGVersion->SetPaintBackgroundEnabled( false );
	m_pLabelBGVersion->SetPaintBorderEnabled( false );
	m_pLabelBGVersion->SizeToContents();
	m_pLabelBGVersion->SetContentAlignment( vgui::Label::a_west );
	m_pLabelBGVersion->SetFgColor( ColourWhite );
	m_pLabelBGVersion->SetVisible(false);
}

//==============================================
// CHudBG2's ApplySchemeSettings
// applies the schemes
//==============================================
void CHudBG2::ApplySchemeSettings( IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );
	SetPaintBackgroundEnabled( false );

}

//==============================================
// CHudBG2's Init
// Inits any vars needed
//==============================================
void CHudBG2::Init( void )
{	
}

//==============================================
// CHudBG2's VidInit
// Inits any textures needed
//==============================================
void CHudBG2::VidInit( void )
{
	m_Base = gHUD.GetIcon( "hud_base" );
	m_Stamina = gHUD.GetIcon( "hud_stamina" );
	m_Health = gHUD.GetIcon( "hud_health" );
	m_Straps = gHUD.GetIcon( "hud_straps" );
}

//==============================================
// CHudBG2's ShouldDraw
// whether the panel should be drawing
//==============================================
bool CHudBG2::ShouldDraw( void )
{
	return CHudElement::ShouldDraw();
}

//==============================================
// CHudBG2's Paint
// errr... paints the panel
//==============================================
void CHudBG2::Paint()
{
	Color ColourRed( 255, 0, 0, 255 );
	Color ColourWhite( 255, 255, 255, 255 );
	C_BaseCombatWeapon *wpn = GetActiveWeapon();
	if (!wpn)
	{
		return;
	}
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if (!player)
	{
		return;
	}
	C_HL2MP_Player *pHL2Player = (C_HL2MP_Player*)C_HL2MP_Player::GetLocalPlayer();
	if (!pHL2Player)
	{
		return;
	}
	char msg2[512];

	int w,h;
	int ystart = GetTall() - m_Base->Height();

	m_Base->DrawSelf(0,ystart,ColourWhite);
	int healthheight = player->GetHealth()* 1.05;
	int healthy = 105 - healthheight;
	int stamheight = pHL2Player->m_iStamina * 1.05;
	int stamy = 105 - stamheight;
	m_Stamina->DrawSelfCropped(50,ystart + 2 + stamy,0,/*m_Stamina->Height()*//*20*/ stamy, 25, stamheight/*20*/,ColourWhite);
	m_Health->DrawSelfCropped(15,ystart + 2 + healthy,0,/*m_Health->Height()*//*40*/ healthy, 25, healthheight /*10*/,ColourWhite);
	m_Straps->DrawSelf(10,ystart + 6,ColourWhite);
	
	C_Team *pAmer = GetGlobalTeam(TEAM_AMERICANS);
	C_Team *pBrit = GetGlobalTeam(TEAM_BRITISH);
	Q_snprintf( msg2, 512, "%i ", pBrit->Get_Score());
	m_pLabelBScore->SetText(msg2);
	m_pLabelBScore->SizeToContents();
	m_pLabelWaveTime->GetSize( w, h );
	m_pLabelBScore->SetPos(90,ystart + 40 - h/2);
	m_pLabelBScore->SetFgColor( ColourWhite );
	m_pLabelBScore->SetVisible(ShouldDraw());
	
	Q_snprintf( msg2, 512, "%i ", pAmer->Get_Score());
	m_pLabelAScore->SetText(msg2);
	m_pLabelAScore->SizeToContents();
	m_pLabelWaveTime->GetSize( w, h );
	m_pLabelAScore->SetPos(135,ystart + 40 - h/2);
	m_pLabelAScore->SetFgColor( ColourWhite );
	m_pLabelAScore->SetVisible(ShouldDraw());

	int iAmmoCount = player->GetAmmoCount(wpn->GetPrimaryAmmoType()) + wpn->Clip1();
	if( iAmmoCount >= 0 )
	{
		Q_snprintf( msg2, 512, "%i ", iAmmoCount);
		m_pLabelAmmo->SetText(msg2);
		if (wpn->Clip1() != 1)
		{
			m_pLabelAmmo->SetFgColor(ColourRed);
		}
		else
		{
			m_pLabelAmmo->SetFgColor(ColourWhite);
		}
		m_pLabelAmmo->SizeToContents();
		m_pLabelAmmo->GetSize( w, h );
		m_pLabelAmmo->SetPos(135,ystart + 100 - h/2);
		m_pLabelAmmo->SetVisible(ShouldDraw());
	}
	else
		m_pLabelAmmo->SetVisible( false );

	Q_snprintf( msg2, 512, "%i:%i ", (HL2MPRules()->m_iWaveTime / 60), (HL2MPRules()->m_iWaveTime % 60));
	m_pLabelWaveTime->SetText(msg2);
	m_pLabelWaveTime->SizeToContents();
	m_pLabelWaveTime->GetSize( w, h );
	m_pLabelWaveTime->SetPos(120,ystart + 67 - h/2);
	m_pLabelWaveTime->SetFgColor( ColourWhite );
	m_pLabelWaveTime->SetVisible(ShouldDraw());

	// BP - BG version display at lower right bottom of screen
	// FIXME, something is bugged here. text disappears after player spawns
	Q_snprintf( msg2, 512, "%s ", HL2MPRules()->GetGameDescription());
	m_pLabelBGVersion->SetText(msg2);
	m_pLabelBGVersion->SizeToContents();
	m_pLabelBGVersion->GetSize( w, h );
	m_pLabelBGVersion->SetPos(5, 60);	
	m_pLabelBGVersion->SetFgColor( ColourWhite );
	m_pLabelBGVersion->SetVisible(ShouldDraw());
}

//==============================================
// CHudBG2's OnThink
// every think check if we're hidden, if so take away the text
//==============================================
void CHudBG2::OnThink()
{
	// display hintbox if stamina drops below 20%
	C_HL2MP_Player *pHL2Player = (C_HL2MP_Player*)C_HL2MP_Player::GetLocalPlayer();
	if (pHL2Player->m_iStamina < 20)
 		(GET_HUDELEMENT( CHintbox ))->UseHint(HINT_STAMINA, 6, DISPLAY_ONCE);
	
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if (player)
	{
		if (player->GetHealth() <= 0)
		{
			m_pLabelAScore->SetVisible(false);
			m_pLabelBScore->SetVisible(false);
			m_pLabelWaveTime->SetVisible(false);
			m_pLabelAmmo->SetVisible(false);
			m_pLabelBGVersion->SetVisible(false);
		}
	}
}