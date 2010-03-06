//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef CLASSMENU_H
#define CLASSMENU_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/IScheme.h>
#include <vgui/keycode.h>
#include <vgui/MouseCode.h>
#include <vgui_controls/HTML.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ToggleButton.h>
#include <vgui_controls/CheckButton.h>
#include "vgui_BitmapImage.h"
#include "vgui_bitmapbutton.h"
#include <vgui_controls/ImagePanel.h>
#include <imagemouseoverbutton.h>

#include <game/client/iviewport.h>

class KeyValues;

namespace vgui
{
	class TextEntry;
	class Button;
	class Panel;
	class ImagePanel;
	class ComboBox;
}

#define BLACK_BAR_COLOR	Color(0, 0, 0, 196)

class IBaseFileSystem;

//-----------------------------------------------------------------------------
// Purpose: the bottom bar panel, this is a seperate panel because it
// wants mouse input and the main window doesn't
//----------------------------------------------------------------------------
class CClassButton : public vgui::Button
{
public:
	DECLARE_CLASS_SIMPLE( CClassButton, vgui::Button);

	CClassButton(Panel *parent, const char *panelName, const char *text) : Button( parent, panelName, text ) { m_iCommand = 0; }

	void SetCommand( int command );
	void OnMousePressed(vgui::MouseCode code);
	void OnCursorEntered( void );
	void OnCursorExited( void ){ m_bMouseOver = false; };
	void PerformCommand( void );
	//Added functionality for images as buttons. -HairyPotter
	virtual void Paint();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void ApplySettings( KeyValues *inResourceData )
	{
		// Active Image
		delete [] BritishImage,
				  AmericanImage,
				  temp;

		BritishImage = AmericanImage = temp = NULL;

		temp = inResourceData->GetString( "BritishImage", "" );
		BritishImage = new char[ Q_strlen( temp ) + 1  ];
		Q_strncpy( (char *)BritishImage, temp, Q_strlen( temp ) + 1 );

		temp = inResourceData->GetString( "AmericanImage", "" );
		AmericanImage = new char[ Q_strlen( temp ) + 1 ];
		Q_strncpy( (char *)AmericanImage, temp, Q_strlen( temp ) + 1 );

		temp = inResourceData->GetString( "BritishMouseoverImage", "" );
		BritishMouseoverImage = new char[ Q_strlen( temp ) + 1  ];
		Q_strncpy( (char *)BritishMouseoverImage, temp, Q_strlen( temp ) + 1 );

		temp = inResourceData->GetString( "AmericanMouseoverImage", "" );
		AmericanMouseoverImage = new char[ Q_strlen( temp ) + 1  ];
		Q_strncpy( (char *)AmericanMouseoverImage, temp, Q_strlen( temp ) + 1 );
		
		BaseClass::ApplySettings( inResourceData );
	}
	//

private:
	int m_iCommand;

	const char *BritishImage,
			*AmericanImage,
			*BritishMouseoverImage,
			*AmericanMouseoverImage,
			*temp;

	bool m_bMouseOver;
};

class CTeamButton : public vgui::Button
{
public:
	DECLARE_CLASS_SIMPLE( CTeamButton, vgui::Button);

	CTeamButton(Panel *parent, const char *panelName, const char *text) : Button( parent, panelName, text ) { m_iCommand = 0; }

	void SetCommand( int command );
	void OnMousePressed(vgui::MouseCode code);
	void OnCursorEntered( void );
	void OnCursorExited( void ){ m_bMouseOver = false; };
	void PerformCommand( void );
	//Added functionality for images as buttons. -HairyPotter
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void Paint( void );
	virtual void ApplySettings( KeyValues *inResourceData )
	{
		// Active Image
		delete [] TeamImage,
				  TeamMouseoverImage;

		TeamImage = TeamMouseoverImage = NULL;

		int len;

		temp = inResourceData->GetString( "Image", "" );
		len = Q_strlen( temp ) + 1;
		TeamImage = new char[ len ];
		Q_strncpy( (char *)TeamImage, temp, len );

		temp = inResourceData->GetString( "MouseoverImage", "" );
		len = Q_strlen( temp ) + 1;
		TeamMouseoverImage = new char[ len ];
		Q_strncpy( (char *)TeamMouseoverImage, temp, len );

		BaseClass::ApplySettings( inResourceData );
	}
	//

private:
	int m_iCommand;

	const char *TeamImage,
			   *TeamMouseoverImage,
			   *temp;

	bool m_bMouseOver;

};

class COkayButton : public vgui::Button
{
public:
	DECLARE_CLASS_SIMPLE( COkayButton, vgui::Button );

	COkayButton(Panel *parent, const char *panelName, const char *text) : Button( parent, panelName, text ) { }

	void SetCommand( int command, bool ammo = false );
	void OnMousePressed(vgui::MouseCode code);
	void OnCursorEntered( void );
	void PerformCommand( void );
};

class CWeaponButton : public vgui::ToggleButton
{
public:
	DECLARE_CLASS_SIMPLE( CWeaponButton, vgui::ToggleButton );

	CWeaponButton(Panel *parent, const char *panelName, const char *text) : ToggleButton( parent, panelName, text ) {  }

	void OnMousePressed(vgui::MouseCode code);
	void OnCursorEntered( void ){ m_bMouseOver = true; };
	void OnCursorExited( void ){ m_bMouseOver = false; };
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	void SetImage( const char *ImageName );
	virtual void Paint( void );
	virtual void ApplySettings( KeyValues *inResourceData )
	{
		Q_strncpy( AInf1, (const char *)inResourceData->GetString( "AInfImage1", "" ), 80 );
		Q_strncpy( AInf2, (const char *)inResourceData->GetString( "AInfImage2", "" ), 80 );
		Q_strncpy( AInf3, (const char *)inResourceData->GetString( "AInfImage3", "" ), 80 );
		Q_strncpy( BInf1, (const char *)inResourceData->GetString( "BInfImage1", "" ), 80 );
		Q_strncpy( BInf2, (const char *)inResourceData->GetString( "BInfImage2", "" ), 80 );
		Q_strncpy( ASki1, (const char *)inResourceData->GetString( "ASkiImage1", "" ), 80 );
		Q_strncpy( ASki2, (const char *)inResourceData->GetString( "ASkiImage2", "" ), 80 );
		Q_strncpy( BSki1, (const char *)inResourceData->GetString( "BSkiImage1", "" ), 80 );
		Q_strncpy( BSki2, (const char *)inResourceData->GetString( "BSkiImage2", "" ), 80 );
		Q_strncpy( BLight1, (const char *)inResourceData->GetString( "BLightImage1", "" ), 80 );

		//Mouse Overs
		Q_strncpy( AInf1M, (const char *)inResourceData->GetString( "AInfMouseImage1", "" ), 80 );
		Q_strncpy( AInf2M, (const char *)inResourceData->GetString( "AInfMouseImage2", "" ), 80 );
		Q_strncpy( AInf3M, (const char *)inResourceData->GetString( "AInfMouseImage3", "" ), 80 );
		Q_strncpy( BInf1M, (const char *)inResourceData->GetString( "BInfMouseImage1", "" ), 80 );
		Q_strncpy( BInf2M, (const char *)inResourceData->GetString( "BInfMouseImage2", "" ), 80 );
		Q_strncpy( ASki1M, (const char *)inResourceData->GetString( "ASkiMouseImage1", "" ), 80 );
		Q_strncpy( ASki2M, (const char *)inResourceData->GetString( "ASkiMouseImage2", "" ), 80 );
		Q_strncpy( BSki1M, (const char *)inResourceData->GetString( "BSkiMouseImage1", "" ), 80 );
		Q_strncpy( BSki2M, (const char *)inResourceData->GetString( "BSkiMouseImage2", "" ), 80 );
		Q_strncpy( BLight1M, (const char *)inResourceData->GetString( "BLightMouseImage1", "" ), 80 );
		//

		BaseClass::ApplySettings( inResourceData );
	}

	char BLight1[80], BInf1[80], BInf2[80], AInf1[80], AInf2[80], AInf3[80], ASki1[80], ASki2[80],
		 BSki1[80], BSki2[80], BLight1M[80], BInf1M[80], BInf2M[80], AInf1M[80], AInf2M[80], AInf3M[80], ASki1M[80], ASki2M[80],
		 BSki1M[80], BSki2M[80];

	bool m_bMouseOver;

	vgui::IImage *m_pImage;
};

class CAmmoButton : public vgui::ToggleButton
{
public:
	DECLARE_CLASS_SIMPLE( CAmmoButton, vgui::ToggleButton );

	CAmmoButton(Panel *parent, const char *panelName, const char *text) : ToggleButton( parent, panelName, text ) {  }

	void OnMousePressed(vgui::MouseCode code);
	void OnCursorEntered( void ){ m_bMouseOver = true; };
	void OnCursorExited( void ){ m_bMouseOver = false; };
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void Paint( void );
	virtual void ApplySettings( KeyValues *inResourceData )
	{
		// Active Image
		delete [] Image,
				  RestrictedImage,
				  MouseoverImage;

		Image = MouseoverImage = RestrictedImage = NULL;

		temp = inResourceData->GetString( "Image", "" );
		Image = new char[ Q_strlen( temp ) + 1 ];
		Q_strncpy( (char *)Image, temp, Q_strlen( temp ) + 1 );

		temp = inResourceData->GetString( "MouseoverImage", "" );
		MouseoverImage = new char[ Q_strlen( temp ) + 1 ];
		Q_strncpy( (char *)MouseoverImage, temp, Q_strlen( temp ) + 1 );

		temp = inResourceData->GetString( "RestrictedImage", "" );
		RestrictedImage = new char[ Q_strlen( temp ) + 1 ];
		Q_strncpy( (char *)RestrictedImage, temp, Q_strlen( temp ) + 1 );

		BaseClass::ApplySettings( inResourceData );
	}

	const char *Image,
			   *MouseoverImage,
			   *RestrictedImage,
			   *temp;

	bool m_bMouseOver, m_bRestricted;
};

class CClassMenu : public vgui::Frame, public IViewPortPanel
{
	DECLARE_CLASS_SIMPLE( CClassMenu, vgui::Frame );

public:
	CClassMenu( IViewPort *pViewPort );
	~CClassMenu();

	virtual const char *GetName( void ) { return PANEL_CLASSES; }
	virtual void SetData(KeyValues *data);
	virtual void Reset( void ) { }//m_pPlayerList->DeleteAllItems(); }
	virtual void Update( void );
	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }
	virtual void ShowPanel( bool bShow );
	virtual void Paint( void );
	virtual void ApplySettings( KeyValues *inResourceData )
	{
		// Active Image
		delete [] BGImage;

		BGImage = NULL;

		const char *image = inResourceData->GetString( "BackgroundImage", "" );
		int len = Q_strlen( image ) + 1;
		BGImage = new char[ len ];
		Q_strncpy( (char *)BGImage, image, len );
		
		BaseClass::ApplySettings( inResourceData );
	}


	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
	virtual void SetParent(vgui::VPANEL parent) { BaseClass::SetParent(parent); }

	CClassButton	*m_pInfantryButton,
					*m_pOfficerButton,
					*m_pSkirmisherButton,
					*m_pSniperButton,
					*m_pLightInfantryButton;

	CTeamButton		*m_pBritishButton,
					*m_pAmericanButton,
					*m_pSpectateButton,
					*m_pAutoassignButton;

	CWeaponButton	*m_pWeaponButton1,
					*m_pWeaponButton2,
					*m_pWeaponButton3;

	CAmmoButton		*m_pAmmoButton1,
					*m_pAmmoButton2;

	vgui::ImagePanel *m_pWeaponSelection,
					 *m_pAmmoSelection;

	vgui::Label *m_pBritishLabel,
				*m_pAmericanLabel,
				*m_pInfantryLabel,
				*m_pOfficerLabel,
				*m_pRiflemanLabel,
				*m_pSkirmisherLabel,
				*m_pLightInfantryLabel;

	vgui::CheckButton		*m_pQuickJoinCheckButton;

	COkayButton		*m_pOK;

	vgui::HTML		*m_pInfoHTML;

	int m_iTeamSelection,
		m_iClassSelection;

	const char *BGImage;

	void OnThink();
	void UpdateClassLabelText( vgui::Label *pLabel, int iClass);
	void UpdateWeaponButtonImages( void );
	void UpdateAmmoButtons( void );
	void ToggleButtons(int iShowScreen);

	//being in classmenu mode means we must be visible aswell
	//Kind of a hack here with the weapon kit selection. I mean.. there's always going to be at least 1 ammo selection.. so yeah..
	bool IsInClassMenu( void ) { return m_pInfantryButton->IsVisible() && IsVisible() || m_pAmmoButton1->IsVisible(); }

	void ShowFile( const char *filename );

private:
	// VGUI2 overrides
	virtual void OnKeyCodePressed(vgui::KeyCode code);
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void PerformLayout();

	void SetViewModeText( const char *text ) { }//m_pViewOptions->SetText( text ); }

	void SetPlayerFgColor( Color c1 ) { }//m_pPlayerList->SetFgColor(c1); }

	IViewPort *m_pViewPort;

	int				m_iInfantryKey,
					m_iOfficerKey,
					m_iSniperKey,
					m_iSkirmisherKey,
					m_iLightInfantryKey,
					m_iSlot6Key,
					m_iOkayKey;

	CClassButton	*m_pCancelButton;
	int				m_iCancelKey,

					teammenu,
					classmenu,
					weaponmenu,
					commmenu,
					commmenu2;
public:
};

#endif // CLASSMENU_H
