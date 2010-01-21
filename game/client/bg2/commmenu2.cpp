//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include <cdll_client_int.h>
#include <globalvars_base.h>
#include <cdll_util.h>
#include <KeyValues.h>

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IPanel.h>
#include <vgui_controls/ImageList.h>
#include <vgui_controls/MenuItem.h>
#include <vgui_controls/Label.h>

#include <stdio.h> // _snprintf define

//#include "spectatorgui.h"
#include "commmenu2.h"
#include "c_hl2mp_player.h"
#include "../server/bg2/vcomm.h"
#include "vguicenterprint.h"

#include <game/client/iviewport.h>
#include "commandmenu.h"
#include "hltvcamera.h"

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Menu.h>
#include "IGameUIFuncs.h" // for key bindings
#include <imapoverview.h>
#include <shareddefs.h>
#include <igameresources.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern IGameUIFuncs *gameuifuncs; // for key binding details

using namespace vgui;

// EnforceOfficerForCommenu2
//  checks if the local player is an officer, and prints an error message if so is not the case
/*bool EnforceOfficerForCommenu2( void )
{
	C_HL2MP_Player *pPlayer = dynamic_cast<C_HL2MP_Player*> (C_HL2MP_Player::GetLocalPlayer());

	if( pPlayer && pPlayer->GetClass() == CLASS_OFFICER )
	{
		return true;
	}
	else
	{
		internalCenterPrint->Print( vgui::localize()->Find( "#BG2_VoiceComm2_Officer_Only" ) );
		return false;
	}
}*/

CCommMenu2::CCommMenu2( IViewPort *pViewPort ) : Frame( NULL, PANEL_COMM2 )
{
	slot1 = slot2 = slot3 = slot4 = slot5 = slot6 = slot7 = slot8 = slot9 = slot10 = -1;
	teammenu = classmenu = commmenu = commmenu2 = -1;
		
	m_pViewPort = pViewPort;

	SetMouseInputEnabled( false );
	SetKeyBoardInputEnabled( true );
	SetTitleBarVisible( false ); // don't draw a title bar
	SetMoveable( false );
	//SetMoveable( true );
	SetSizeable( false );
	//SetSizeable( true );
	SetProportional(true);
	//SetProportional(false);

	SetScheme("ClientScheme");

	SetSize( 640, 480 );

	int w,h;
	surface()->GetScreenSize(w, h);

	SetPos( 50, h - GetTall() );
	SetAlpha( 0 );
	SetPaintBackgroundEnabled( false );
	SetBorder( NULL );
	//SetVisible( true );

	m_pLabel = new Label( this, "label", g_pVGuiLocalize->Find("#BG2_VoiceComm_Menu_B") );
	m_pLabel->SetPos( 50, 50 );
	m_pLabel->SizeToContents();
}

void CCommMenu2::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	int w,h;
	surface()->GetScreenSize(w, h);

	SetPos( 50, h - GetTall() );
}

void CCommMenu2::PerformLayout()
{

}

void CCommMenu2::OnKeyCodePressed(KeyCode code)
{
	// we can't compare the keycode to a known code, because translation from bound keys
	// to vgui key codes is not 1:1. Get the engine version of the key for the binding
	// and the actual pressed key, and compare those..
	int iLastTrappedKey = /*engine->GetLastPressedEngineKey()*/code;	// the enginekey version of the code param

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !pPlayer )
		return;

#define ifkey( index, index2 ) if( iLastTrappedKey == slot##index ) { engine->ServerCmd( "voicecomm " #index2 ); ShowPanel( false ); }
			ifkey( 1, 9 )
	else	ifkey( 2, 10 )
	else	ifkey( 3, 11 )
	else	ifkey( 4, 12 )
	else	ifkey( 5, 13 )
	else	ifkey( 6, 14 )
	else	ifkey( 7, 15 )
	else	ifkey( 8, 16 )
	else	ifkey( 9, 17 )
	else if( iLastTrappedKey == slot10 )
		m_pViewPort->ShowPanel( this, false );
	else if( iLastTrappedKey == teammenu )
	{
		m_pViewPort->ShowPanel( PANEL_CLASSES, true );
		m_pViewPort->ShowPanel( PANEL_COMM, false );
		m_pViewPort->ShowPanel( PANEL_COMM2, false );
		
		IViewPortPanel *panel = gViewPortInterface->FindPanelByName( PANEL_CLASSES );
		if( panel )
			panel->SetData( (KeyValues*)0 );	//HACKHACK

		return;
	}
	else if( iLastTrappedKey == classmenu )
	{
		if( pPlayer->GetTeamNumber() > TEAM_SPECTATOR )
		{
			m_pViewPort->ShowPanel( PANEL_CLASSES, true );

			IViewPortPanel *panel = gViewPortInterface->FindPanelByName( PANEL_CLASSES );
			if( panel )
				panel->SetData( (KeyValues*)1 );	//HACKHACK
		}
		else
		{
			m_pViewPort->ShowPanel( PANEL_CLASSES, false );
			internalCenterPrint->Print( "You can\'t select class before selecting team" );
		}

		m_pViewPort->ShowPanel( PANEL_COMM, false );
		m_pViewPort->ShowPanel( PANEL_COMM2, false );

		return;
	}
	else if( iLastTrappedKey == commmenu )
	{
		if ( pPlayer->GetTeamNumber() <= TEAM_SPECTATOR || 
			!pPlayer->IsAlive() ) //Make sure the player is alive to use voicecomms. -HairyPotter
			return;

		m_pViewPort->ShowPanel( PANEL_CLASSES, false );
		m_pViewPort->ShowPanel( PANEL_COMM, true );
		m_pViewPort->ShowPanel( PANEL_COMM2, false );
		
		return;
	}
	else if( iLastTrappedKey == commmenu2 )
	{
		if ( pPlayer->GetTeamNumber() <= TEAM_SPECTATOR || 
			!pPlayer->IsAlive() ) //Make sure the player is alive to use voicecomms. -HairyPotter
			return;

		m_pViewPort->ShowPanel( PANEL_CLASSES, false );
		m_pViewPort->ShowPanel( PANEL_COMM, false );
		m_pViewPort->ShowPanel( PANEL_COMM2, false );
		
		return;
	}
	else
		BaseClass::OnKeyCodePressed( code );
}

void CCommMenu2::ShowPanel(bool bShow)
{
	if ( BaseClass::IsVisible() == bShow )
		return;

	if ( bShow )
	{
		Activate();
		SetMouseInputEnabled( false );
		SetKeyBoardInputEnabled( true );
	}
	else
	{
		SetVisible( false );
		SetMouseInputEnabled( false );
		SetKeyBoardInputEnabled( false );
	}

	m_pLabel->SetVisible( bShow );
}

void CCommMenu2::Update( void )
{
#define getkey( index ) if( slot##index < 0 ) slot##index = gameuifuncs->GetButtonCodeForBind( "slot" #index )
	getkey( 1 );
	getkey( 2 );
	getkey( 3 );
	getkey( 4 );
	getkey( 5 );
	getkey( 6 );
	getkey( 7 );
	getkey( 8 );
	getkey( 9 );
	getkey( 10 );

	if( teammenu < 0 ) teammenu = gameuifuncs->GetButtonCodeForBind( "teammenu" );
	if( classmenu < 0 ) classmenu = gameuifuncs->GetButtonCodeForBind( "classmenu" );
	if( commmenu < 0 ) commmenu = gameuifuncs->GetButtonCodeForBind( "commmenu" );
	if( commmenu2 < 0 ) commmenu2 = gameuifuncs->GetButtonCodeForBind( "commmenu2" );
}


