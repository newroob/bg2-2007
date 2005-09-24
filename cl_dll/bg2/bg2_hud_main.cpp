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

	CPanelAnimationVarAliasType( float, m_flLineHeight, "LineHeight", "15", "proportional_float" );

	CPanelAnimationVar( float, m_flMaxDeathNotices, "MaxDeathNotices", "4" );

	CPanelAnimationVar( bool, m_bRightJustify, "RightJustify", "1" );

	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "HudNumbersTimer" ); 

	CHudTexture		* m_Base; 
	CHudTexture		* m_Stamina;
	CHudTexture		* m_Health;

	vgui::Label * m_pLabelBScore; 
	vgui::Label * m_pLabelAScore; 
	vgui::Label * m_pLabelAmmo; 
	vgui::Label * m_pLabelWaveTime; 
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

	SetHiddenBits( HIDEHUD_MISCSTATUS );

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

	
	int ystart = GetTall() - m_Base->Height();

	m_Base->DrawSelf(0,ystart,ColourWhite);
	int healthheight = player->GetHealth()* 0.94;
	int healthy = 94 - healthheight;
	int stamheight = pHL2Player->m_iStamina * 0.94;
	int stamy = 94 - stamheight;
	m_Stamina->DrawSelfCropped(35,ystart + 24 + stamy,0,0,23,stamheight,ColourWhite);
	m_Health->DrawSelfCropped(5,ystart + 24 + healthy,0,0,23,healthheight,ColourWhite);
	
	C_Team *pAmer = GetGlobalTeam(TEAM_AMERICANS);
	C_Team *pBrit = GetGlobalTeam(TEAM_BRITISH);
	Q_snprintf( msg2, 512, "%i", pBrit->Get_Score());
	m_pLabelBScore->SetText(msg2);
	m_pLabelBScore->SetPos(95,ystart + 65);
	m_pLabelBScore->SizeToContents();
	m_pLabelBScore->SetVisible(ShouldDraw());
	
	Q_snprintf( msg2, 512, "%i", pAmer->Get_Score());
	m_pLabelAScore->SetText(msg2);
	m_pLabelAScore->SetPos(128,ystart + 65);
	m_pLabelAScore->SizeToContents();
	m_pLabelAScore->SetVisible(ShouldDraw());

	int iAmmoCount = player->GetAmmoCount(wpn->GetPrimaryAmmoType()) + wpn->Clip1();
	Q_snprintf( msg2, 512, "%i", iAmmoCount);
	m_pLabelAmmo->SetText(msg2);
	if (wpn->Clip1() != 1)
	{
		m_pLabelAmmo->SetFgColor(ColourRed);
	}
	else
	{
		m_pLabelAmmo->SetFgColor(ColourWhite);
	}
	m_pLabelAmmo->SetPos(210,ystart + 105);
	m_pLabelAmmo->SizeToContents();
	m_pLabelAmmo->SetVisible(ShouldDraw());

	Q_snprintf( msg2, 512, "%i:%i", (HL2MPRules()->m_iWaveTime / 60), (HL2MPRules()->m_iWaveTime % 60));
	m_pLabelWaveTime->SetText(msg2);
	m_pLabelWaveTime->SetPos(120,ystart + 85);
	m_pLabelWaveTime->SizeToContents();
	m_pLabelWaveTime->SetVisible(ShouldDraw());
}

//==============================================
// CHudBG2's OnThink
// every think check if we're hidden, if so take away the text
//==============================================
void CHudBG2::OnThink()
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if (player)
	{
		if (player->GetHealth() <= 0)
		{
			m_pLabelAScore->SetVisible(false);
			m_pLabelBScore->SetVisible(false);
			m_pLabelWaveTime->SetVisible(false);
			m_pLabelAmmo->SetVisible(false);
		}
	}
}