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
/*#include <vgui_controls/ImageList.h>
#include <vgui_controls/MenuItem.h>*/

#include <stdio.h> // _snprintf define

//#include "spectatorgui.h"
#include "c_team.h"
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

#include "classmenu.h"
#include "hl2mp_gamerules.h"

// Included for the port. -HairyPotter
#include "c_hl2mp_player.h"
//

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern IGameUIFuncs *gameuifuncs; // for key binding details

ConVar cl_quickjoin( "cl_quickjoin", "0", FCVAR_ARCHIVE, "Automatically join the game after choosing a class, spawing with the default weapon kit.");

#define CANCEL -2

using namespace vgui;

void ResolveLocalizedPath( const char *localString, char *pANSIPath, int iANSIPathSize )
{
	//this takes a "local" string and converts it to ANSI. used for paths, therefore the name
	const wchar_t *pPath = g_pVGuiLocalize->Find(localString);
	g_pVGuiLocalize->ConvertUnicodeToANSI( pPath, pANSIPath, iANSIPathSize );
}

//
//	CClassButton
//
void CClassButton::SetCommand( int command )
{
	m_iCommand = command;
}

void CClassButton::OnMousePressed(MouseCode code)
{
	CClassMenu * pThisMenu = (CClassMenu *)GetParent();

	if ( !pThisMenu )
		return;

	if ( m_iCommand == CANCEL )
	{
		pThisMenu->ToggleButtons( 1 );
		pThisMenu->ShowPanel( false );
		return;
	}


	pThisMenu->m_iClassSelection = m_iCommand; //This is used in the weapon/ammo display.

	cl_quickjoin.SetValue( pThisMenu->m_pQuickJoinCheckButton->IsSelected() ); //Set quickjoin var.

	if ( pThisMenu->m_pQuickJoinCheckButton->IsSelected() ) //Just join game with the default kit.
	{
		GetParent()->SetVisible( false );
		SetSelected( false );
		PerformCommand();
	}
	else
	{
		pThisMenu->ToggleButtons( 3 ); //Go to weapon selection.
	}	
}

void CClassButton::OnCursorEntered( void )
{
	BaseClass::OnCursorEntered();

	CClassMenu *pThisMenu = (CClassMenu*)GetParent();

	if ( !pThisMenu )
		return;

	char pANSIPath[512] = "readme.txt";	//a default value that doesn't risk crashes in case of missing resource files

	//BG2 - Make it a switch for great justice. -HairyPotter
	switch ( m_iCommand )
	{
		case CLASS_OFFICER:
			{
				if( pThisMenu->m_iTeamSelection == TEAM_AMERICANS )
					ResolveLocalizedPath( "#BG2_InfoHTML_A_Off_Path", pANSIPath, sizeof pANSIPath );
				else //if( pThisMenu->m_iTeamSelection == TEAM_BRITISH )
					ResolveLocalizedPath( "#BG2_InfoHTML_B_Off_Path", pANSIPath, sizeof pANSIPath );
				//else
					//return;
			}
			break;
		case CLASS_INFANTRY:
			{
				if( pThisMenu->m_iTeamSelection == TEAM_AMERICANS )
					ResolveLocalizedPath( "#BG2_InfoHTML_A_Inf_Path", pANSIPath, sizeof pANSIPath );
				else //if( pThisMenu->m_iTeamSelection == TEAM_BRITISH )
					ResolveLocalizedPath( "#BG2_InfoHTML_B_Inf_Path", pANSIPath, sizeof pANSIPath );
				//else
					//return;
			}
			break;
		case CLASS_SNIPER:
			{
				if( pThisMenu->m_iTeamSelection == TEAM_AMERICANS )
					ResolveLocalizedPath( "#BG2_InfoHTML_A_Rif_Path", pANSIPath, sizeof pANSIPath );
				else //if( pThisMenu->m_iTeamSelection == TEAM_BRITISH )
					ResolveLocalizedPath( "#BG2_InfoHTML_B_Rif_Path", pANSIPath, sizeof pANSIPath );
				//else
				//	return;
			}
			break;
		case CLASS_SKIRMISHER:
			{
				if( pThisMenu->m_iTeamSelection == TEAM_AMERICANS )
					ResolveLocalizedPath( "#BG2_InfoHTML_A_Ski_Path", pANSIPath, sizeof pANSIPath );
				else //if( pThisMenu->m_iTeamSelection == TEAM_BRITISH )
					ResolveLocalizedPath( "#BG2_InfoHTML_B_Ski_Path", pANSIPath, sizeof pANSIPath );
				//else
					//return;
			}
			break;
		case CLASS_LIGHT_INFANTRY:
			{
				//only british have light infantry
				if( pThisMenu->m_iTeamSelection == TEAM_BRITISH )
					ResolveLocalizedPath( "#BG2_InfoHTML_B_Linf_Path", pANSIPath, sizeof pANSIPath );
				//else
					//return;
			}
			break;
		default:
			return;
	}

	pThisMenu->ShowFile( pANSIPath );
}

void CClassButton::PerformCommand( void )
{
	CClassMenu *pThisMenu = (CClassMenu*)GetParent();

	if ( !pThisMenu )
		return;

	char cmd[64];
	Q_snprintf( cmd, sizeof( cmd ), "class %i %i \n", pThisMenu->m_iTeamSelection, m_iCommand );
	engine->ServerCmd( cmd );
}

//
//	CTeamButton
//
void CTeamButton::SetCommand( int command )
{
	m_iCommand = command;
}

void CTeamButton::OnMousePressed(MouseCode code)
{
	CClassMenu *pThisMenu = (CClassMenu *)GetParent();

	if ( !pThisMenu )
		return;

	SetSelected( false );

	if ( m_iCommand == CANCEL ) //Cancel Button
	{
		pThisMenu->ToggleButtons( 1 );
		pThisMenu->ShowPanel( false );
		return;
	}

	if( m_iCommand == TEAM_UNASSIGNED ) //Spectate
	{
		//join spectators
		engine->ServerCmd( "spectate", true );
		pThisMenu->ToggleButtons(1);
		pThisMenu->ShowPanel( false );
		return;
	}

	if( m_iCommand == -1 )
	{
		//autoassign
		int americans = g_Teams[TEAM_AMERICANS]->Get_Number_Players(),
			british = g_Teams[TEAM_BRITISH]->Get_Number_Players();

		//pick team with least players. or if equal, pick random
		if( americans > british )
			pThisMenu->m_iTeamSelection = TEAM_BRITISH;
		else if( americans < british )
			pThisMenu->m_iTeamSelection = TEAM_AMERICANS;
		else
			pThisMenu->m_iTeamSelection = random->RandomInt( TEAM_AMERICANS, TEAM_BRITISH );
	
		return;
	}

	PerformCommand();

	pThisMenu->ToggleButtons(2);
}

void CTeamButton::OnCursorEntered( void )
{
	BaseClass::OnCursorEntered();
	
	CClassMenu *pThisMenu = (CClassMenu *)GetParent();
	
	if ( !pThisMenu )
		return;

	char pANSIPath[512];

	//BG2 - Make it a switch for great justice. -HairyPotter
	switch ( m_iCommand )
	{
		case TEAM_AMERICANS:
			ResolveLocalizedPath( "#BG2_InfoHTML_Americans_Path", pANSIPath, sizeof pANSIPath );
			break;
		case TEAM_BRITISH:
			ResolveLocalizedPath( "#BG2_InfoHTML_British_Path", pANSIPath, sizeof pANSIPath );
			break;
		case TEAM_UNASSIGNED:	//Spectate
			ResolveLocalizedPath( "#BG2_InfoHTML_Spectate_Path", pANSIPath, sizeof pANSIPath );
			break;
		case -1:				//Autoassign
			ResolveLocalizedPath( "#BG2_InfoHTML_Autoassign_Path", pANSIPath, sizeof pANSIPath );
			break;
		default:
			return;
	}
	
	pThisMenu->ShowFile( pANSIPath );
}

void CTeamButton::PerformCommand( void )
{
	CClassMenu * pThisMenu = (CClassMenu *)GetParent();

	if ( !pThisMenu )
		return;

	pThisMenu->m_iTeamSelection = m_iCommand;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CClassMenu::CClassMenu( IViewPort *pViewPort ) : Frame( NULL, PANEL_CLASSES )
{
	m_iInfantryKey = m_iOfficerKey = m_iSniperKey = m_iSkirmisherKey = m_iCancelKey = m_iSlot6Key = 
	teammenu = classmenu = commmenu = commmenu2 = weaponmenu = -1; //Default all of these to -1
		
	m_pViewPort = pViewPort;

	SetMouseInputEnabled( true );
	SetKeyBoardInputEnabled( true );
	SetTitleBarVisible( false ); // don't draw a title bar
	SetMoveable( false );
	SetSizeable( false );
	//SetProportional(true);
	SetProportional( false );

	SetScheme("ClientScheme");

	/*SetSize( 640, 480 );
	SetPos( (ScreenWidth() - 640) / 2, (ScreenHeight() - 480) / 2 );*/

	m_pBritishButton = new CTeamButton( this, "BritishButton", "1. British" );
	m_pAmericanButton = new CTeamButton( this, "AmericanButton", "2. American" );
	m_pAutoassignButton = new CTeamButton( this, "AutoassignButton", "3. Autoassign team" );
	m_pSpectateButton = new CTeamButton( this, "SpectateButton", "4. Spectate" );
	
	m_pInfantryButton = new CClassButton( this, "InfantryButton", "1. Infantry(in code)" );
	m_pOfficerButton = new CClassButton( this, "OfficerButton", "2. Officer(in code)" );
	m_pSniperButton = new CClassButton( this, "RiflemanButton", "3. Rifleman(in code)" );
	m_pSkirmisherButton = new CClassButton( this, "SkirmisherButton", "4. Skirmisher(in code)" );
	m_pLightInfantryButton = new CClassButton( this, "LightInfantryButton", "5. Light infantry(in code)" );

	m_pQuickJoinCheckButton = new vgui::CheckButton( this, "QuickJoinCheckButton", "" );

	m_pWeaponButton1 = new CWeaponButton( this, "WeaponButton1", "Gun 1 (in code)" );
	m_pWeaponButton2 = new CWeaponButton( this, "WeaponButton2", "Gun 2( in code)" );
	m_pWeaponButton3 = new CWeaponButton( this, "WeaponButton3", "Gun 3 (in code)" );
	m_pAmmoButton1 = new CAmmoButton( this, "AmmoButton1", "Ammo 1 (in code)" );
	m_pAmmoButton2 = new CAmmoButton( this, "AmmoButton2", "Ammo 2 (in code)" );
	m_pAmmoButton3 = new CAmmoButton( this, "AmmoButton3", "Ammo 3 (in code)" );
	m_pOK = new COkayButton( this, "Okay", "Okay (in code)" );

	m_pCancelButton = new CClassButton( this, "CancelButton", "0. Cancel" );

	m_pInfoHTML = new HTML( this, "InfoHTML" );
	//m_pInfoHTML->SetPos( 100, 100 );
	//m_pInfoHTML->SetSize( 540, 380 );

	LoadControlSettings( "Resource/BG2Classmenu.res" );

	m_pBritishButton->SetCommand(TEAM_BRITISH);
	m_pAmericanButton->SetCommand(TEAM_AMERICANS);
	m_pAutoassignButton->SetCommand(-1);
	m_pSpectateButton->SetCommand(TEAM_UNASSIGNED);

	m_pOK->SetCommand( 1 );
	m_pCancelButton->SetCommand( CANCEL );

	m_pInfantryButton->SetCommand( CLASS_INFANTRY );
	m_pOfficerButton->SetCommand( CLASS_OFFICER );
	m_pSniperButton->SetCommand( CLASS_SNIPER );
	m_pSkirmisherButton->SetCommand( CLASS_SKIRMISHER );
	m_pLightInfantryButton->SetCommand( CLASS_LIGHT_INFANTRY );

	char pANSIPath[512];
	ResolveLocalizedPath( "#BG2_InfoHTML_Blank_Path", pANSIPath, sizeof pANSIPath );
	ShowFile( pANSIPath );

	ToggleButtons( 1 );
}

CClassMenu::~CClassMenu()
{
}

void CClassMenu::ShowFile( const char *filename )
{
	char localURL[_MAX_PATH + 7];
	Q_strncpy( localURL, "file://", sizeof( localURL ) );
		
	char pPathData[ _MAX_PATH ];
	g_pFullFileSystem->GetLocalPath( filename, pPathData, sizeof(pPathData) );
	Q_strncat( localURL, pPathData, sizeof( localURL ), COPY_ALL_CHARACTERS );

	m_pInfoHTML->OpenURL( localURL );
}

void CClassMenu::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetSize( 640, 480 );
	SetPos( (ScreenWidth() - 640) / 2, (ScreenHeight() - 480) / 2 );

	m_pAmericanButton->MakeReadyForUse();
	m_pBritishButton->MakeReadyForUse();
	m_pAmericanButton->SetBgColor( BLACK_BAR_COLOR );
	m_pBritishButton->SetBgColor( BLACK_BAR_COLOR );

	m_pInfantryButton->MakeReadyForUse();
	m_pOfficerButton->MakeReadyForUse();
	m_pSniperButton->MakeReadyForUse();
	m_pSkirmisherButton->MakeReadyForUse();
	m_pLightInfantryButton->MakeReadyForUse();

	m_pInfantryButton->SetBgColor( BLACK_BAR_COLOR );
	m_pOfficerButton->SetBgColor( BLACK_BAR_COLOR );
	m_pSniperButton->SetBgColor( BLACK_BAR_COLOR );
	m_pSkirmisherButton->SetBgColor( BLACK_BAR_COLOR );
	m_pLightInfantryButton->SetBgColor( BLACK_BAR_COLOR );

	// For weapons and ammo selection menu.. -HairyPotter
	m_pAmmoButton1->MakeReadyForUse();
	m_pAmmoButton2->MakeReadyForUse();
	m_pAmmoButton3->MakeReadyForUse();
	m_pWeaponButton1->MakeReadyForUse();
	m_pWeaponButton2->MakeReadyForUse();
	m_pWeaponButton3->MakeReadyForUse();
	m_pOK->MakeReadyForUse();

	m_pAmmoButton1->SetBgColor( BLACK_BAR_COLOR );
	m_pAmmoButton2->SetBgColor( BLACK_BAR_COLOR );
	m_pAmmoButton3->SetBgColor( BLACK_BAR_COLOR );
	m_pWeaponButton1->SetBgColor( BLACK_BAR_COLOR );
	m_pWeaponButton2->SetBgColor( BLACK_BAR_COLOR );
	m_pWeaponButton3->SetBgColor( BLACK_BAR_COLOR );
	m_pOK->SetBgColor( BLACK_BAR_COLOR );
	//

	m_pInfantryButton->SetBgColor( BLACK_BAR_COLOR );
	m_pOfficerButton->SetBgColor( BLACK_BAR_COLOR );
	m_pSniperButton->SetBgColor( BLACK_BAR_COLOR );
	m_pSkirmisherButton->SetBgColor( BLACK_BAR_COLOR );
	m_pLightInfantryButton->SetBgColor( BLACK_BAR_COLOR );

	m_pCancelButton->MakeReadyForUse();
	m_pCancelButton->SetBgColor( BLACK_BAR_COLOR );
}

//-----------------------------------------------------------------------------
// Purpose: makes the GUI fill the screen
//-----------------------------------------------------------------------------
void CClassMenu::PerformLayout()
{
}


//-----------------------------------------------------------------------------
// Purpose: when duck is pressed it hides the active part of the GUI
//-----------------------------------------------------------------------------
void CClassMenu::OnKeyCodePressed(KeyCode code)
{
	// we can't compare the keycode to a known code, because translation from bound keys
	// to vgui key codes is not 1:1. Get the engine version of the key for the binding
	// and the actual pressed key, and compare those..
//	int iLastTrappedKey = engine->GetLastPressedEngineKey();	// the enginekey version of the code param

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !pPlayer )
		return;

	MouseCode code2 = MOUSE_LEFT;	//for faking mouse presses
	if( code == m_iInfantryKey )
	{
		if( m_pBritishButton->IsVisible() ) //First slot, were in the Team Selection menu.
			m_pBritishButton->OnMousePressed( code2 );

		else if ( m_pInfantryButton->IsVisible() )
		{
			m_pInfantryButton->OnMousePressed( code2 );
		}
		else if ( m_pWeaponButton1->IsVisible() )
		{
			m_pWeaponButton1->OnMousePressed( code2 );
		}
	}
	else if( code == m_iOfficerKey )
	{
		if( m_pAmericanButton->IsVisible() ) //Second slot, were in the Team Selection menu.
			m_pAmericanButton->OnMousePressed( code2 );

		else if ( m_pOfficerButton->IsVisible() )
		{
			m_pOfficerButton->OnMousePressed( code2 );
		}
		else if ( m_pWeaponButton2->IsVisible() )
		{
			m_pWeaponButton2->OnMousePressed( code2 );
		}
	}
	else if( code == m_iSniperKey ) 
	{
		if( m_pAutoassignButton->IsVisible() ) //Third Slot...
			m_pAutoassignButton->OnMousePressed( code2 );

		else if( m_pSniperButton->IsVisible() )
		{
			m_pSniperButton->OnMousePressed( code2 );
		}
		else if ( m_pWeaponButton3->IsVisible() )
		{
			m_pWeaponButton3->OnMousePressed( code2 );
		}
	}
	else if( code == m_iSkirmisherKey )
	{
		if( m_pSpectateButton->IsVisible() ) //And fourth.
			m_pSpectateButton->OnMousePressed( code2 );

		else if( m_pSkirmisherButton->IsVisible() )
		{
			m_pSkirmisherButton->OnMousePressed( code2 );
		}
		else if ( m_pAmmoButton1->IsVisible() )
		{
			m_pAmmoButton1->OnMousePressed( code2 );
		}
	}
	else if( code == m_iLightInfantryKey )
	{
		if( m_pSpectateButton->IsVisible() ) //fifth
			m_pSpectateButton->OnMousePressed( code2 );

		else if( m_pLightInfantryButton->IsVisible() )
		{
			m_pLightInfantryButton->OnMousePressed( code2 );
		}
		else if ( m_pAmmoButton2->IsVisible() )
		{
			m_pAmmoButton2->OnMousePressed( code2 );
		}
	}
	else if( code == m_iSlot6Key ) //This is used only for
	{
		if ( m_pAmmoButton3->IsVisible() )
		{
			m_pAmmoButton3->OnMousePressed( code2 );
		}
	}
	else if( code == m_iCancelKey )
	{
		ToggleButtons(1);
		m_pViewPort->ShowPanel( this, false );
	}
	/*else if( code == m_iOkayKey )
	{
		if( m_pOK->IsVisible() ) //And fourth.
			m_pOK->OnMousePressed( code2 );
	}*/
	else if( code == teammenu )
	{
		if( m_pBritishButton->IsVisible() )
		{
			m_pViewPort->ShowPanel( PANEL_CLASSES, false );
			m_pViewPort->ShowPanel( PANEL_COMM, false );
			m_pViewPort->ShowPanel( PANEL_COMM2, false );
		}
		else
			ToggleButtons( 1 );
		
		return;
	}
	else if( code == classmenu )
	{
		if( IsInClassMenu() )
		{
			m_pViewPort->ShowPanel( PANEL_CLASSES, false );
			m_pViewPort->ShowPanel( PANEL_COMM, false );
			m_pViewPort->ShowPanel( PANEL_COMM2, false );
		}
		else if( !IsInClassMenu() && pPlayer->GetTeamNumber() <= TEAM_SPECTATOR )
		{
			internalCenterPrint->Print( "You can\'t select class before selecting team" );
			m_pViewPort->ShowPanel( PANEL_CLASSES, false );
			m_pViewPort->ShowPanel( PANEL_COMM, false );
			m_pViewPort->ShowPanel( PANEL_COMM2, false );
		}
		else
			ToggleButtons( 2 );
		
		return;
	}
	else if( code == weaponmenu )
	{
		if( IsInClassMenu() )
		{
			m_pViewPort->ShowPanel( PANEL_CLASSES, false );
			m_pViewPort->ShowPanel( PANEL_COMM, false );
			m_pViewPort->ShowPanel( PANEL_COMM2, false );
		}
		else if( !IsInClassMenu() && pPlayer->GetTeamNumber() <= TEAM_SPECTATOR )
		{
			internalCenterPrint->Print( "You can\'t select a weapon before choosing a team and class." );
			m_pViewPort->ShowPanel( PANEL_CLASSES, false );
			m_pViewPort->ShowPanel( PANEL_COMM, false );
			m_pViewPort->ShowPanel( PANEL_COMM2, false );
		}
		else
			ToggleButtons( 3 );
		
		return;
	}
	else if( code == commmenu )
	{
		if ( !pPlayer->IsAlive() ) //Make sure the player is on a team and alive to use voicecomms. -HairyPotter
			return;

		m_pViewPort->ShowPanel( PANEL_CLASSES, false );
		m_pViewPort->ShowPanel( PANEL_COMM, true );
		m_pViewPort->ShowPanel( PANEL_COMM2, false );
		
		return;
	}
	else if( code == commmenu2 )
	{
		if ( !pPlayer->IsAlive() ) //Make sure the player is on a team and alive to use voicecomms. -HairyPotter
			return;

		m_pViewPort->ShowPanel( PANEL_CLASSES, false );
		m_pViewPort->ShowPanel( PANEL_COMM, false );
		m_pViewPort->ShowPanel( PANEL_COMM2, true );
		
		return;
	}
	else
		BaseClass::OnKeyCodePressed( code );
}

void CClassMenu::ShowPanel(bool bShow)
{
	if ( BaseClass::IsVisible() == bShow )
		return;

	//clear html screen
	char pANSIPath[512];
	ResolveLocalizedPath( "#BG2_InfoHTML_Blank_Path", pANSIPath, sizeof pANSIPath );
	ShowFile( pANSIPath );

	if ( bShow )
	{
		Activate();
		SetMouseInputEnabled( true );
		SetKeyBoardInputEnabled( true );
	}
	else
	{
		SetVisible( false );
		SetMouseInputEnabled( false );
		SetKeyBoardInputEnabled( false );
	}

	m_pCancelButton->SetVisible( bShow );
}





//-----------------------------------------------------------------------------
// Purpose: Resets the list of players
//-----------------------------------------------------------------------------
void CClassMenu::Update( void )
{
	
	if( m_iInfantryKey < 0 ) m_iInfantryKey = gameuifuncs->GetButtonCodeForBind( "slot1" );
	if( m_iOfficerKey < 0 ) m_iOfficerKey = gameuifuncs->GetButtonCodeForBind( "slot2" );
	if( m_iSniperKey < 0 ) m_iSniperKey = gameuifuncs->GetButtonCodeForBind( "slot3" );
	if( m_iSkirmisherKey < 0 ) m_iSkirmisherKey = gameuifuncs->GetButtonCodeForBind( "slot4" );
	if( m_iLightInfantryKey < 0 ) m_iLightInfantryKey = gameuifuncs->GetButtonCodeForBind( "slot5" );
	//Below are for ammo selection only.
	if( m_iSlot6Key < 0 ) m_iSlot6Key = gameuifuncs->GetButtonCodeForBind( "slot6" );
	//

	if( m_iCancelKey < 0 ) m_iCancelKey = gameuifuncs->GetButtonCodeForBind( "slot10" );
	//if( m_iOkayKey < 0 ) m_iOkayKey = gameuifuncs->GetButtonCodeForBind( "enter" );

	if( teammenu < 0 ) teammenu = gameuifuncs->GetButtonCodeForBind( "teammenu" );
	if( classmenu < 0 ) classmenu = gameuifuncs->GetButtonCodeForBind( "classmenu" );
	if( weaponmenu < 0 ) weaponmenu = gameuifuncs->GetButtonCodeForBind( "weaponmenu" );
	if( commmenu < 0 ) commmenu = gameuifuncs->GetButtonCodeForBind( "commmenu" );
	if( commmenu2 < 0 ) commmenu2 = gameuifuncs->GetButtonCodeForBind( "commmenu2" );
}

void CClassMenu::OnThink()
{
	if ( !engine->IsInGame() ) //This prevents a crash when disconnecting from a game with the classmenu open, then rejoining. - HairyPotter
	{
		ToggleButtons( 1 ); //Reset the menu.
		return;
	}

	BaseClass::OnThink();
	//BG2 - Tjoppen - show autoassign button if we're not in the class part and we're unassigned or spectator
	m_pAutoassignButton->SetVisible( IsInClassMenu() ? false : C_BasePlayer::GetLocalPlayer() ? C_BasePlayer::GetLocalPlayer()->GetTeamNumber() <= TEAM_SPECTATOR : false );

	//show number of players in teams..
	char tmp[256];
	if( g_Teams[TEAM_BRITISH] )		//check just in case..
	{
		sprintf( tmp, "1. British (%i)", g_Teams[TEAM_BRITISH]->Get_Number_Players() );
		m_pBritishButton->SetText( tmp );
	}

	if( g_Teams[TEAM_AMERICANS] )	//check just in case..
	{
		sprintf( tmp, "2. American (%i)", g_Teams[TEAM_AMERICANS]->Get_Number_Players() );
		m_pAmericanButton->SetText( tmp );
	}

	//BG2 - Draco - set the proper names for the classes
	//BG2 - Tjoppen - updated to use UpdateClassButtonText()
	UpdateClassButtonText( m_pInfantryButton, CLASS_INFANTRY, 
			m_iTeamSelection == TEAM_AMERICANS ? "1. Continental Soldier" : "1. Royal Infantry" );

	UpdateClassButtonText( m_pOfficerButton, CLASS_OFFICER, 
			m_iTeamSelection == TEAM_AMERICANS ? "2. Continental Officer" : "2. Royal Commander" );

	UpdateClassButtonText( m_pSniperButton, CLASS_SNIPER, 
			m_iTeamSelection == TEAM_AMERICANS ? "3. Frontiersman" : "3. Jaeger" );

	UpdateClassButtonText( m_pSkirmisherButton, CLASS_SKIRMISHER, 
			m_iTeamSelection == TEAM_AMERICANS ? "4. Militia" : "4. Native" );

	UpdateClassButtonText( m_pLightInfantryButton, CLASS_LIGHT_INFANTRY, "5. Light Infantry" );

	UpdateWeaponButtonText( m_iTeamSelection, m_iClassSelection );
	UpdateAmmoButtonText( m_iTeamSelection, m_iClassSelection );
}

void CClassMenu::UpdateClassButtonText( CClassButton *pButton, int iClass, const char *pPrefix )
{
	//BG2 - Tjoppen - this function figures out what to print on the specified button's text field
	//					based on class limits and such
	char temp[512];
	int limit = HL2MPRules()->GetLimitTeamClass(m_iTeamSelection, iClass);

	if( m_iTeamSelection != TEAM_AMERICANS && m_iTeamSelection != TEAM_BRITISH )
		return;

	C_Team *pCurrentTeam = g_Teams[m_iTeamSelection];

	if( !pCurrentTeam )
		return;

	if( limit >= 0 )
		Q_snprintf( temp, sizeof temp, "%s (%i/%i)", pPrefix, pCurrentTeam->GetNumOfClass(iClass), limit );
	else
		Q_snprintf( temp, sizeof temp, "%s (%i)", pPrefix, pCurrentTeam->GetNumOfClass(iClass) );

	pButton->SetText( temp );
}

void CClassMenu::UpdateAmmoButtonText( int iTeam, int iClass )
{
	//This may not even be necessary if we follow through with this idea.
	switch ( iTeam )
	{
		case TEAM_AMERICANS:
			switch ( iClass )
			{
				case CLASS_OFFICER:
					m_pAmmoButton1->SetText( "4. Coded in later.." );
					m_pAmmoButton2->SetText( "5. Coded in later.." );
					m_pAmmoButton3->SetText( "6. Coded in later.." );
					break;
				case CLASS_INFANTRY:
					m_pAmmoButton1->SetText( "4. Coded in later.." );
					m_pAmmoButton2->SetText( "5. Coded in later.." );
					m_pAmmoButton3->SetText( "6. Coded in later.." );
					break;
				case CLASS_SNIPER:
					m_pAmmoButton1->SetText( "4. Coded in later.." );
					m_pAmmoButton2->SetText( "5. Coded in later.." );
					m_pAmmoButton3->SetText( "6. Coded in later.." );
					break;
				case CLASS_SKIRMISHER:
					m_pAmmoButton1->SetText( "4. Coded in later.." );
					m_pAmmoButton2->SetText( "5. Coded in later.." );
					m_pAmmoButton3->SetText( "6. Coded in later.." );
					break;
				default:
					m_pAmmoButton1->SetVisible( false );
					m_pAmmoButton2->SetVisible( false );
					m_pAmmoButton3->SetVisible( false );
					//Msg("There was an error determining which American class you chose to play as. \n");
					break;
			}
			break;
		case TEAM_BRITISH:
			switch ( iClass )
			{
				case CLASS_OFFICER:
					m_pAmmoButton1->SetText( "4. Coded in later.." );
					m_pAmmoButton2->SetText( "5. Coded in later.." );
					m_pAmmoButton3->SetText( "6. Coded in later.." );
					break;
				case CLASS_INFANTRY:
					m_pAmmoButton1->SetText( "4. Coded in later.." );
					m_pAmmoButton2->SetText( "5. Coded in later.." );
					m_pAmmoButton3->SetText( "6. Coded in later.." );
					break;
				case CLASS_SNIPER:
					m_pAmmoButton1->SetText( "4. Coded in later.." );
					m_pAmmoButton2->SetText( "5. Coded in later.." );
					m_pAmmoButton3->SetText( "6. Coded in later.." );
					break;
				case CLASS_SKIRMISHER:
					m_pAmmoButton1->SetText( "4. Coded in later.." );
					m_pAmmoButton2->SetText( "5. Coded in later.." );
					m_pAmmoButton3->SetText( "6. Coded in later.." );
					break;
				case CLASS_LIGHT_INFANTRY:
					m_pAmmoButton1->SetText( "4. Coded in later.." );
					m_pAmmoButton2->SetText( "5. Coded in later.." );
					m_pAmmoButton3->SetText( "6. Coded in later.." );
					break;
				default:
					m_pAmmoButton1->SetVisible( false );
					m_pAmmoButton2->SetVisible( false );
					m_pAmmoButton3->SetVisible( false );
					//Msg("There was an error determining which British class you chose to play as. \n");
					break;
			}
			break;
	}
	//
}

void CClassMenu::UpdateWeaponButtonText( int iTeam, int iClass )
{
	switch ( iTeam )
	{
		case TEAM_AMERICANS:
			switch ( iClass )
			{
				case CLASS_OFFICER:
					m_pWeaponButton1->SetText( "1. American Pistol + Sword" );
					m_pWeaponButton2->SetVisible( false );
					m_pWeaponButton3->SetVisible( false );
					break;
				case CLASS_INFANTRY:
					m_pWeaponButton1->SetText( "1. Charleville" );
					m_pWeaponButton2->SetText( "2. American Brownbess" );
					m_pWeaponButton3->SetText( "3. Revolutionnaire" );
					break;
				case CLASS_SNIPER:
					m_pWeaponButton1->SetText( "1. Pennsylvania Longrifle + Knife" );
					m_pWeaponButton2->SetVisible( false );
					m_pWeaponButton3->SetVisible( false );
					break;
				case CLASS_SKIRMISHER: 
					m_pWeaponButton1->SetText( "1. Fowler + BeltAxe" );
					m_pWeaponButton2->SetText( "2. American Brownbess + BeltAxe" );
					m_pWeaponButton3->SetVisible( false );
					break;
				default:
					m_pWeaponButton1->SetVisible( false );
					m_pWeaponButton2->SetVisible( false );
					m_pWeaponButton3->SetVisible( false );
					//Msg("There was an error determining which American class you chose to play as. \n");
					break;
			}
			break;
		case TEAM_BRITISH:
			switch ( iClass )
			{
				case CLASS_OFFICER:
					m_pWeaponButton1->SetText( "1. Sabre + British Pistol" );
					m_pWeaponButton2->SetVisible( false );
					m_pWeaponButton3->SetVisible( false );
					break;
				case CLASS_INFANTRY:
					m_pWeaponButton1->SetText( "1. Brownbess" );
					m_pWeaponButton2->SetText( "2. Longpattern" );
					m_pWeaponButton3->SetVisible( false );
					break;
				case CLASS_SNIPER:
					m_pWeaponButton1->SetText( "1. Jaeger Rifle + Hirschfanger" );
					m_pWeaponButton2->SetVisible( false );
					m_pWeaponButton3->SetVisible( false );
					break;
				case CLASS_SKIRMISHER:
					m_pWeaponButton1->SetText( "1. Brownbess + Tomahawk" );
					m_pWeaponButton2->SetText( "2. Longpattern + Tomahawk" );
					m_pWeaponButton3->SetVisible( false );
					break;
				case CLASS_LIGHT_INFANTRY:
					m_pWeaponButton1->SetText( "1. Brownbess carbine" );
					m_pWeaponButton2->SetVisible( false );
					m_pWeaponButton3->SetVisible( false );
					break;
				default:
					m_pWeaponButton1->SetVisible( false );
					m_pWeaponButton2->SetVisible( false );
					m_pWeaponButton3->SetVisible( false );
					//Msg("There was an error determining which British class you chose to play as. \n");
					break;
			}
			break;
		default:
			//Msg("There was an error determining which class you chose in the menu. \n");
			break;
	}
}

void CClassMenu::SetData(KeyValues *data)
{
	//HACKHACK
	if( (int)data == 1 )
		ToggleButtons(2);
	else if( (int)data == 0 )
		ToggleButtons(1);
}

void CClassMenu::ToggleButtons(int iShowScreen)
{
	switch (iShowScreen)
	{
		case 1:
			m_pInfoHTML->SetVisible(true);
			m_pCancelButton->SetVisible(true);
			m_pBritishButton->SetVisible(true);
			m_pAmericanButton->SetVisible(true);
			//BG2 - Tjoppen - show autoassign button if we're not in the class part and we're unassigned or spectator
			m_pAutoassignButton->SetVisible( IsInClassMenu() ? false : C_BasePlayer::GetLocalPlayer() ? C_BasePlayer::GetLocalPlayer()->GetTeamNumber() <= TEAM_SPECTATOR : false );
			//
			m_pSpectateButton->SetVisible(true);
			m_pOfficerButton->SetVisible(false);
			m_pInfantryButton->SetVisible(false);
			m_pSniperButton->SetVisible(false);
			m_pSkirmisherButton->SetVisible(false);
			m_pLightInfantryButton->SetVisible(false);
			m_pWeaponButton1->SetVisible(false);
			m_pWeaponButton2->SetVisible(false);
			m_pWeaponButton3->SetVisible(false);
			m_pAmmoButton1->SetVisible(false);
			m_pAmmoButton2->SetVisible(false);
			m_pAmmoButton3->SetVisible(false);
			m_pOK->SetVisible(false);
			m_pQuickJoinCheckButton->SetVisible( false );
			break;
		case 2:
			m_pInfoHTML->SetVisible(true);
			m_pCancelButton->SetVisible(true);
			m_pBritishButton->SetVisible(false);
			m_pAmericanButton->SetVisible(false);
			m_pAutoassignButton->SetVisible(false);
			m_pSpectateButton->SetVisible(false);
			m_pOfficerButton->SetVisible(true);
			m_pInfantryButton->SetVisible(true);
			m_pSniperButton->SetVisible(true);
			m_pSkirmisherButton->SetVisible(true);
			m_pLightInfantryButton->SetVisible(m_iTeamSelection == TEAM_BRITISH);
			m_pWeaponButton1->SetVisible(false);
			m_pWeaponButton2->SetVisible(false);
			m_pWeaponButton3->SetVisible(false);
			m_pAmmoButton1->SetVisible(false);
			m_pAmmoButton2->SetVisible(false);
			m_pAmmoButton3->SetVisible(false);
			m_pOK->SetVisible(false);
			m_pQuickJoinCheckButton->SetVisible( true );
			m_pQuickJoinCheckButton->SetSelected( cl_quickjoin.GetBool() );
			break;
		case 3:
			m_pInfoHTML->SetVisible(false);
			m_pCancelButton->SetVisible(true);
			m_pBritishButton->SetVisible(false);
			m_pAmericanButton->SetVisible(false);
			m_pAutoassignButton->SetVisible(false);
			m_pSpectateButton->SetVisible(false);
			m_pOfficerButton->SetVisible(false);
			m_pInfantryButton->SetVisible(false);
			m_pSniperButton->SetVisible(false);
			m_pSkirmisherButton->SetVisible(false);
			m_pLightInfantryButton->SetVisible(false);
			m_pWeaponButton1->SetVisible(true);
			m_pWeaponButton2->SetVisible(true);
			m_pWeaponButton3->SetVisible(true);
			m_pAmmoButton1->SetVisible(true);
			m_pAmmoButton2->SetVisible(true);
			m_pAmmoButton3->SetVisible(true);
			m_pOK->SetVisible(true);
			m_pQuickJoinCheckButton->SetVisible( false );
			break;
	}
}

void CAmmoButton::OnMousePressed(MouseCode code)
{
	CClassMenu *pThisMenu = (CClassMenu*)GetParent();

	if ( !pThisMenu )
		return;

	//Force the buttons to "reset" so we don't get mixed signals sent to the server.
	pThisMenu->m_pAmmoButton1->SetSelected ( false );
	pThisMenu->m_pAmmoButton2->SetSelected ( false );
	pThisMenu->m_pAmmoButton3->SetSelected ( false );

	SetSelected( true );
}

void CAmmoButton::OnCursorEntered( void )
{
	BaseClass::OnCursorEntered();
}

void CWeaponButton::OnMousePressed(MouseCode code)
{
	CClassMenu *pThisMenu = (CClassMenu*)GetParent();

	if ( !pThisMenu )
		return;

	//Force the buttons to "reset" so we don't get mixed signals sent to the server.
	pThisMenu->m_pWeaponButton1->SetSelected ( false );
	pThisMenu->m_pWeaponButton2->SetSelected ( false );
	pThisMenu->m_pWeaponButton3->SetSelected ( false );
	//

	SetSelected( true );
}

void CWeaponButton::OnCursorEntered( void )
{
	BaseClass::OnCursorEntered();
}

//The button that gathers the weapons and ammo kit settings and sends the server command.
void COkayButton::SetCommand( int command, bool ammo )
{
}

void COkayButton::OnMousePressed(MouseCode code)
{
	SetSelected( false );
	PerformCommand();
}

void COkayButton::OnCursorEntered( void )
{
	BaseClass::OnCursorEntered();
}

void COkayButton::PerformCommand( void )
{
	CClassMenu *pThisMenu = (CClassMenu*)GetParent();

	if ( !pThisMenu )
		return;

	int m_iGunType = 1,
		m_iAmmoType = AMMO_KIT_BALL;

	pThisMenu->ToggleButtons( 1 ); 
	GetParent()->SetVisible( false );

	if ( pThisMenu->m_pWeaponButton1->IsSelected() )
		m_iGunType = 1;

	if ( pThisMenu->m_pWeaponButton2->IsSelected() )
		m_iGunType = 2;

	if ( pThisMenu->m_pWeaponButton3->IsSelected() )
		m_iGunType = 3;

	//Note: use AMMO_KIT_* instead of magic numbers
	if ( pThisMenu->m_pAmmoButton1->IsSelected() )
		m_iAmmoType = AMMO_KIT_BALL;

	if ( pThisMenu->m_pAmmoButton2->IsSelected() )
		m_iAmmoType = AMMO_KIT_BUCKSHOT;

	if ( pThisMenu->m_pAmmoButton3->IsSelected() )
		m_iAmmoType = AMMO_KIT_RESERVED1;

	char cmd[64];
	Q_snprintf( cmd, sizeof( cmd ), "kit %i %i \n", m_iGunType, m_iAmmoType );
	engine->ServerCmd( cmd );

	char sClass[64];
	Q_snprintf( sClass, sizeof( sClass ), "class %i %i \n", pThisMenu->m_iTeamSelection, pThisMenu->m_iClassSelection );
	engine->ServerCmd( sClass ); //Send the class AFTER we've selected a gun, otherwise you won't spawn with the right kit.
}