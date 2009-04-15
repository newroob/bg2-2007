//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
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
	pThisMenu->ToggleButtons(1);
	GetParent()->SetVisible( false );
	SetSelected( false );
	PerformCommand();
}

void CClassButton::OnCursorEntered( void )
{
	BaseClass::OnCursorEntered();

	CClassMenu *pThisMenu = (CClassMenu*)GetParent();
	char pANSIPath[512];

	//BG2 - Make it a switch for great justice. -HairyPotter
	switch ( m_iCommand )
	{
		case 1: //Officer
			{
				if( pThisMenu->m_iTeamSelection == TEAM_AMERICANS )
					ResolveLocalizedPath( "#BG2_InfoHTML_A_Off_Path", pANSIPath, sizeof pANSIPath );
				else if( pThisMenu->m_iTeamSelection == TEAM_BRITISH )
					ResolveLocalizedPath( "#BG2_InfoHTML_B_Off_Path", pANSIPath, sizeof pANSIPath );
				else
					return;
			}
			break;
		case 2: //Infantry
			{
				if( pThisMenu->m_iTeamSelection == TEAM_AMERICANS )
					ResolveLocalizedPath( "#BG2_InfoHTML_A_Inf_Path", pANSIPath, sizeof pANSIPath );
				else if( pThisMenu->m_iTeamSelection == TEAM_BRITISH )
					ResolveLocalizedPath( "#BG2_InfoHTML_B_Inf_Path", pANSIPath, sizeof pANSIPath );
				else
					return;
			}
			break;
		case 3: //Rifleman
			{
				if( pThisMenu->m_iTeamSelection == TEAM_AMERICANS )
					ResolveLocalizedPath( "#BG2_InfoHTML_A_Rif_Path", pANSIPath, sizeof pANSIPath );
				else if( pThisMenu->m_iTeamSelection == TEAM_BRITISH )
					ResolveLocalizedPath( "#BG2_InfoHTML_B_Rif_Path", pANSIPath, sizeof pANSIPath );
				else
					return;
			}
			break;
		case 4: //Skirmisher
			{
				if( pThisMenu->m_iTeamSelection == TEAM_AMERICANS )
					ResolveLocalizedPath( "#BG2_InfoHTML_A_Rif_Path", pANSIPath, sizeof pANSIPath ); //Changeme - HairyPotter
				else if( pThisMenu->m_iTeamSelection == TEAM_BRITISH )
					ResolveLocalizedPath( "#BG2_InfoHTML_B_Rif_Path", pANSIPath, sizeof pANSIPath ); //Changeme - HairyPotter
				else
					return;
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

	//BG2 - Make it a switch for great justice. -HairyPotter
	switch ( m_iCommand )
	{
		case 1: //Officer
			{
				if( pThisMenu->m_iTeamSelection == TEAM_AMERICANS )
					engine->ServerCmd( "light_a" );
				else //if( pThisMenu->m_iTeamSelection == TEAM_BRITISH )
					engine->ServerCmd( "light_b" );
			}
			break;
		case 2: //Infantry
			{
				if( pThisMenu->m_iTeamSelection == TEAM_AMERICANS )
					engine->ServerCmd( "heavy_a" );
				else //if( pThisMenu->m_iTeamSelection == TEAM_BRITISH )
					engine->ServerCmd( "medium_b" );
			}
			break;
		case 3: //Rifleman
			{
				if( pThisMenu->m_iTeamSelection == TEAM_AMERICANS )
					engine->ServerCmd( "medium_a" );
				else //if( pThisMenu->m_iTeamSelection == TEAM_BRITISH )
					engine->ServerCmd( "heavy_b" );
			}
			break;
		case 4: //Skirmisher
			{
				if( pThisMenu->m_iTeamSelection == TEAM_AMERICANS )
					engine->ServerCmd( "skirm_a" );
				else //if( pThisMenu->m_iTeamSelection == TEAM_BRITISH )
					engine->ServerCmd( "skirm_b" );
			}
			break;
		default:
			return;
	}
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
	SetSelected( false );

	if( m_iCommand == TEAM_UNASSIGNED )
	{
		//join spectators
		engine->ServerCmd( "spectate", true );
		pThisMenu->ToggleButtons(1);
		pThisMenu->ShowPanel( false );
		return;
	}

	pThisMenu->ToggleButtons(2);
	
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
}

void CTeamButton::OnCursorEntered( void )
{
	BaseClass::OnCursorEntered();
	
	CClassMenu *pThisMenu = (CClassMenu *)GetParent();
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
	pThisMenu->m_iTeamSelection = m_iCommand;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CClassMenu::CClassMenu( IViewPort *pViewPort ) : Frame( NULL, PANEL_CLASSES )
{
	m_iInfantryKey = m_iOfficerKey = m_iSniperKey = m_iSkirmisherKey = -1;
	m_iCancelKey = -1;
	m_iSpectateKey = -1;
	teammenu = classmenu = commmenu = commmenu2 = -1;
		
	m_pViewPort = pViewPort;

	SetMouseInputEnabled( true );
	SetKeyBoardInputEnabled( true );
	SetTitleBarVisible( false ); // don't draw a title bar
	SetMoveable( false );
	SetSizeable( false );
	//SetProportional(true);
	SetProportional(false);

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

	m_pCancelButton = new CClassButton( this, "CancelButton", "0. Cancel" );

	m_pInfoHTML = new HTML( this, "InfoHTML" );
	//m_pInfoHTML->SetPos( 100, 100 );
	//m_pInfoHTML->SetSize( 540, 380 );

	LoadControlSettings( "Resource/BG2Classmenu.res" );

	m_pBritishButton->SetCommand(TEAM_BRITISH);
	m_pAmericanButton->SetCommand(TEAM_AMERICANS);
	m_pAutoassignButton->SetCommand(-1);
	m_pSpectateButton->SetCommand(TEAM_UNASSIGNED);

	m_pInfantryButton->SetCommand( 2 );
	m_pOfficerButton->SetCommand( 1 );
	m_pSniperButton->SetCommand( 3 );
	m_pSkirmisherButton->SetCommand( 4 );

	char pANSIPath[512];
	ResolveLocalizedPath( "#BG2_InfoHTML_Blank_Path", pANSIPath, sizeof pANSIPath );
	ShowFile( pANSIPath );

	ToggleButtons( 1 );
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

	m_pInfantryButton->SetBgColor( BLACK_BAR_COLOR );
	m_pOfficerButton->SetBgColor( BLACK_BAR_COLOR );
	m_pSniperButton->SetBgColor( BLACK_BAR_COLOR );
	m_pSkirmisherButton->SetBgColor( BLACK_BAR_COLOR );

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

//	C_BasePlayer *pPlayer = (C_BasePlayer *) C_BasePlayer::GetLocalPlayer();


	// we can't compare the keycode to a known code, because translation from bound keys
	// to vgui key codes is not 1:1. Get the engine version of the key for the binding
	// and the actual pressed key, and compare those..
//	int iLastTrappedKey = engine->GetLastPressedEngineKey();	// the enginekey version of the code param

	MouseCode code2 = MOUSE_LEFT;	//for faking mouse presses
	if( code/*iLastTrappedKey*/ == m_iInfantryKey )
	{
		if( m_pBritishButton->IsVisible() )
			m_pBritishButton->OnMousePressed( code2 );
		else
		{
			m_pInfantryButton->OnMousePressed( code2 );
			m_pViewPort->ShowPanel( this, false );
		}
	}
	else if( code == m_iOfficerKey )
	{
		if( m_pAmericanButton->IsVisible() )
			m_pAmericanButton->OnMousePressed( code2 );
		else
		{
			m_pOfficerButton->OnMousePressed( code2 );
			m_pViewPort->ShowPanel( this, false );
		}
	}
	else if( code == m_iSniperKey )
	{
		if( m_pAutoassignButton->IsVisible() )
			m_pAutoassignButton->OnMousePressed( code2 );
		else if( m_pSniperButton->IsVisible() )
		{
			m_pSniperButton->OnMousePressed( code2 );
			m_pViewPort->ShowPanel( this, false );
		}
	}
	else if( code == m_iSkirmisherKey )
	{
		if( m_pAutoassignButton->IsVisible() )
			m_pAutoassignButton->OnMousePressed( code2 );
		else if( m_pSkirmisherButton->IsVisible() )
		{
			m_pSkirmisherButton->OnMousePressed( code2 );
			m_pViewPort->ShowPanel( this, false );
		}
	}
	else if( code == m_iSpectateKey )
	{
		if( m_pSpectateButton->IsVisible() )
		{
			m_pSpectateButton->OnMousePressed( code2 );
			m_pViewPort->ShowPanel( this, false );
		}
	}
	else if( code == m_iCancelKey )
	{
		ToggleButtons(1);
		m_pViewPort->ShowPanel( this, false );
	}
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
		else if( !IsInClassMenu() && C_BasePlayer::GetLocalPlayer()->GetTeamNumber() <= TEAM_SPECTATOR )
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
	else if( code == commmenu )
	{
		if ( !C_BasePlayer::GetLocalPlayer()->IsAlive() ) //Make sure the player is on a team and alive to use voicecomms. -HairyPotter
			return;

		m_pViewPort->ShowPanel( PANEL_CLASSES, false );
		m_pViewPort->ShowPanel( PANEL_COMM, true );
		m_pViewPort->ShowPanel( PANEL_COMM2, false );
		
		return;
	}
	else if( code == commmenu2 )
	{
		if ( !C_BasePlayer::GetLocalPlayer()->IsAlive() ) //Make sure the player is on a team and alive to use voicecomms. -HairyPotter
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
	if( m_iSpectateKey < 0 ) m_iSpectateKey = gameuifuncs->GetButtonCodeForBind( "slot5" );

	if( m_iCancelKey < 0 ) m_iCancelKey = gameuifuncs->GetButtonCodeForBind( "slot10" );

	if( teammenu < 0 ) teammenu = gameuifuncs->GetButtonCodeForBind( "teammenu" );
	if( classmenu < 0 ) classmenu = gameuifuncs->GetButtonCodeForBind( "classmenu" );
	if( commmenu < 0 ) commmenu = gameuifuncs->GetButtonCodeForBind( "commmenu" );
	if( commmenu2 < 0 ) commmenu2 = gameuifuncs->GetButtonCodeForBind( "commmenu2" );
}

void CClassMenu::OnThink()
{
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
			break;
		case 2:
			m_pBritishButton->SetVisible(false);
			m_pAmericanButton->SetVisible(false);
			m_pAutoassignButton->SetVisible(false);
			m_pSpectateButton->SetVisible(false);
			m_pOfficerButton->SetVisible(true);
			m_pInfantryButton->SetVisible(true);
			m_pSniperButton->SetVisible(true);
			m_pSkirmisherButton->SetVisible(true);
			break;
	}
}