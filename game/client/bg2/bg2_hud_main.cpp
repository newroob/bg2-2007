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
		Tomas "Tjoppen" Härdin		mail, in reverse: se . gamedev @ tjoppen
		Jason "Draco" Houston		iamlorddraco@gmail.com

	You may also contact us via the Battle Grounds website and/or forum at:
		www.bgmod.com
*/
#include "cbase.h"
#include "hudelement.h"
#include "hl2mp_gamerules.h"
//BG2 - Tjoppen - #includes
#include "engine/IEngineSound.h"
#include "../server/bg2/weapon_bg2base.h"
#include "bg2_hud_main.h"
//

// hintbox header
//#include "hintbox.h" //For now at least. -HairyPotter


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

DECLARE_HUDELEMENT( CHudBG2 );
DECLARE_HUD_MESSAGE( CHudBG2, HitVerif );
DECLARE_HUD_MESSAGE( CHudBG2, WinMusic );
DECLARE_HUD_MESSAGE( CHudBG2, CaptureSounds ); //HairyPotter
DECLARE_HUD_MESSAGE( CHudBG2, VCommSounds );

CHudBG2 *CHudBG2::s_pInstance = NULL;
CHudBG2 *CHudBG2::GetInstance()
{
	return CHudBG2::s_pInstance;
}
//==============================================
// CHudBG2's CHudFlags
// Constructor
//==============================================
CHudBG2::CHudBG2( const char *pElementName ) :
	CHudElement( pElementName ), BaseClass( NULL, "HudBG2" ),
	m_IsVictimAccumulator(true), m_IsAttackerAccumulator(false)
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	s_pInstance = this; //BG2 - HairyPotter (Nifty: Why is this here? If you're inside of the object, the "this" keyword is the address to the current object, and it's always available. Outside of the object, you can get access to an object's address by properly getting the reference.)

	SetHiddenBits( HIDEHUD_ALL );//HIDEHUD_MISCSTATUS );

	m_Base = NULL; 
	m_AmerHealthBase = m_AmerHealth = m_AmerStamina = NULL;
	m_BritHealthBase = m_BritHealth = m_BritStamina = NULL;
	m_SwingometerRed = m_SwingometerBlue = m_PowderHorn = NULL;

	m_flExpireTime = 0;
	m_flLastSwing = 0.5f;

	Color ColourWhite( 255, 255, 255, 255 );

	m_pLabelBScore = new vgui::Label( this, "RoundState_warmup", "");
	m_pLabelBScore->SetPaintBackgroundEnabled( false );
	m_pLabelBScore->SetPaintBorderEnabled( false );
	m_pLabelBScore->SizeToContents();
	m_pLabelBScore->SetContentAlignment( vgui::Label::a_west );
	m_pLabelBScore->SetFgColor( ColourWhite );

	m_pLabelAScore = new vgui::Label( this, "RoundState_warmup", "");
	m_pLabelAScore->SetPaintBackgroundEnabled( false );
	m_pLabelAScore->SetPaintBorderEnabled( false );
	m_pLabelAScore->SizeToContents();
	m_pLabelAScore->SetContentAlignment( vgui::Label::a_west );
	m_pLabelAScore->SetFgColor( ColourWhite );

	m_pLabelBTickets = new vgui::Label( this, "RoundState_warmup", "");
	m_pLabelBTickets->SetPaintBackgroundEnabled( false );
	m_pLabelBTickets->SetPaintBorderEnabled( false );
	m_pLabelBTickets->SizeToContents();
	m_pLabelBTickets->SetContentAlignment( vgui::Label::a_west );
	m_pLabelBTickets->SetFgColor( ColourWhite );

	m_pLabelATickets = new vgui::Label( this, "RoundState_warmup", "");
	m_pLabelATickets->SetPaintBackgroundEnabled( false );
	m_pLabelATickets->SetPaintBorderEnabled( false );
	m_pLabelATickets->SizeToContents();
	m_pLabelATickets->SetContentAlignment( vgui::Label::a_west );
	m_pLabelATickets->SetFgColor( ColourWhite );

	m_pLabelCurrentRound= new vgui::Label( this, "RoundState_warmup", "");
	m_pLabelCurrentRound->SetPaintBackgroundEnabled( false );
	m_pLabelCurrentRound->SetPaintBorderEnabled( false );
	m_pLabelCurrentRound->SizeToContents();
	m_pLabelCurrentRound->SetContentAlignment( vgui::Label::a_west );
	m_pLabelCurrentRound->SetFgColor( ColourWhite );

	m_pLabelAmmo = new vgui::Label( this, "RoundState_warmup", "");
	m_pLabelAmmo->SetPaintBackgroundEnabled( false );
	m_pLabelAmmo->SetPaintBorderEnabled( false );
	m_pLabelAmmo->SizeToContents();
	m_pLabelAmmo->SetContentAlignment( vgui::Label::a_west );
	m_pLabelAmmo->SetFgColor( ColourWhite );

	m_pLabelWaveTime = new vgui::Label( this, "RoundState_warmup", "");
	m_pLabelWaveTime->SetPaintBackgroundEnabled( false );
	m_pLabelWaveTime->SetPaintBorderEnabled( false );
	m_pLabelWaveTime->SizeToContents();
	m_pLabelWaveTime->SetContentAlignment( vgui::Label::a_west );
	m_pLabelWaveTime->SetFgColor( ColourWhite );

	m_pLabelRoundTime = new vgui::Label( this, "RoundState_warmup", "");
	m_pLabelRoundTime->SetPaintBackgroundEnabled( false );
	m_pLabelRoundTime->SetPaintBorderEnabled( false );
	m_pLabelRoundTime->SizeToContents();
	m_pLabelRoundTime->SetContentAlignment( vgui::Label::a_west );
	m_pLabelRoundTime->SetFgColor( ColourWhite );

	m_pLabelDamageVerificator = new vgui::Label( pParent, "RoundState_warmup", "");
	m_pLabelDamageVerificator->SetPaintBackgroundEnabled( false );
	m_pLabelDamageVerificator->SetPaintBorderEnabled( false );
	m_pLabelDamageVerificator->SizeToContents();
	m_pLabelDamageVerificator->SetContentAlignment( vgui::Label::a_west );
	m_pLabelDamageVerificator->SetFgColor( ColourWhite );

	m_pLabelLMS = new vgui::Label( this, "RoundState_warmup", "");
	m_pLabelLMS->SetPaintBackgroundEnabled( false );
	m_pLabelLMS->SetPaintBorderEnabled( false );
	m_pLabelLMS->SizeToContents();
	m_pLabelLMS->SetContentAlignment( vgui::Label::a_west );
	m_pLabelLMS->SetFgColor( ColourWhite );

	CBaseEntity::PrecacheScriptSound( "Americans.win" );
	CBaseEntity::PrecacheScriptSound( "British.win" );

	//hide all
	HideShowAll(false);
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
	HOOK_HUD_MESSAGE( CHudBG2, HitVerif );
	HOOK_HUD_MESSAGE( CHudBG2, WinMusic );
	HOOK_HUD_MESSAGE( CHudBG2, CaptureSounds );
	HOOK_HUD_MESSAGE( CHudBG2, VCommSounds );
	//BG2 - Tjoppen - serverside blood, a place to hook as good as any
	extern void  __MsgFunc_ServerBlood( bf_read &msg );
	HOOK_MESSAGE( ServerBlood );
	//
}

//==============================================
// CHudBG2's VidInit
// Inits any textures needed
//==============================================
void CHudBG2::VidInit( void )
{
	m_Base = gHUD.GetIcon( "hud_base" );
	m_AmerHealthBase = gHUD.GetIcon("hud_amer_health_base");
	m_AmerHealth     = gHUD.GetIcon("hud_amer_health");
	m_AmerStamina    = gHUD.GetIcon("hud_amer_stamina");
	m_BritHealthBase = gHUD.GetIcon("hud_brit_health_base");
	m_BritHealth     = gHUD.GetIcon("hud_brit_health");
	m_BritStamina    = gHUD.GetIcon("hud_brit_stamina");
	m_SwingometerRed = gHUD.GetIcon("hud_swingometer_red");
	m_SwingometerBlue= gHUD.GetIcon("hud_swingometer_blue");
	m_PowderHorn     = gHUD.GetIcon("hud_powderhorn");
}

//==============================================
// CHudBG2's ShouldDraw
// whether the panel should be drawing
//==============================================
bool CHudBG2::ShouldDraw( void )
{
	bool temp = CHudElement::ShouldDraw();
	HideShowAll(temp);	//BG2 - Tjoppen - HACKHACK
	return temp;
}

//==============================================
// CHudBG2's Paint
// errr... paints the panel
//==============================================
static ConVar cl_draw_lms_indicator( "cl_draw_lms_indicator", "1", FCVAR_CLIENTDLL, "Draw last man standing indicator string?" );

void CHudBG2::Paint()
{
	if( !m_Base || !m_AmerHealthBase || !m_AmerHealth || !m_AmerStamina ||
		!m_BritHealthBase || !m_BritHealth || !m_BritStamina ||
		!m_SwingometerRed || !m_SwingometerBlue || !m_PowderHorn )
		return;

	//BG2 - Tjoppen - Always paint damage label, so it becomes visible while using iron sights
	//fade out the last second
	float alpha = (m_flExpireTime - gpGlobals->curtime) * 255.0f;
	if( alpha < 0.0f )
		alpha = 0.0f;
	else if( alpha > 255.0f )
		alpha = 255.0f;

	if( alpha > 0 )
	{
		m_pLabelDamageVerificator->SizeToContents();
		//center and put somewhat below crosshair
		m_pLabelDamageVerificator->SetPos((ScreenWidth()-m_pLabelDamageVerificator->GetWide())/2, (ScreenHeight()*5)/8);
		m_pLabelDamageVerificator->SetFgColor( Color( 255, 255, 255, (int)alpha ) );
		m_pLabelDamageVerificator->SetVisible( true );
	}
	else
		m_pLabelDamageVerificator->SetVisible( false );

	C_HL2MP_Player *pHL2Player = dynamic_cast<C_HL2MP_Player*>(C_HL2MP_Player::GetLocalPlayer());
	if (!pHL2Player || !pHL2Player->IsAlive())
	{
		//spectating
		int n = GetSpectatorTarget();
		if( n <= 0 )
		{
			HideShowAll(false);
			return;	//don't look at worldspawn..
		}

		pHL2Player = dynamic_cast<C_HL2MP_Player*>(UTIL_PlayerByIndex(n));

		if( !pHL2Player )
		{
			HideShowAll(false);
			return;
		}
	}

	C_BaseCombatWeapon *wpn = pHL2Player->GetActiveWeapon();
	if (!wpn)
	{
		HideShowAll(false);
		return;
	}
	// Don't draw hud if we're using Iron Sights. -HairyPotter
	if (wpn->m_bIsIronsighted)
	{
		HideShowAll(false);
		return;
	}
	//

	HideShowAll(true);

	Color ColourRed( 255, 0, 0, 255 );
	Color ColourWhite( 255, 255, 255, 255 );

	char msg2[512];

	int w,h;
	int ystart = GetTall() - m_Base->Height();

	m_Base->DrawSelf(0,ystart,ColourWhite);

	C_Team *pAmer = GetGlobalTeam(TEAM_AMERICANS);
	C_Team *pBrit = GetGlobalTeam(TEAM_BRITISH);

	//swingometer
	//displays based on tickets in ticket mode, score otherwise
	int swinga = 0, swingb = 0;

	if( HL2MPRules()->UsingTickets() )
	{
		swinga = pAmer ? pAmer->m_iTicketsLeft : 0;
		swingb = pBrit ? pBrit->m_iTicketsLeft : 0;
	}
	else
	{
		swinga = pAmer ? pAmer->Get_Score() : 0;
		swingb = pBrit ? pBrit->Get_Score() : 0;
	}

	float swing = m_flLastSwing;

	//guard against negatives. you never know..
	if( swinga >= 0 && swingb >= 0 )
	{
		//avoid division by zero - leave swing alone if we would
		int tot = swinga + swingb;

		if( tot > 0 )
			swing = swinga / (float)tot;
	}

	//move swing value towards m_flLastSwing
	float swingdiff = swing - m_flLastSwing;	//useful for setting alpha on bars
	float swingadjust = swingdiff;

	//cap swing rate so each percent takes 20 ms (meaning a full swing = 2 sec)
	float swingrate = 0.5f;

	if( swingadjust < -swingrate * gpGlobals->frametime ) swingadjust = -swingrate * gpGlobals->frametime;
	if( swingadjust >  swingrate * gpGlobals->frametime ) swingadjust =  swingrate * gpGlobals->frametime;

	swing = m_flLastSwing + swingadjust;

	int swingw = m_SwingometerBlue->Width();
	int swingx = (m_Base->Width() - swingw) / 2;
	int swingm = swingw * swing;
	int swingh = m_SwingometerBlue->Height();

	m_SwingometerBlue->DrawSelfCropped( swingx,          ystart, 0,      0, swingm,          swingh, ColourWhite );
	 m_SwingometerRed->DrawSelfCropped( swingx + swingm, ystart, swingm, 0, swingw - swingm, swingh, ColourWhite );

	m_flLastSwing = swing;

	CHudTexture *pHealthBase, *pHealth, *pStamina;

	if (pHL2Player->GetTeamNumber() == TEAM_AMERICANS)
	{
		pHealthBase  = m_AmerHealthBase;
		pHealth      = m_AmerHealth;
		pStamina     = m_AmerStamina;
	}
	else
	{
		pHealthBase  = m_BritHealthBase;
		pHealth      = m_BritHealth;
		pStamina     = m_BritStamina;
	}

	int healthheight = pHealth->Height()  * (100 - pHL2Player->GetHealth()) / 100;
	int stamheight   = pStamina->Height() * pHL2Player->m_iStamina  / 100;

	int stamy   = pStamina->Height() - stamheight;
	int offset = (pStamina->Height() - pHealth->Height()) / 2;
	int ystart2 = GetTall() - pStamina->Height();

	pStamina->DrawSelfCropped(m_Base->Width(), ystart2 + stamy,  0, stamy, 64, stamheight,   ColourWhite);
	pHealthBase->DrawSelf(    m_Base->Width(), ystart2 + offset,                               ColourWhite);
	pHealth->DrawSelfCropped( m_Base->Width(), ystart2 + offset, 0, 0,     64, healthheight, ColourWhite);

	if( HL2MPRules()->UsingTickets() )
	{
	Q_snprintf( msg2, 512, "%i ", pBrit ? pBrit->Get_Score() : 0);	//BG2 - Tjoppen - avoid NULL
	m_pLabelBScore->SetText(msg2);
	m_pLabelBScore->SizeToContents();
	m_pLabelWaveTime->GetSize( w, h );
	m_pLabelBScore->SetPos(m_Base->Width() - 149,ystart + 132);
	m_pLabelBScore->SetFgColor( ColourWhite );
	
	Q_snprintf( msg2, 512, "%i ", pAmer ? pAmer->Get_Score() : 0);	//BG2 - Tjoppen - avoid NULL
	m_pLabelAScore->SetText(msg2);
	m_pLabelAScore->SizeToContents();
	m_pLabelWaveTime->GetSize( w, h );
	m_pLabelAScore->SetPos(133,ystart + 132);
	m_pLabelAScore->SetFgColor( ColourWhite );

		extern ConVar mp_tickets_rounds;

		Q_snprintf( msg2, 512, "Round %i/%i ", HL2MPRules()->m_iCurrentRound, mp_tickets_rounds.GetInt() );
		m_pLabelCurrentRound->SetText(msg2);
		m_pLabelCurrentRound->SizeToContents();
		m_pLabelCurrentRound->SetPos(10,ystart - 15);
		m_pLabelCurrentRound->SetFgColor( ColourWhite );
	}

		Q_snprintf( msg2, 512, "%i ", swingb);
		m_pLabelBTickets->SetText(msg2);
		m_pLabelBTickets->SizeToContents();
		m_pLabelBTickets->SetPos(m_Base->Width() - 44,ystart + 103);
		m_pLabelBTickets->SetFgColor( ColourWhite );

		Q_snprintf( msg2, 512, "%i ", swinga);
		m_pLabelATickets->SetText(msg2);
		m_pLabelATickets->SizeToContents();
		m_pLabelATickets->SetPos(11,ystart + 103);
		m_pLabelATickets->SetFgColor( ColourWhite );

	int iAmmoCount = pHL2Player->GetAmmoCount(wpn->GetPrimaryAmmoType()) + wpn->Clip1();
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

		int hornx = m_Base->Width() + pStamina->Width();
		m_pLabelAmmo->SetPos( hornx + m_PowderHorn->Width(),GetTall() - m_PowderHorn->Height() / 2 - h/2);
		m_PowderHorn->DrawSelf( hornx, GetTall() - m_PowderHorn->Height(), ColourWhite );
	}
	else
		m_pLabelAmmo->SetVisible( false );

	int wavetime = ceilf(HL2MPRules()->m_fLastRespawnWave + mp_respawntime.GetFloat() - gpGlobals->curtime);
	if(	wavetime < 0 )
		wavetime = 0;

	Q_snprintf( msg2, 512, "%i:%02i ", wavetime / 60, wavetime % 60 );
	m_pLabelWaveTime->SetText(msg2);
	m_pLabelWaveTime->SizeToContents();
	m_pLabelWaveTime->GetSize( w, h );
	m_pLabelWaveTime->SetPos(20,ystart + 137);
	m_pLabelWaveTime->SetFgColor( ColourWhite );

	if( HL2MPRules()->UsingTickets() )
	{
		int roundtime = ceilf(HL2MPRules()->m_fLastRoundRestart + mp_tickets_roundtime.GetFloat() - gpGlobals->curtime);
		if(	roundtime < 0 )
			roundtime = 0;

		Q_snprintf( msg2, 512, "%i:%02i ", roundtime / 60, roundtime % 60 );
		m_pLabelRoundTime->SetText(msg2);
		m_pLabelRoundTime->SizeToContents();
		m_pLabelRoundTime->SetPos(220,ystart + 137);
		m_pLabelRoundTime->SetFgColor( ColourWhite );
	}

	// BP - BG version display at lower right bottom of screen
	/*Q_snprintf( msg2, 512, "%s ", HL2MPRules()->GetGameDescription());
	m_pLabelBGVersion->SetText(msg2);
	m_pLabelBGVersion->SizeToContents();
	m_pLabelBGVersion->GetSize( w, h );
	m_pLabelBGVersion->SetPos(5, 60);	
	m_pLabelBGVersion->SetFgColor( ColourWhite );*/

	m_pLabelLMS->SetText( g_pVGuiLocalize->Find("#LMS") );
	m_pLabelLMS->SizeToContents();
	m_pLabelLMS->GetSize( w, h );
	m_pLabelLMS->SetPos(5, ystart - 1.3*h);
	m_pLabelLMS->SetFgColor( ColourWhite );
}

//==============================================
// CHudBG2's OnThink
// every think check if we're hidden, if so take away the text
//==============================================
void CHudBG2::OnThink()
{
	// display hintbox if stamina drops below 20%
	//C_HL2MP_Player *pHL2Player = dynamic_cast<C_HL2MP_Player*>(C_HL2MP_Player::GetLocalPlayer()); //RE-enable these. -HairyPotter
	//if (pHL2Player && pHL2Player->m_iStamina < 20)
 	//	(GET_HUDELEMENT( CHintbox ))->UseHint(HINT_STAMINA, 6, DISPLAY_ONCE);
	
	//let paint sort it out
	/*C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if (player)
	{
		if (player->GetHealth() <= 0)
		{
			HideShowAll(false);
		}
	}*/
}

void CHudBG2::Reset( void )
{
	//mapchange, clear indicators. and stuff.
	m_flExpireTime = 0;
	HideShowAll( false );
}

void CHudBG2::HideShowAll( bool visible )
{
	m_pLabelAScore->SetVisible(visible && HL2MPRules()->UsingTickets());
	m_pLabelBScore->SetVisible(visible && HL2MPRules()->UsingTickets());
	m_pLabelBTickets->SetVisible(visible);
	m_pLabelATickets->SetVisible(visible);
	m_pLabelCurrentRound->SetVisible(visible && HL2MPRules()->UsingTickets());
	m_pLabelWaveTime->SetVisible(visible);
	m_pLabelRoundTime->SetVisible(visible && HL2MPRules()->UsingTickets());
	m_pLabelAmmo->SetVisible(visible);
	//m_pLabelBGVersion->SetVisible(false);	// BP: not used yet as its not subtle enough, m_pLabelBGVersion->SetVisible(ShouldDraw());
	m_pLabelDamageVerificator->SetVisible(m_flExpireTime > gpGlobals->curtime);	//always show damage indicator (unless expired)
	m_pLabelLMS->SetVisible( visible && mp_respawnstyle.GetInt() == 2 && cl_draw_lms_indicator.GetBool() );
}

//BG2 - Tjoppen - cl_hitverif & cl_winmusic && capturesounds & voice comm sounds //HairyPotter
ConVar	cl_hitverif( "cl_hitverif", "1", FCVAR_ARCHIVE, "Display hit verification? 1 For all. 2 For Melee only. 3 For Firearms only." );
ConVar	cl_winmusic( "cl_winmusic", "1", FCVAR_ARCHIVE, "Play win music?" );
ConVar	cl_capturesounds( "cl_capturesounds", "1", FCVAR_ARCHIVE, "Play flag capture sounds?" );
ConVar cl_vcommsounds("cl_vcommsounds", "1", FCVAR_ARCHIVE, "Allow voice comm sounds?" );
//


void CHudBG2::MsgFunc_HitVerif( bf_read &msg )
{
	int attacker, victim, hitgroup;
	int attackType;
	int damage;

	attacker	= msg.ReadByte();
	victim		= msg.ReadByte();
	hitgroup	= msg.ReadByte();
	damage		= msg.ReadShort();

	//attack type and hitgroup are packed into the same byte
	attackType = (hitgroup >> 4) & 0xF;
	hitgroup &= 0xF;

	C_HL2MP_Player *pAttacker = dynamic_cast<C_HL2MP_Player*>(UTIL_PlayerByIndex( attacker ));

	if( !pAttacker )
		return;

	C_BaseBG2Weapon *pWeapon = dynamic_cast<C_BaseBG2Weapon*>(pAttacker->GetActiveWeapon());

	if( !pWeapon || !C_BasePlayer::GetLocalPlayer() )
		return;

	//play melee hit sound if attacker's last attack was a melee attack
	//note: hit sound should always be played, regardless of what cl_hitverif is set to
	//in other words: don't mess up the order of these tests
	if( attackType == C_BaseBG2Weapon::ATTACKTYPE_STAB || attackType == C_BaseBG2Weapon::ATTACKTYPE_SLASH )
	{
		//we need play the hit sound like this since WeaponSound() only plays the local player's weapon's sound, not those of the other players
		const char *sound = pWeapon->GetShootSound( MELEE_HIT );

		if( sound && sound[0] )
		{
			CLocalPlayerFilter filter;
			C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, sound );
		}

		//cl_hitverif 3 -> don't display melee hits
		if( cl_hitverif.GetInt() == 3 )
			return;
	}

	//cl_hitverif 0 -> don't display any hits, while 2 -> don't display shot hits
	if( cl_hitverif.GetInt() == 0 || (cl_hitverif.GetInt() == 2 && attackType == C_BaseBG2Weapon::ATTACKTYPE_FIREARM) )
		return;

	//Msg( "MsgFunc_HitVerif: %i %i %i %f\n", attacker, victim, hitgroup, damage );

	//accumulate partial HitVerif messages within a small time window
	//this makes buckshot damage display correctly with and without simulated bullets
	std::string message;

	if( C_BasePlayer::GetLocalPlayer()->entindex() == victim )
	{
		//local player is victim ("You were hit...")
		message = m_IsVictimAccumulator.Accumulate( damage, attacker, hitgroup );
	}
	else if( C_BasePlayer::GetLocalPlayer()->entindex() == attacker )
	{
		//"You hit..."
		message = m_IsAttackerAccumulator.Accumulate( damage, victim, hitgroup );
	}
	else
	{
		/**
		 * We're neither the attacker nor the victim.
		 * This should rarely happen since only the attacker and victim are added to
		 * this message's recipient filter. However, a user reported the indicator
		 * showing another player's damage, so return in this case just to be safe.
		 */
		return;
	}

	m_pLabelDamageVerificator->SetText( message.c_str() );
	m_flExpireTime = gpGlobals->curtime + 5.0f;
}

void CHudBG2::MsgFunc_WinMusic( bf_read &msg )
{
	if( !cl_winmusic.GetBool() )
		return;

	int team = msg.ReadByte();

	CLocalPlayerFilter filter;


	switch( team ) //Switches are more efficient.
	{
		case TEAM_AMERICANS:
			C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Americans.win" );
			break;
		case TEAM_BRITISH:
			C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "British.win" );
			break;
	}
}
//Make capture sounds client side. -HairyPotter
void CHudBG2::MsgFunc_CaptureSounds( bf_read &msg ) 
{
	if( !cl_capturesounds.GetBool() )
		return;

	Vector pos;
	char sound[255];

	msg.ReadBitVec3Coord( pos );
	msg.ReadString( sound, sizeof(sound) );
	PlayCaptureSound( pos, sound );
}
void CHudBG2::PlayCaptureSound( const Vector &origin, char sound[255] )
{
	CLocalPlayerFilter filter;

	//Hadto fix this because valve's code doesn't like direct wavs + scripts being played with the same code.
	EmitSound_t params;
	params.m_pSoundName = sound;
	params.m_flSoundTime = 0.0f;
	params.m_pOrigin = &origin;
	/*if ( params.m_flVolume == NULL )
	{
		Msg("The volume params aren't being set right for direct .wav sounds \n");
		params.m_flVolume = 0.8f;
	}*/
	if ( params.m_nChannel == NULL ) //Defualt to CHAN_AUTO for now.
	{
		//Msg("The channel params aren't being set right for direct .wav sounds \n");
		params.m_nChannel = CHAN_AUTO;
	}
	if ( params.m_SoundLevel == NULL )
	{
		//Msg("The soundlevel params aren't being set right for direct .wav sounds \n");
		params.m_SoundLevel = SNDLVL_60dB;
	}


	C_BaseEntity::EmitSound( filter, SOUND_FROM_WORLD, params );
	//
}
//
//Make voice comm sounds client side. -HairyPotter
void CHudBG2::MsgFunc_VCommSounds( bf_read &msg ) 
{
	if ( !cl_vcommsounds.GetBool() )
		return;

	int client	= msg.ReadByte();
	char snd[512];

	msg.ReadString( snd, sizeof(snd) );

	PlayVCommSound( snd, client );
}
void CHudBG2::PlayVCommSound( char snd[512], int playerindex )
{
	C_BasePlayer *pPlayer = UTIL_PlayerByIndex( playerindex );

	if ( pPlayer ) //Just make sure.
		pPlayer->EmitSound( snd );
}
//