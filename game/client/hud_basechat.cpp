//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hud_basechat.h"

#include <vgui/IScheme.h>
#include <vgui/IVGui.h>
#include "iclientmode.h"
#include "hud_macros.h"
#include "engine/IEngineSound.h"
#include "text_message.h"
#include <vgui/ILocalize.h>
#include "vguicenterprint.h"
#include "vgui/keycode.h"
#include <KeyValues.h>
#include "ienginevgui.h"
#include "c_playerresource.h"
#include "ihudlcd.h"
#include "vgui/IInput.h"
#include "vgui/ILocalize.h"
#include "multiplay_gamerules.h"
//BG2 - Tjoppen - #includes
#include "hl2mp_gamerules.h"
#include "../game/server/bg2/vcomm.h"
//


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define CHAT_WIDTH_PERCENTAGE 0.6f

#ifndef _XBOX
ConVar hud_saytext_time( "hud_saytext_time", "12", 0 );
ConVar cl_showtextmsg( "cl_showtextmsg", "1", 0, "Enable/disable text messages printing on the screen." );
ConVar cl_chatfilters( "cl_chatfilters", "127", FCVAR_ARCHIVE, "Stores the chat filter settings " );

Color g_ColorBlue( 153, 204, 255, 255 );
Color g_ColorRed( 255, 63.75, 63.75, 255 );
Color g_ColorGreen( 153, 255, 153, 255 );
Color g_ColorDarkGreen( 64, 255, 64, 255 );
Color g_ColorYellow( 255, 178.5, 0.0, 255 );
Color g_ColorGrey( 204, 204, 204, 255 );

//--------------------------------------------------------------------------------------------------------
/**
* Simple utility function to allocate memory and duplicate a wide string
*/
#ifdef _WIN32
inline wchar_t *CloneWString( const wchar_t *str )
{
	wchar_t *cloneStr = new wchar_t [ wcslen(str)+1 ];
	wcscpy( cloneStr, str );
	return cloneStr;
}
#endif

// removes all color markup characters, so Msg can deal with the string properly
// returns a pointer to str
char* RemoveColorMarkup( char *str )
{
	char *out = str;
	for ( char *in = str; *in != 0; ++in )
	{
		if ( *in > 0 && *in < COLOR_MAX )
		{
			continue;
		}
		*out = *in;
		++out;
	}
	*out = 0;

	return str;
}

// converts all '\r' characters to '\n', so that the engine can deal with the properly
// returns a pointer to str
char* ConvertCRtoNL( char *str )
{
	for ( char *ch = str; *ch != 0; ch++ )
		if ( *ch == '\r' )
			*ch = '\n';
	return str;
}

// converts all '\r' characters to '\n', so that the engine can deal with the properly
// returns a pointer to str
wchar_t* ConvertCRtoNL( wchar_t *str )
{
	for ( wchar_t *ch = str; *ch != 0; ch++ )
		if ( *ch == L'\r' )
			*ch = L'\n';
	return str;
}

void StripEndNewlineFromString( char *str )
{
	int s = strlen( str ) - 1;
	if ( s >= 0 )
	{
		if ( str[s] == '\n' || str[s] == '\r' )
			str[s] = 0;
	}
}

void StripEndNewlineFromString( wchar_t *str )
{
	int s = wcslen( str ) - 1;
	if ( s >= 0 )
	{
		if ( str[s] == L'\n' || str[s] == L'\r' )
			str[s] = 0;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Reads a string from the current message and checks if it is translatable
//-----------------------------------------------------------------------------
wchar_t* ReadLocalizedString( bf_read &msg, wchar_t *pOut, int outSize, bool bStripNewline, char *originalString, int originalSize )
{
	char szString[2048];
	szString[0] = 0;
	msg.ReadString( szString, sizeof(szString) );

	if ( originalString )
	{
		Q_strncpy( originalString, szString, originalSize );
	}

	const wchar_t *pBuf = g_pVGuiLocalize->Find( szString );
	if ( pBuf )
	{
		wcsncpy( pOut, pBuf, outSize/sizeof(wchar_t) );
		pOut[outSize/sizeof(wchar_t)-1] = 0;
	}
	else
	{
		g_pVGuiLocalize->ConvertANSIToUnicode( szString, pOut, outSize );
	}

	if ( bStripNewline )
		StripEndNewlineFromString( pOut );

	return pOut;
}

//-----------------------------------------------------------------------------
// Purpose: Reads a string from the current message, converts it to unicode, and strips out color codes
//-----------------------------------------------------------------------------
wchar_t* ReadChatTextString( bf_read &msg, wchar_t *pOut, int outSize )
{
	char szString[2048];
	szString[0] = 0;
	msg.ReadString( szString, sizeof(szString) );

	g_pVGuiLocalize->ConvertANSIToUnicode( szString, pOut, outSize );

	StripEndNewlineFromString( pOut );

	// converts color control characters into control characters for the normal color
	for ( wchar_t *test = pOut; test && *test; ++test )
	{
		if ( *test && (*test < COLOR_MAX ) )
		{
			*test = COLOR_NORMAL;
		}
	}

	return pOut;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *parent - 
//			*panelName - 
//-----------------------------------------------------------------------------
CBaseHudChatLine::CBaseHudChatLine( vgui::Panel *parent, const char *panelName ) : 
	vgui::RichText( parent, panelName )
{
	m_hFont = m_hFontMarlett = 0;
	m_flExpireTime = 0.0f;
	m_flStartTime = 0.0f;
	m_iNameLength	= 0;
	m_text = NULL;

	SetPaintBackgroundEnabled( true );
	
	SetVerticalScrollbar( false );
}

CBaseHudChatLine::~CBaseHudChatLine()
{
	if ( m_text )
	{
		delete[] m_text;
		m_text = NULL;
	}
}
	

void CBaseHudChatLine::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_hFont = pScheme->GetFont( "Default" );

#ifdef HL1_CLIENT_DLL
	SetBgColor( Color( 0, 0, 0, 0 ) );
	SetFgColor( Color( 0, 0, 0, 0 ) );

	SetBorder( NULL );
#else
	SetBgColor( Color( 0, 0, 0, 100 ) );
#endif


	m_hFontMarlett = pScheme->GetFont( "Marlett" );

	m_clrText = pScheme->GetColor( "FgColor", GetFgColor() );
	SetFont( m_hFont );
}


void CBaseHudChatLine::PerformFadeout( void )
{
	// Flash + Extra bright when new
	float curtime = gpGlobals->curtime;

	int lr = m_clrText[0];
	int lg = m_clrText[1];
	int lb = m_clrText[2];
	
	if ( curtime >= m_flStartTime && curtime < m_flStartTime + CHATLINE_FLASH_TIME )
	{
		float frac1 = ( curtime - m_flStartTime ) / CHATLINE_FLASH_TIME;
		float frac = frac1;

		frac *= CHATLINE_NUM_FLASHES;
		frac *= 2 * M_PI;

		frac = cos( frac );

		frac = clamp( frac, 0.0f, 1.0f );

		frac *= (1.0f-frac1);

		int r = lr, g = lg, b = lb;

		r = r + ( 255 - r ) * frac;
		g = g + ( 255 - g ) * frac;
		b = b + ( 255 - b ) * frac;
	
		// Draw a right facing triangle in red, faded out over time
		int alpha = 63 + 192 * (1.0f - frac1 );
		alpha = clamp( alpha, 0, 255 );

		wchar_t wbuf[4096];
		GetText(0, wbuf, sizeof(wbuf));

		SetText( "" );

		InsertColorChange( Color( r, g, b, 255 ) );
		InsertString( wbuf );
	}
	else if ( curtime <= m_flExpireTime && curtime > m_flExpireTime - CHATLINE_FADE_TIME )
	{
		float frac = ( m_flExpireTime - curtime ) / CHATLINE_FADE_TIME;

		int alpha = frac * 255;
		alpha = clamp( alpha, 0, 255 );

		wchar_t wbuf[4096];
		GetText(0, wbuf, sizeof(wbuf));

		SetText( "" );

		InsertColorChange( Color( lr * frac, lg * frac, lb * frac, alpha ) );
		InsertString( wbuf );
	}
	else
	{
		wchar_t wbuf[4096];
		GetText(0, wbuf, sizeof(wbuf));

		SetText( "" );

		InsertColorChange( Color( lr, lg, lb, 255 ) );
		InsertString( wbuf );
	}

	OnThink();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : time - 
//-----------------------------------------------------------------------------
void CBaseHudChatLine::SetExpireTime( void )
{
	m_flStartTime = gpGlobals->curtime;
	m_flExpireTime = m_flStartTime + hud_saytext_time.GetFloat();
	m_nCount = CBaseHudChat::m_nLineCounter++;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseHudChatLine::GetCount( void )
{
	return m_nCount;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseHudChatLine::IsReadyToExpire( void )
{
	// Engine disconnected, expire right away
	if ( !engine->IsInGame() && !engine->IsConnected() )
		return true;

	if ( gpGlobals->curtime >= m_flExpireTime )
		return true;
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CBaseHudChatLine::GetStartTime( void )
{
	return m_flStartTime;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseHudChatLine::Expire( void )
{
	SetVisible( false );

	// Spit out label text now
//	char text[ 256 ];
//	GetText( text, 256 );

//	Msg( "%s\n", text );
}
#endif _XBOX

//-----------------------------------------------------------------------------
// Purpose: The prompt and text entry area for chat messages
//-----------------------------------------------------------------------------
#ifndef _XBOX
CBaseHudChatInputLine::CBaseHudChatInputLine( CBaseHudChat *parent, char const *panelName ) : 
	vgui::Panel( parent, panelName )
{
	SetMouseInputEnabled( false );

	m_pPrompt = new vgui::Label( this, "ChatInputPrompt", L"Enter text:" );

	m_pInput = new CBaseHudChatEntry( this, "ChatInput", parent );	
	m_pInput->SetMaximumCharCount( 127 );
}

void CBaseHudChatInputLine::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	
	// FIXME:  Outline
	vgui::HFont hFont = pScheme->GetFont( "ChatFont" );

	m_pPrompt->SetFont( hFont );
	m_pInput->SetFont( hFont );

	m_pInput->SetFgColor( pScheme->GetColor( "Chat.TypingText", pScheme->GetColor( "Panel.FgColor", Color( 255, 255, 255, 255 ) ) ) );

	SetPaintBackgroundEnabled( true );
	m_pPrompt->SetPaintBackgroundEnabled( true );
	m_pPrompt->SetContentAlignment( vgui::Label::a_west );
	m_pPrompt->SetTextInset( 2, 0 );

	m_pInput->SetMouseInputEnabled( true );

#ifdef HL1_CLIENT_DLL
	m_pInput->SetBgColor( Color( 255, 255, 255, 0 ) );
#endif

	SetBgColor( Color( 0, 0, 0, 0) );

}

void CBaseHudChatInputLine::SetPrompt( const wchar_t *prompt )
{
	Assert( m_pPrompt );
	m_pPrompt->SetText( prompt );
	InvalidateLayout();
}

void CBaseHudChatInputLine::ClearEntry( void )
{
	Assert( m_pInput );
	SetEntry( L"" );
}

void CBaseHudChatInputLine::SetEntry( const wchar_t *entry )
{
	Assert( m_pInput );
	Assert( entry );

	m_pInput->SetText( entry );
}

void CBaseHudChatInputLine::GetMessageText( wchar_t *buffer, int buffersizebytes )
{
	m_pInput->GetText( buffer, buffersizebytes);
}

void CBaseHudChatInputLine::PerformLayout()
{
	BaseClass::PerformLayout();

	int wide, tall;
	GetSize( wide, tall );

	int w,h;
	m_pPrompt->GetContentSize( w, h); 
	m_pPrompt->SetBounds( 0, 0, w, tall );

	m_pInput->SetBounds( w + 2, 0, wide - w - 2 , tall );
}

vgui::Panel *CBaseHudChatInputLine::GetInputPanel( void )
{
	return m_pInput;
}
#endif //_XBOX


CHudChatFilterButton::CHudChatFilterButton( vgui::Panel *pParent, const char *pName, const char *pText ) : 
BaseClass( pParent, pName, pText )
{
}

CHudChatFilterCheckButton::CHudChatFilterCheckButton( vgui::Panel *pParent, const char *pName, const char *pText, int iFlag ) : 
BaseClass( pParent, pName, pText )
{
	m_iFlag = iFlag;
}


CHudChatFilterPanel::CHudChatFilterPanel( vgui::Panel *pParent, const char *pName ) : BaseClass ( pParent, pName )
{
	SetParent( pParent );

	new CHudChatFilterCheckButton( this, "joinleave_button", "Sky is blue?", CHAT_FILTER_JOINLEAVE );
	new CHudChatFilterCheckButton( this, "namechange_button", "Sky is blue?", CHAT_FILTER_NAMECHANGE );
	new CHudChatFilterCheckButton( this, "publicchat_button", "Sky is blue?", CHAT_FILTER_PUBLICCHAT );
	new CHudChatFilterCheckButton( this, "servermsg_button", "Sky is blue?", CHAT_FILTER_SERVERMSG );
	new CHudChatFilterCheckButton( this, "teamchange_button", "Sky is blue?", CHAT_FILTER_TEAMCHANGE );
	new CHudChatFilterCheckButton( this, "voicecomm_button", "Sky is blue?", CHAT_FILTER_VOICECOMMS ); //BG2 - Voice Comm Text Filter? -HairyPotter
	new CHudChatFilterCheckButton( this, "classchange_button", "Sky is blue?", CHAT_FILTER_CLASSCHANGES ); //BG2 - Class Change Text Filter? -HairyPotter
}

void CHudChatFilterPanel::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	LoadControlSettings( "resource/UI/ChatFilters.res" );

	BaseClass::ApplySchemeSettings( pScheme );

	Color cColor = pScheme->GetColor( "DullWhite", GetBgColor() );
	SetBgColor( Color ( cColor.r(), cColor.g(), cColor.b(), CHAT_HISTORY_ALPHA ) );

	SetFgColor( pScheme->GetColor( "Blank", GetFgColor() ) );
}

void CHudChatFilterPanel::OnFilterButtonChecked( vgui::Panel *panel )
{
	CHudChatFilterCheckButton *pButton = dynamic_cast < CHudChatFilterCheckButton * > ( panel );

	if ( pButton && GetChatParent() && IsVisible() )
	{
		if ( pButton->IsSelected() )
		{
			GetChatParent()->SetFilterFlag( GetChatParent()->GetFilterFlags() | pButton->GetFilterFlag() );
		}
		else
		{
			GetChatParent()->SetFilterFlag( GetChatParent()->GetFilterFlags() & ~ pButton->GetFilterFlag() );
		}
	}
}

void CHudChatFilterPanel::SetVisible(bool state)
{
	if ( state == true )
	{
		for (int i = 0; i < GetChildCount(); i++)
		{
			CHudChatFilterCheckButton *pButton = dynamic_cast < CHudChatFilterCheckButton * > ( GetChild(i) );

			if ( pButton )
			{
				if ( cl_chatfilters.GetInt() & pButton->GetFilterFlag() )
				{
					pButton->SetSelected( true );
				}
				else
				{
					pButton->SetSelected( false );
				}
			}
		}
	}

	BaseClass::SetVisible( state );
}

void CHudChatFilterButton::DoClick( void )
{
	BaseClass::DoClick();

	CBaseHudChat *pChat = dynamic_cast < CBaseHudChat * > (GetParent() );

	if ( pChat )
	{
		pChat->GetChatInput()->RequestFocus();

		if ( pChat->GetChatFilterPanel() )
		{
			if ( pChat->GetChatFilterPanel()->IsVisible() )
			{
				pChat->GetChatFilterPanel()->SetVisible( false );
			}
			else
			{
				pChat->GetChatFilterPanel()->SetVisible( true );
				pChat->GetChatFilterPanel()->MakePopup();
				pChat->GetChatFilterPanel()->SetMouseInputEnabled( true );
			}
		}
	}
}

CHudChatHistory::CHudChatHistory( vgui::Panel *pParent, const char *panelName ) : BaseClass( pParent, "HudChatHistory" )
{
	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFileEx( enginevgui->GetPanel( PANEL_CLIENTDLL ), "resource/ChatScheme.res", "ChatScheme");
	SetScheme(scheme);

	InsertFade( -1, -1 );
}

void CHudChatHistory::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetFont( pScheme->GetFont( "ChatFont" ) );
	SetAlpha( 255 );
}

int CBaseHudChat::m_nLineCounter = 1;
//-----------------------------------------------------------------------------
// Purpose: Text chat input/output hud element
//-----------------------------------------------------------------------------
CBaseHudChat::CBaseHudChat( const char *pElementName )
: CHudElement( pElementName ), BaseClass( NULL, "HudChat" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFileEx( enginevgui->GetPanel( PANEL_CLIENTDLL ), "resource/ChatScheme.res", "ChatScheme" );
	SetScheme(scheme);

	g_pVGuiLocalize->AddFile( "resource/chat_%language%.txt" );

	m_nMessageMode = 0;

	vgui::ivgui()->AddTickSignal( GetVPanel() );

	// (We don't actually want input until they bring up the chat line).
	MakePopup();
	SetZPos( -30 );

	SetHiddenBits( HIDEHUD_CHAT );

	m_pFiltersButton = new CHudChatFilterButton( this, "ChatFiltersButton", "Filters" );

	if ( m_pFiltersButton )
	{
		m_pFiltersButton->SetScheme( scheme );
		m_pFiltersButton->SetVisible( true );
		m_pFiltersButton->SetEnabled( true );
		m_pFiltersButton->SetMouseInputEnabled( true );
		m_pFiltersButton->SetKeyBoardInputEnabled( false );
	}

	m_pChatHistory = new CHudChatHistory( this, "HudChatHistory" );

	CreateChatLines();
	CreateChatInputLine();
	GetChatFilterPanel();

	m_iFilterFlags = cl_chatfilters.GetInt();
}

void CBaseHudChat::CreateChatInputLine( void )
{
#ifndef _XBOX
	m_pChatInput = new CBaseHudChatInputLine( this, "ChatInputLine" );
	m_pChatInput->SetVisible( false );

	if ( GetChatHistory() )
	{
		GetChatHistory()->SetMaximumCharCount( 127 * 100 );
		GetChatHistory()->SetVisible( true );
	}
#endif
}

void CBaseHudChat::CreateChatLines( void )
{
#ifndef _XBOX
	m_ChatLine = new CBaseHudChatLine( this, "ChatLine1" );
	m_ChatLine->SetVisible( false );		
#endif
}


#define BACKGROUND_BORDER_WIDTH 20

CHudChatFilterPanel *CBaseHudChat::GetChatFilterPanel( void )
{
	if ( m_pFilterPanel == NULL )
	{
		m_pFilterPanel = new CHudChatFilterPanel( this, "HudChatFilterPanel"  );

		if ( m_pFilterPanel )
		{
			vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFileEx( enginevgui->GetPanel( PANEL_CLIENTDLL ), "resource/ChatScheme.res", "ChatScheme");

			m_pFilterPanel->SetScheme( scheme );
			m_pFilterPanel->InvalidateLayout( true, true );
			m_pFilterPanel->SetMouseInputEnabled( true );
			m_pFilterPanel->SetPaintBackgroundType( 2 );
			m_pFilterPanel->SetPaintBorderEnabled( true );
			m_pFilterPanel->SetVisible( false );
		}
	}

	return m_pFilterPanel;
}

void CBaseHudChat::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( "resource/UI/BaseChat.res" );

	BaseClass::ApplySchemeSettings( pScheme );

	SetPaintBackgroundType( 2 );
	SetPaintBorderEnabled( true );
	SetPaintBackgroundEnabled( true );

	SetKeyBoardInputEnabled( false );
	SetMouseInputEnabled( false );
	m_nVisibleHeight = 0;

#ifdef HL1_CLIENT_DLL
	SetBgColor( Color( 0, 0, 0, 0 ) );
	SetFgColor( Color( 0, 0, 0, 0 ) );
#endif

	Color cColor = pScheme->GetColor( "DullWhite", GetBgColor() );
	SetBgColor( Color ( cColor.r(), cColor.g(), cColor.b(), CHAT_HISTORY_ALPHA ) );

	GetChatHistory()->SetVerticalScrollbar( false );
}

void CBaseHudChat::Reset( void )
{
#ifndef HL1_CLIENT_DLL
	m_nVisibleHeight = 0;
	Clear();
#endif
}

#ifdef _XBOX
bool CBaseHudChat::ShouldDraw()
{
	// never think, never draw
	return false;
}
#endif

void CBaseHudChat::Paint( void )
{
#ifndef _XBOX
	if ( m_nVisibleHeight == 0 )
		return;
#endif
}

CHudChatHistory *CBaseHudChat::GetChatHistory( void )
{
	return m_pChatHistory;
}



void CBaseHudChat::Init( void )
{
	if ( IsXbox() )
		return;

	ListenForGameEvent( "hltv_chat" );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pszName - 
//			iSize - 
//			*pbuf - 
//-----------------------------------------------------------------------------
void CBaseHudChat::MsgFunc_SayText( bf_read &msg )
{
	char szString[256];

	int client = msg.ReadByte();
	msg.ReadString( szString, sizeof(szString) );
	bool bWantsToChat = msg.ReadByte();

	if ( bWantsToChat )
	{
		// print raw chat text
		ChatPrintf( client, CHAT_FILTER_NONE, "%s", szString );
	}
	else
	{
		// try to lookup translated string
		Printf( CHAT_FILTER_NONE, "%s", hudtextmessage->LookupString( szString ) );
	}

	CLocalPlayerFilter filter;
	C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "HudChat.Message" );

	Msg( "%s", szString );
}

int CBaseHudChat::GetFilterForString( const char *pString )
{
	if ( !Q_stricmp( pString, "#HL_Name_Change" ) ) 
	{
		return CHAT_FILTER_NAMECHANGE;
	}

	return CHAT_FILTER_NONE;
}


//-----------------------------------------------------------------------------
// Purpose: Reads in a player's Chat text from the server
//-----------------------------------------------------------------------------
void CBaseHudChat::MsgFunc_SayText2( bf_read &msg )
{
	// Got message during connection
	if ( !g_PR )
		return;

	int client = msg.ReadByte();
	bool bWantsToChat = msg.ReadByte();

	wchar_t szBuf[6][256];
	char untranslated_msg_text[256];
	wchar_t *msg_text = ReadLocalizedString( msg, szBuf[0], sizeof( szBuf[0] ), false, untranslated_msg_text, sizeof( untranslated_msg_text ) );

	// keep reading strings and using C format strings for subsituting the strings into the localised text string
	ReadChatTextString ( msg, szBuf[1], sizeof( szBuf[1] ) );		// player name
	ReadChatTextString ( msg, szBuf[2], sizeof( szBuf[2] ) );		// chat text
	ReadLocalizedString( msg, szBuf[3], sizeof( szBuf[3] ), true );
	ReadLocalizedString( msg, szBuf[4], sizeof( szBuf[4] ), true );

	g_pVGuiLocalize->ConstructString( szBuf[5], sizeof( szBuf[5] ), msg_text, 4, szBuf[1], szBuf[2], szBuf[3], szBuf[4] );

	char ansiString[512];
	g_pVGuiLocalize->ConvertUnicodeToANSI( ConvertCRtoNL( szBuf[5] ), ansiString, sizeof( ansiString ) );

	if ( bWantsToChat )
	{
		int iFilter = CHAT_FILTER_NONE;

		if ( client > 0 && (g_PR->GetTeam( client ) != g_PR->GetTeam( GetLocalPlayerIndex() )) )
		{
			iFilter = CHAT_FILTER_PUBLICCHAT;
		}

		// print raw chat text
		ChatPrintf( client, iFilter, "%s", ansiString );

		Msg( "%s\n", RemoveColorMarkup(ansiString) );

		CLocalPlayerFilter filter;
		C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "HudChat.Message" );
	}
	else
	{
		// print raw chat text
		ChatPrintf( client, GetFilterForString( untranslated_msg_text), "%s", ansiString );
	}
}

//-----------------------------------------------------------------------------
// Message handler for text messages
// displays a string, looking them up from the titles.txt file, which can be localised
// parameters:
//   byte:   message direction  ( HUD_PRINTCONSOLE, HUD_PRINTNOTIFY, HUD_PRINTCENTER, HUD_PRINTTALK )
//   string: message
// optional parameters:
//   string: message parameter 1
//   string: message parameter 2
//   string: message parameter 3
//   string: message parameter 4
// any string that starts with the character '#' is a message name, and is used to look up the real message in titles.txt
// the next (optional) one to four strings are parameters for that string (which can also be message names if they begin with '#')
//-----------------------------------------------------------------------------
void CBaseHudChat::MsgFunc_TextMsg( bf_read &msg )
{
	m_iFilterFlags = cl_chatfilters.GetInt(); //LAWL

	char szString[2048];
	int msg_dest = msg.ReadByte();

	wchar_t szBuf[5][256],
			outputBuf[256];

	for ( int i=0; i<5; ++i )
	{
		msg.ReadString( szString, sizeof(szString) );
		char *tmpStr = hudtextmessage->LookupString( szString, &msg_dest );
		const wchar_t *pBuf = g_pVGuiLocalize->Find( tmpStr );
		if ( pBuf )
		{
			// Copy pBuf into szBuf[i].
			int nMaxChars = sizeof( szBuf[i] ) / sizeof( wchar_t );
			wcsncpy( szBuf[i], pBuf, nMaxChars );
			szBuf[i][nMaxChars-1] = 0;
		}
		else
		{
			if ( i )
			{
				StripEndNewlineFromString( tmpStr );  // these strings are meant for subsitution into the main strings, so cull the automatic end newlines
			}
			g_pVGuiLocalize->ConvertANSIToUnicode( tmpStr, szBuf[i], sizeof(szBuf[i]) );
		}
	}

	if ( !cl_showtextmsg.GetInt() )
		return;

	g_pVGuiLocalize->ConstructString( outputBuf, sizeof(outputBuf), szBuf[0], 4, szBuf[1], szBuf[2], szBuf[3], szBuf[4] );

	int len;
	switch ( msg_dest )
	{
	case HUD_PRINTCENTER:
		internalCenterPrint->Print( ConvertCRtoNL( outputBuf ) );
		break;

	case HUD_PRINTNOTIFY:
		g_pVGuiLocalize->ConvertUnicodeToANSI( outputBuf, szString, sizeof(szString) );
		len = strlen( szString );
		if ( len && szString[len-1] != '\n' && szString[len-1] != '\r' )
		{
			Q_strncat( szString, "\n", sizeof(szString), 1 );
		}
		Msg( "%s", ConvertCRtoNL( szString ) );
		break;

	case HUD_PRINTTALK:
		g_pVGuiLocalize->ConvertUnicodeToANSI( outputBuf, szString, sizeof(szString) );
		len = strlen( szString );
		if ( len && szString[len-1] != '\n' && szString[len-1] != '\r' )
		{
			Q_strncat( szString, "\n", sizeof(szString), 1 );
		}
		Printf( CHAT_FILTER_NONE, "%s", ConvertCRtoNL( szString ) );
		Msg( "%s", ConvertCRtoNL( szString ) );
		break;

	//BG2 - So we can filter class changes. -HairyPotter
	case HUD_BG2CLASSCHANGE:
		g_pVGuiLocalize->ConvertUnicodeToANSI( outputBuf, szString, sizeof(szString) );
		len = strlen( szString );
		if ( len && szString[len-1] != '\n' && szString[len-1] != '\r' )
		{
			Q_strncat( szString, "\n", sizeof(szString), 1 );
		}
		Printf( CHAT_FILTER_CLASSCHANGES, "%s", ConvertCRtoNL( szString ) );
		Msg( "%s", ConvertCRtoNL( szString ) );
		break;
	//

	case HUD_PRINTCONSOLE:
		g_pVGuiLocalize->ConvertUnicodeToANSI( outputBuf, szString, sizeof(szString) );
		len = strlen( szString );
		if ( len && szString[len-1] != '\n' && szString[len-1] != '\r' )
		{
			Q_strncat( szString, "\n", sizeof(szString), 1 );
		}
		Msg( "%s", ConvertCRtoNL( szString ) );
		break;
	}
}

void CBaseHudChat::MsgFunc_BG2Events( bf_read &msg )
{
	if ( !cl_showtextmsg.GetInt() )
		return;

	char szString[2048];
	int msg_type = msg.ReadByte(),
		msg_dest = msg.ReadByte(); //BG2 - Used for BG2 localization. -HairyPotter

	wchar_t szBuf[2][256],
			outputBuf[256];

	for ( int i = 0; i < 2; ++i )
	{
		msg.ReadString( szString, sizeof(szString) );
		char *tmpStr = hudtextmessage->LookupString( szString, &msg_dest );
		const wchar_t *pBuf = g_pVGuiLocalize->Find( tmpStr );
		if ( pBuf )
		{
			// Copy pBuf into szBuf[i].
			int nMaxChars = sizeof( szBuf[i] ) / sizeof( wchar_t );
			wcsncpy( szBuf[i], pBuf, nMaxChars );
			szBuf[i][nMaxChars-1] = 0;
		}
		else
		{
			if ( i )
			{
				StripEndNewlineFromString( tmpStr );  // these strings are meant for subsitution into the main strings, so cull the automatic end newlines
			}
			g_pVGuiLocalize->ConvertANSIToUnicode( tmpStr, szBuf[i], sizeof(szBuf[i]) );
		}
	}

	switch ( msg_type )
	{
		case ROUND_DRAW:
			_snwprintf(outputBuf, sizeof( outputBuf ), g_pVGuiLocalize->Find( "#BG2_Round_Draw" ) );
			break;
		case MAP_DRAW:
			_snwprintf(outputBuf, sizeof( outputBuf ), g_pVGuiLocalize->Find( "#BG2_Map_Draw" ) );
			break;
		case DEFAULT_DRAW:
			_snwprintf(outputBuf, sizeof( outputBuf ), g_pVGuiLocalize->Find( "#BG2_Default_Draw" ) );
			break;
		case BRITISH_ROUND_WIN:
			_snwprintf(outputBuf, sizeof( outputBuf ), g_pVGuiLocalize->Find( "#BG2_British_Round_Win" ) );
			break;
		case AMERICAN_ROUND_WIN:
			_snwprintf(outputBuf, sizeof( outputBuf ), g_pVGuiLocalize->Find( "#BG2_American_Round_Win" ) );
			break;
		case BRITISH_MAP_WIN:
			_snwprintf(outputBuf, sizeof( outputBuf ), g_pVGuiLocalize->Find( "#BG2_British_Map_Win" ) );
			break;
		case AMERICAN_MAP_WIN:
			_snwprintf(outputBuf, sizeof( outputBuf ), g_pVGuiLocalize->Find( "#BG2_American_Map_Win" ) );
			break;
		case BRITISH_DEFAULT_WIN:
			_snwprintf(outputBuf, sizeof( outputBuf ), g_pVGuiLocalize->Find( "#BG2_British_Default_Win" ) );
			break;
		case AMERICAN_DEFAULT_WIN:
			_snwprintf(outputBuf, sizeof( outputBuf ), g_pVGuiLocalize->Find( "#BG2_American_Default_Win" ) );
			break;
		case CTF_DENY_CAPTURE:
			_snwprintf(outputBuf, sizeof( outputBuf ), g_pVGuiLocalize->Find( "#BG2_CTF_Deny_Capture" ) );
			break;
		case CTF_DENY_PICKUP:
			_snwprintf(outputBuf, sizeof( outputBuf ), g_pVGuiLocalize->Find( "#BG2_CTF_Deny_Pickup" ) );
			break;
		case CTF_CAPTURE:
			if ( !szBuf[0] || !szBuf[1] ) //Just to be safe
				break;
			_snwprintf(outputBuf, sizeof( outputBuf ), g_pVGuiLocalize->Find( "#BG2_CTF_Capture" ), szBuf[0], szBuf[1] );
			break;
		case CTF_PICKUP:
			if ( !szBuf[0] || !szBuf[1] ) //Just to be safe
				break;
			_snwprintf(outputBuf, sizeof( outputBuf ), g_pVGuiLocalize->Find( "#BG2_CTF_Pickup" ), szBuf[0], szBuf[1] );
			break;
		case CTF_DROP:
			if ( !szBuf[0] || !szBuf[1] ) //Just to be safe
				break;
			_snwprintf(outputBuf, sizeof( outputBuf ), g_pVGuiLocalize->Find( "#BG2_CTF_Drop" ), szBuf[0], szBuf[1] );
			break;
		case CTF_RETURNED:
			if ( !szBuf[0] ) //Just to be safe
				break;
			_snwprintf(outputBuf, sizeof( outputBuf ), g_pVGuiLocalize->Find( "#BG2_CTF_Returned" ), szBuf[0] );
			break;
	}

	int len;
	switch ( msg_dest )
	{
	case HUD_PRINTCENTER:
		internalCenterPrint->Print( ConvertCRtoNL( outputBuf ) );
		break;

	case HUD_PRINTTALK:
		g_pVGuiLocalize->ConvertUnicodeToANSI( outputBuf, szString, sizeof(szString) );
		len = strlen( szString );
		if ( len && szString[len-1] != '\n' && szString[len-1] != '\r' )
		{
			Q_strncat( szString, "\n", sizeof(szString), 1 );
		}
		Printf( CHAT_FILTER_NONE, "%s", ConvertCRtoNL( szString ) );
		Msg( "%s", ConvertCRtoNL( szString ) );
		break;
	}
}

void CBaseHudChat::MsgFunc_VoiceSubtitle( bf_read &msg )
{
	// Got message during connection
	if ( !g_PR )
		return;

	if ( !cl_showtextmsg.GetInt() )
		return;

	char szString[2048];
	char szPrefix[64];	//(Voice)
	wchar_t szBuf[128];

	int client = msg.ReadByte();
	int iMenu = msg.ReadByte();
	int iItem = msg.ReadByte();

	const char *pszSubtitle = "";

	CGameRules *pGameRules = GameRules();

	CMultiplayRules *pMultiRules = dynamic_cast< CMultiplayRules * >( pGameRules );

	Assert( pMultiRules );

	if ( pMultiRules )
	{
		pszSubtitle = pMultiRules->GetVoiceCommandSubtitle( iMenu, iItem );
	}

	SetVoiceSubtitleState( true );

	const wchar_t *pBuf = g_pVGuiLocalize->Find( pszSubtitle );
	if ( pBuf )
	{
		// Copy pBuf into szBuf[i].
		int nMaxChars = sizeof( szBuf ) / sizeof( wchar_t );
		wcsncpy( szBuf, pBuf, nMaxChars );
		szBuf[nMaxChars-1] = 0;
	}
	else
	{
		g_pVGuiLocalize->ConvertANSIToUnicode( pszSubtitle, szBuf, sizeof(szBuf) );
	}

	int len;
	g_pVGuiLocalize->ConvertUnicodeToANSI( szBuf, szString, sizeof(szString) );
	len = strlen( szString );
	if ( len && szString[len-1] != '\n' && szString[len-1] != '\r' )
	{
		Q_strncat( szString, "\n", sizeof(szString), 1 );
	}

	const wchar_t *pVoicePrefix = g_pVGuiLocalize->Find( "#Voice" );
	g_pVGuiLocalize->ConvertUnicodeToANSI( pVoicePrefix, szPrefix, sizeof(szPrefix) );
	
	ChatPrintf( client, CHAT_FILTER_NONE, "%c(%s) %s%c: %s", COLOR_PLAYERNAME, szPrefix, GetDisplayedSubtitlePlayerName( client ), COLOR_NORMAL, ConvertCRtoNL( szString ) );

	SetVoiceSubtitleState( false );
}

const char *CBaseHudChat::GetDisplayedSubtitlePlayerName( int clientIndex )
{
	return g_PR->GetPlayerName( clientIndex );
}

#ifndef _XBOX
static int __cdecl SortLines( void const *line1, void const *line2 )
{
	CBaseHudChatLine *l1 = *( CBaseHudChatLine ** )line1;
	CBaseHudChatLine *l2 = *( CBaseHudChatLine ** )line2;

	// Invisible at bottom
	if ( l1->IsVisible() && !l2->IsVisible() )
		return -1;
	else if ( !l1->IsVisible() && l2->IsVisible() )
		return 1;

	// Oldest start time at top
	if ( l1->GetStartTime() < l2->GetStartTime() )
		return -1;
	else if ( l1->GetStartTime() > l2->GetStartTime() )
		return 1;

	// Otherwise, compare counter
	if ( l1->GetCount() < l2->GetCount() )
		return -1;
	else if ( l1->GetCount() > l2->GetCount() )
		return 1;

	return 0;
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Allow inheriting classes to change this spacing behavior
//-----------------------------------------------------------------------------
int CBaseHudChat::GetChatInputOffset( void )
{
	return m_iFontHeight;
}

//-----------------------------------------------------------------------------
// Purpose: Do respositioning here to avoid latency due to repositioning of vgui
//  voice manager icon panel
//-----------------------------------------------------------------------------
void CBaseHudChat::OnTick( void )
{
#ifndef _XBOX
	m_nVisibleHeight = 0;

	CBaseHudChatLine *line = m_ChatLine;

	if ( line )
	{
		vgui::HFont font = line->GetFont();
		m_iFontHeight = vgui::surface()->GetFontTall( font ) + 2;

		// Put input area at bottom

		int iChatX, iChatY, iChatW, iChatH;
		int iInputX, iInputY, iInputW, iInputH;
		
		m_pChatInput->GetBounds( iInputX, iInputY, iInputW, iInputH );
		GetBounds( iChatX, iChatY, iChatW, iChatH );

		m_pChatInput->SetBounds( iInputX, iChatH - (m_iFontHeight * 1.75), iInputW, m_iFontHeight );

		//Resize the History Panel so it fits more lines depending on the screen resolution.
		int iChatHistoryX, iChatHistoryY, iChatHistoryW, iChatHistoryH;

		GetChatHistory()->GetBounds( iChatHistoryX, iChatHistoryY, iChatHistoryW, iChatHistoryH );

		iChatHistoryH = (iChatH - (m_iFontHeight * 2.25)) - iChatHistoryY;

		GetChatHistory()->SetBounds( iChatHistoryX, iChatHistoryY, iChatHistoryW, iChatHistoryH );
	}

	FadeChatHistory();

#endif
}

// Release build is crashing on long strings...sigh
#pragma optimize( "", off )

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : width - 
//			*text - 
//			textlen - 
// Output : int
//-----------------------------------------------------------------------------
int CBaseHudChat::ComputeBreakChar( int width, const char *text, int textlen )
{
#ifndef _XBOX
	CBaseHudChatLine *line = m_ChatLine;
	vgui::HFont font = line->GetFont();

	int currentlen = 0;
	int lastbreak = textlen;
	for (int i = 0; i < textlen ; i++)
	{
		char ch = text[i];

		if ( ch <= 32 )
		{
			lastbreak = i;
		}

		wchar_t wch[2];

		g_pVGuiLocalize->ConvertANSIToUnicode( &ch, wch, sizeof( wch ) );

		int a,b,c;

		vgui::surface()->GetCharABCwide(font, wch[0], a, b, c);
		currentlen += a + b + c;

		if ( currentlen >= width )
		{
			// If we haven't found a whitespace char to break on before getting
			//  to the end, but it's still too long, break on the character just before
			//  this one
			if ( lastbreak == textlen )
			{
				lastbreak = max( 0, i - 1 );
			}
			break;
		}
	}

	if ( currentlen >= width )
	{
		return lastbreak;
	}
	return textlen;
#else
	return 0;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *fmt - 
//			... - 
//-----------------------------------------------------------------------------
#pragma warning( push )
#pragma warning( disable: 4748 ) // /GS can not protect parameters and local variables from local buffer overrun because optimizations are disabled in function
void CBaseHudChat::Printf( int iFilter, const char *fmt, ... )
{
	va_list marker;
	char msg[4096];

	va_start(marker, fmt);
	Q_vsnprintf(msg, sizeof( msg), fmt, marker);
	va_end(marker);

	ChatPrintf( 0, iFilter, "%s", msg );
}
#pragma warning( pop )
#pragma optimize( "", on )

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseHudChat::StartMessageMode( int iMessageModeType )
{
#ifndef _XBOX
	m_nMessageMode = iMessageModeType;

	m_pChatInput->ClearEntry();

	if ( m_nMessageMode == MM_SAY )
	{
		m_pChatInput->SetPrompt( L"Say :" );
	}
	else
	{
		m_pChatInput->SetPrompt( L"Say (TEAM) :" );
	}
	
	if ( GetChatHistory() )
	{
		GetChatHistory()->SetMouseInputEnabled( true );
		GetChatHistory()->SetKeyBoardInputEnabled( false );
		GetChatHistory()->SetVerticalScrollbar( true );
		GetChatHistory()->ResetAllFades( true );
		GetChatHistory()->SetPaintBorderEnabled( true );
		GetChatHistory()->SetVisible( true );
	}

	vgui::SETUP_PANEL( this );
	SetKeyBoardInputEnabled( true );
	SetMouseInputEnabled( true );
	m_pChatInput->SetVisible( true );
	vgui::surface()->CalculateMouseVisible();
	m_pChatInput->RequestFocus();
	m_pChatInput->SetPaintBorderEnabled( true );
	m_pChatInput->SetMouseInputEnabled( true );

	//Place the mouse cursor near the text so people notice it.
	int x, y, w, h;
	GetChatHistory()->GetBounds( x, y, w, h );
	vgui::input()->SetCursorPos( x + ( w/2), y + (h/2) );

	m_flHistoryFadeTime = gpGlobals->curtime + CHAT_HISTORY_FADE_TIME;

	m_pFilterPanel->SetVisible( false );

	engine->ClientCmd_Unrestricted( "gameui_preventescapetoshow\n" );
		
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseHudChat::StopMessageMode( void )
{
#ifndef _XBOX

	engine->ClientCmd_Unrestricted( "gameui_allowescapetoshow\n" );

	SetKeyBoardInputEnabled( false );
	SetMouseInputEnabled( false );
	
	if ( GetChatHistory() )
	{
		GetChatHistory()->SetPaintBorderEnabled( false );
		GetChatHistory()->GotoTextEnd();
		GetChatHistory()->SetMouseInputEnabled( false );
		GetChatHistory()->SetVerticalScrollbar( false );
		GetChatHistory()->ResetAllFades( false, true, CHAT_HISTORY_FADE_TIME );
		GetChatHistory()->SelectNoText();
	}

	//Clear the entry since we wont need it anymore.
	m_pChatInput->ClearEntry();

	m_flHistoryFadeTime = gpGlobals->curtime + CHAT_HISTORY_FADE_TIME;
#endif
}


void CBaseHudChat::FadeChatHistory( void )
{
	float frac = ( m_flHistoryFadeTime -  gpGlobals->curtime ) / CHAT_HISTORY_FADE_TIME;

	int alpha = frac * CHAT_HISTORY_ALPHA;
	alpha = clamp( alpha, 0, CHAT_HISTORY_ALPHA );

	if ( alpha >= 0 )
	{
		if ( GetChatHistory() )
		{
			if ( IsMouseInputEnabled() )
			{
				SetAlpha( 255 );
				GetChatHistory()->SetBgColor( Color( 0, 0, 0, CHAT_HISTORY_ALPHA - alpha ) );
				m_pChatInput->GetPrompt()->SetAlpha( (CHAT_HISTORY_ALPHA*2) - alpha );
				m_pChatInput->GetInputPanel()->SetAlpha( (CHAT_HISTORY_ALPHA*2) - alpha );
				SetBgColor( Color( GetBgColor().r(), GetBgColor().g(), GetBgColor().b(), CHAT_HISTORY_ALPHA - alpha ) );
				m_pFiltersButton->SetAlpha( (CHAT_HISTORY_ALPHA*2) - alpha );
			}
			else
			{
				GetChatHistory()->SetBgColor( Color( 0, 0, 0, alpha ) );
				SetBgColor( Color( GetBgColor().r(), GetBgColor().g(), GetBgColor().b(), alpha ) );
			
				m_pChatInput->GetPrompt()->SetAlpha( alpha );
				m_pChatInput->GetInputPanel()->SetAlpha( alpha );
				m_pFiltersButton->SetAlpha( alpha );
			}
		}
	}
}

void CBaseHudChat::SetFilterFlag( int iFilter )
{
	m_iFilterFlags = iFilter;

	cl_chatfilters.SetValue( m_iFilterFlags );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Color CBaseHudChat::GetTextColorForClient( TextColor colorNum, int clientIndex )
{
	Color c;
	switch ( colorNum )
	{
	case COLOR_PLAYERNAME:
		c = GetClientColor( clientIndex );
	break;

	case COLOR_LOCATION:
		c = g_ColorDarkGreen;
		break;

	case COLOR_ACHIEVEMENT:
		{
			vgui::IScheme *pSourceScheme = vgui::scheme()->GetIScheme( vgui::scheme()->GetScheme( "SourceScheme" ) ); 
			if ( pSourceScheme )
			{
				c = pSourceScheme->GetColor( "SteamLightGreen", GetBgColor() );
			}
			else
			{
				c = g_ColorYellow;
			}
		}
		break;

	default:
		c = g_ColorYellow;
	}

	return Color( c[0], c[1], c[2], 255 );
}

//-----------------------------------------------------------------------------
Color CBaseHudChat::GetClientColor( int clientIndex )
{
	//BG2 --
	if ( clientIndex == 0 ) // console msg
	{
		return g_ColorYellow;
	}
	else if( g_PR )
	{
		switch ( g_PR->GetTeam( clientIndex ) )
		{
		case TEAM_AMERICANS	: return g_ColorBlue;
		case TEAM_BRITISH	: return g_ColorRed;
		default	: return g_ColorYellow;
		}
	}

	return g_ColorYellow;
	//
}

//-----------------------------------------------------------------------------
// Purpose: Parses a line of text for color markup and inserts it via Colorize()
//-----------------------------------------------------------------------------
void CBaseHudChatLine::InsertAndColorizeText( wchar_t *buf, int clientIndex )
{
	if ( m_text )
	{
		delete[] m_text;
		m_text = NULL;
	}
	m_textRanges.RemoveAll();

	m_text = CloneWString( buf );

	CBaseHudChat *pChat = dynamic_cast<CBaseHudChat*>(GetParent() );

	if ( pChat == NULL )
		return;

	//BG2 - This is all fucked up here. Thanks Valve. -HairyPotter
	//wchar_t *txt = m_text;
	//int lineLen = wcslen( m_text );
	/*if ( m_text[0] == COLOR_PLAYERNAME || m_text[0] == COLOR_LOCATION || m_text[0] == COLOR_NORMAL || m_text[0] == COLOR_ACHIEVEMENT )
	{
		while ( txt && *txt )
		{
			TextRange range;

			switch ( *txt )
			{
			case COLOR_PLAYERNAME:
			case COLOR_LOCATION:
			case COLOR_ACHIEVEMENT:
			case COLOR_NORMAL:
				{
					// save this start
					range.start = (txt-m_text) + 1;
					range.color = pChat->GetTextColorForClient( (TextColor)(*txt), clientIndex );
					range.end = lineLen;

					int count = m_textRanges.Count();
					if ( count )
					{
						m_textRanges[count-1].end = range.start - 1;
					}

					m_textRanges.AddToTail( range );
				}
				++txt;
				break;

			default:
				++txt;
			}
		}
	}*/

	if ( !m_textRanges.Count() && m_iNameLength > 0 /*&& m_text[0] == COLOR_USEOLDCOLORS*/ )
	{
		TextRange range;
		range.start = 0;
		range.end = m_iNameStart;
		range.color = pChat->GetTextColorForClient( COLOR_NORMAL, clientIndex );
		m_textRanges.AddToTail( range );

		range.start = m_iNameStart;
		range.end = m_iNameStart + m_iNameLength;
		range.color = pChat->GetTextColorForClient( COLOR_PLAYERNAME, clientIndex ); //Nada
		m_textRanges.AddToTail( range );

		range.start = range.end;
		range.end = wcslen( m_text );
		range.color = pChat->GetTextColorForClient( COLOR_NORMAL, clientIndex );
		m_textRanges.AddToTail( range );
	}

	if ( !m_textRanges.Count() )
	{
		TextRange range;
		range.start = 0;
		range.end = wcslen( m_text );
		range.color = pChat->GetTextColorForClient( COLOR_NORMAL, clientIndex );
		m_textRanges.AddToTail( range );
	}

	for ( int i=0; i<m_textRanges.Count(); ++i )
	{
		wchar_t * start = m_text + m_textRanges[i].start;
		if ( *start > 0 && *start < COLOR_MAX )
		{
			m_textRanges[i].start += 1;
		}
	}

	Colorize();
}

//-----------------------------------------------------------------------------
// Purpose: Inserts colored text into the RichText control at the given alpha
//-----------------------------------------------------------------------------
void CBaseHudChatLine::Colorize( int alpha )
{
	// clear out text
	SetText( "" );

	CBaseHudChat *pChat = dynamic_cast<CBaseHudChat*>(GetParent() );

	if ( pChat && pChat->GetChatHistory() )
	{	
		pChat->GetChatHistory()->InsertString( "\n" );
	}

	wchar_t wText[4096];
	Color color;
	for ( int i=0; i<m_textRanges.Count(); ++i )
	{
		wchar_t * start = m_text + m_textRanges[i].start;
		int len = m_textRanges[i].end - m_textRanges[i].start + 1;
		if ( len > 1 )
		{
			wcsncpy( wText, start, len );
			wText[len-1] = 0;
			color = m_textRanges[i].color;
			color[3] = alpha;
			InsertColorChange( color );
			InsertString( wText );

			CBaseHudChat *pChat = dynamic_cast<CBaseHudChat*>(GetParent() );

			if ( pChat && pChat->GetChatHistory() )
			{	
				pChat->GetChatHistory()->InsertColorChange( color );
				pChat->GetChatHistory()->InsertString( wText );
				pChat->GetChatHistory()->InsertFade( hud_saytext_time.GetFloat(), CHAT_HISTORY_IDLE_FADE_TIME );

				if ( i == m_textRanges.Count()-1 )
				{
					pChat->GetChatHistory()->InsertFade( -1, -1 );
				}
			}

		}
	}

	InvalidateLayout( true );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : CBaseHudChatLine
//-----------------------------------------------------------------------------
CBaseHudChatLine *CBaseHudChat::FindUnusedChatLine( void )
{
#ifndef _XBOX
	return m_ChatLine;
#else
	return NULL;
#endif
}

void CBaseHudChat::Send( void )
{
#ifndef _XBOX
	wchar_t szTextbuf[128];

	m_pChatInput->GetMessageText( szTextbuf, sizeof( szTextbuf ) );
	
	char ansi[128];
	g_pVGuiLocalize->ConvertUnicodeToANSI( szTextbuf, ansi, sizeof( ansi ) );

	int len = Q_strlen(ansi);

	/*
This is a very long string that I am going to attempt to paste into the cs hud chat entry and we will see if it gets cropped or not.
	*/

	// remove the \n
	if ( len > 0 &&
		ansi[ len - 1 ] == '\n' )
	{
		ansi[ len - 1 ] = '\0';
	}

	if( len > 0 )
	{
		char szbuf[144];	// more than 128
		Q_snprintf( szbuf, sizeof(szbuf), "%s \"%s\"", m_nMessageMode == MM_SAY ? "say" : "say_team", ansi );

		//engine->ClientCmd_Unrestricted(szbuf);
		engine->ClientCmd(szbuf); //BG2 - Hmm. -HairyPotter
	}
	
	m_pChatInput->ClearEntry();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : vgui::Panel
//-----------------------------------------------------------------------------
vgui::Panel *CBaseHudChat::GetInputPanel( void )
{
#ifndef _XBOX
	return m_pChatInput->GetInputPanel();
#else
	return NULL;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseHudChat::Clear( void )
{
#ifndef _XBOX
	// Kill input prompt
	StopMessageMode();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *newmap - 
//-----------------------------------------------------------------------------
void CBaseHudChat::LevelInit( const char *newmap )
{
	Clear();
}

void CBaseHudChat::LevelShutdown( void )
{
	Clear();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *fmt - 
//			... - 
//-----------------------------------------------------------------------------
void CBaseHudChat::ChatPrintf( int iPlayerIndex, int iFilter, const char *fmt, ... )
{
	m_iFilterFlags = cl_chatfilters.GetInt(); //LAWL

	va_list marker;
	char msg[4096];

	va_start(marker, fmt);
	Q_vsnprintf(msg, sizeof( msg), fmt, marker);
	va_end(marker);

	// Strip any trailing '\n'
	if ( strlen( msg ) > 0 && msg[ strlen( msg )-1 ] == '\n' )
	{
		msg[ strlen( msg ) - 1 ] = 0;
	}

	// Strip leading \n characters ( or notify/color signifiers ) for empty string check
	char *pmsg = msg;
	while ( *pmsg && ( *pmsg == '\n' || ( *pmsg > 0 && *pmsg < COLOR_MAX ) ) )
	{
		pmsg++;
	}

	if ( !*pmsg )
		return;

	// Now strip just newlines, since we want the color info for printing
	pmsg = msg;
	while ( *pmsg && ( *pmsg == '\n' ) )
		pmsg++;

	if ( !*pmsg )
		return;

	CBaseHudChatLine *line = (CBaseHudChatLine *)FindUnusedChatLine();
	if ( !line )
		line = (CBaseHudChatLine *)FindUnusedChatLine();

	if ( !line )
		return;

	if ( iFilter != CHAT_FILTER_NONE )
	{
		if ( !(iFilter & m_iFilterFlags ) )
			return;
	}

	if ( *pmsg < 32 )
		hudlcd->AddChatLine( pmsg + 1 );
	else
		hudlcd->AddChatLine( pmsg );

	line->SetText( "" );

	int iNameStart = 0;
	int iNameLength = 0;

	player_info_t sPlayerInfo;
	if ( iPlayerIndex == 0 )
	{
		Q_memset( &sPlayerInfo, 0, sizeof(player_info_t) );
		Q_strncpy( sPlayerInfo.name, "Console", sizeof(sPlayerInfo.name)  );	
	}
	else
	{
		engine->GetPlayerInfo( iPlayerIndex, &sPlayerInfo );
	}	

	int bufSize = (strlen( pmsg ) + 1 ) * sizeof(wchar_t);
	wchar_t *wbuf = static_cast<wchar_t *>( _alloca( bufSize ) );
	if ( wbuf )
	{
		Color clrNameColor = GetClientColor( iPlayerIndex );

		line->SetExpireTime();

		g_pVGuiLocalize->ConvertANSIToUnicode( pmsg, wbuf, bufSize);

		// find the player's name in the unicode string, in case there is no color markup
		const char *pName = sPlayerInfo.name;

		if ( pName )
		{
			wchar_t wideName[MAX_PLAYER_NAME_LENGTH];
			g_pVGuiLocalize->ConvertANSIToUnicode( pName, wideName, sizeof( wideName ) );

			const wchar_t *nameInString = wcsstr( wbuf, wideName );

			if ( nameInString )
			{
				iNameStart = (nameInString - wbuf);
				iNameLength = wcslen( wideName );
			}
		}

		line->SetNameStart( iNameStart );
		line->SetNameLength( iNameLength );

		line->InsertAndColorizeText( wbuf, iPlayerIndex );

	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseHudChat::FireGameEvent( IGameEvent *event )
{
#ifndef _XBOX
	const char *eventname = event->GetName();

	if ( Q_strcmp( "hltv_chat", eventname ) == 0 )
	{
		C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();

		if ( !player )
			return;
		
		ChatPrintf( player->entindex(), CHAT_FILTER_NONE, "(SourceTV) %s", event->GetString( "text" ) );
	}
#endif
}

//BG2 - Tjoppen - VoiceComm usermessage
void CBaseHudChat::MsgFunc_VoiceComm( bf_read &msg )
{
	m_iFilterFlags = cl_chatfilters.GetInt(); //LAWL

	int client		= msg.ReadByte();
	int commdata	= msg.ReadByte(),
		comm		= commdata & 31,
		isamerican	= commdata >> 5;
	int m_iClass	= msg.ReadByte();

	wchar_t string1[64];
	string1[0] = 0;

	player_info_t sPlayerInfo;
	engine->GetPlayerInfo( client, &sPlayerInfo );

	if ( !(CHAT_FILTER_VOICECOMMS & m_iFilterFlags ) ) //BG2 - Check to see if we should actually display this test for the client. -HairyPotter
			return;

	if( comm >= 0 && comm < NUM_VOICECOMMS )
	{
		//need to resolve unicode stuff which makes this a bit.. tricky
		char searchstring[32];

		//first build our search string
		//BG2 - quick hack for now. Fix this up later. - HairyPotter
		if ( !isamerican && m_iClass == CLASS_SKIRMISHER && ( comm + 1 == 19 ) ) //Native battlecry
			Q_snprintf( searchstring, sizeof searchstring, "#BG2_VoiceComm_N19" );
		else
			Q_snprintf( searchstring, sizeof searchstring, "#BG2_VoiceComm_%c%i", isamerican ? 'A' : 'B', comm + 1 );

		//look up our strings
		wchar_t *resolved = g_pVGuiLocalize->Find( searchstring ),				// "Yes"
				*prefix = g_pVGuiLocalize->Find( "#BG2_VoiceComm_Prefix" );	// "(Command)"
		
		if( !resolved || !prefix  )
			return;

		//This whole thing really is a hack but I got tired of playing Valve's mind games. -HairyPotter
		wchar_t voicecomm[512];
		char msg[512], name[512], chprefix[512], chresolved[512];

		g_pVGuiLocalize->ConvertUnicodeToANSI( resolved, chresolved, sizeof(chresolved) );
		g_pVGuiLocalize->ConvertUnicodeToANSI( prefix, chprefix, sizeof(chprefix) );

		Q_snprintf( name, 512, "%s %s", chprefix, sPlayerInfo.name); //(command): Playername
		Q_snprintf( msg, 512, "%s: %s", name, chresolved );//(command): Playername: "Retreat!"

		g_pVGuiLocalize->ConvertANSIToUnicode( msg, voicecomm, sizeof( voicecomm ) );
		//

		//now just use some code copied from ChatPrintf to print this in the chat
		//this because ChatPrintf doesn't take wchar_t*
		CBaseHudChatLine *line = (CBaseHudChatLine *)FindUnusedChatLine();
		if ( !line )
		{
			//ExpireOldest();
			line = (CBaseHudChatLine *)FindUnusedChatLine();
		}

		if ( !line )
			return;

		line->SetText( "" );

		line->SetExpireTime();

		// find the player's name in the unicode string, in case there is no color markup
		const char *pName = name;
		int iNameStart = 0;
		int iNameLength = 0;

		if ( pName )
		{
			wchar_t wideName[48]; //Used to be MAX_PLAYER_NAME_LENGTH (32), but forgot to compensate for the "(Command)" Characters -HairyPotter
			g_pVGuiLocalize->ConvertANSIToUnicode( pName, wideName, sizeof( wideName ) );

			const wchar_t *nameInString = wcsstr( voicecomm, wideName );

			if ( nameInString )
			{
				iNameStart = (nameInString - voicecomm);
				iNameLength = wcslen( wideName );
			}
		}

		line->SetNameStart( iNameStart );
		line->SetNameLength( iNameLength );

		line->InsertAndColorizeText( voicecomm, client );

		//play a sound..
		CLocalPlayerFilter filter;
		C_BaseEntity::EmitSound( filter, -1 /*SOUND_FROM_LOCAL_PLAYER*/, "HudChat.Message" );
	}
}