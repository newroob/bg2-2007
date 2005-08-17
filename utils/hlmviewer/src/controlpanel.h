//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           ControlPanel.h
// last modified:  May 29 programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
// version:        1.2
//
// email:          mete@swissquake.ch
// web:            http://www.swissquake.ch/chumbalum-soft/
//
#ifndef INCLUDED_CONTROLPANEL
#define INCLUDED_CONTROLPANEL



#ifndef INCLUDED_MXWINDOW
#include <mx/mxWindow.h>
#endif


#define IDC_TAB						1901
#define IDC_RENDERMODE				2001
#define IDC_GROUND					2003
#define IDC_MOVEMENT				2004
#define IDC_BACKGROUND				2005
#define IDC_HITBOXES				2006
#define IDC_BONES					2007
#define IDC_ATTACHMENTS				2008
#define IDC_PHYSICSMODEL			2009
#define IDC_PHYSICSHIGHLIGHT		2010
#define IDC_LODCHOICE				2011
#define IDC_AUTOLOD					2012
#define IDC_LODSWITCH				2013
#define IDC_SOFTWARESKIN			2014
#define IDC_OVERBRIGHT2				2015
#define IDC_RENDER_FOV				2016
#define IDC_SEQUENCEBOXES			2017
#define IDC_RUNIK					2018
#define IDC_HEADTURN				2019
#define IDC_NORMALS					2020
#define IDC_NORMALMAP				2021
#define IDC_PARALLAXMAP				2022
#define IDC_SPECULAR				2023
#define IDC_SHADOW					2024
#define IDC_ILLUMPOSITION			2025

#define MAX_SEQUENCES				5
#define IDC_SEQUENCE0				3000
#define IDC_SEQUENCE1				3001
#define IDC_SEQUENCE2				3002
#define IDC_SEQUENCE3				3003
#define IDC_SEQUENCE4				3004

#define IDC_SEQUENCESCALE0			3005
#define IDC_SEQUENCESCALE1			3006
#define IDC_SEQUENCESCALE2			3007
#define IDC_SEQUENCESCALE3			3008
#define IDC_SEQUENCESCALE4			3009

#define IDC_FRAMESELECTION0			3010
#define IDC_FRAMESELECTION1			3011
#define IDC_FRAMESELECTION2			3012
#define IDC_FRAMESELECTION3			3013
#define IDC_FRAMESELECTION4			3014

#define NUM_POSEPARAMETERS			8
#define IDC_POSEPARAMETER_SCALE		3100
#define IDC_POSEPARAMETER			3150

#define IDC_SPEEDSCALE				3201
#define IDC_FORCEFRAME				3202
#define IDC_BLENDSEQUENCECHANGES	3203
#define IDC_BLENDNOW				3204
#define IDC_BLENDTIME				3205

#define IDC_BODYPART				4001
#define IDC_SUBMODEL				4002
#define IDC_CONTROLLER				4003
#define IDC_CONTROLLERVALUE			4004
#define IDC_SKINS					4005

#define IDC_BONE_BONELIST			5000
#define IDC_BONE_GENERATEQC			5001
#define IDC_BONE_HIGHLIGHT_BONE		5002
#define IDC_BONE_HITBOXLIST			5003
#define IDC_BONE_SURFACEPROP		5004
#define IDC_BONE_HIGHLIGHT_HITBOX	5005
#define IDC_BONE_ADD_HITBOX			5006
#define IDC_BONE_DELETE_HITBOX		5007
#define IDC_BONE_APPLY_TO_CHILDREN	5008
#define IDC_BONE_SHOW_DEFAULT_POSE	5009
#define IDC_BONE_HITBOX_ORIGINX		5010
#define IDC_BONE_HITBOX_ORIGINY		5011
#define IDC_BONE_HITBOX_ORIGINZ		5012
#define IDC_BONE_HITBOX_SIZEX		5013
#define IDC_BONE_HITBOX_SIZEY		5014
#define IDC_BONE_HITBOX_SIZEZ		5015
#define IDC_BONE_HITBOX_GROUP		5016
#define IDC_BONE_UPDATE_HITBOX		5017
#define IDC_BONE_USE_AUTOGENERATED_HITBOXES	5018
#define IDC_BONE_HITBOXSET			5019
#define IDC_BONE_HITBOXADDSET		5020
#define IDC_BONE_HITBOXDELETESET	5021
#define IDC_BONE_HITBOXSETNAME		5022
#define IDC_BONE_HITBOXSETNAME_EDIT	5023

// This range is reserved for the attachment window.
#define IDC_ATTACHMENT_WINDOW_FIRST	5024
#define IDC_ATTACHMENT_WINDOW_LAST	5100
#define IDC_BONE_HITBOX_NAME		5101

#define IDC_FLEX					7001
#define IDC_FLEXSCALE				7101

#define NUM_FLEX_SLIDERS			32

#define IDC_PHYS_FIRST				7501
#define IDC_PHYS_BONE				7501
#define IDC_PHYS_CON_LINK_LIMITS	7502
#define IDC_PHYS_MATERIAL			7503
#define IDC_PHYS_CON_MIN			7504
#define IDC_PHYS_CON_MAX			7505
#define IDC_PHYS_CON_TEST			7506
#define IDC_PHYS_P_MASSBIAS			7507
#define IDC_PHYS_CON_FRICTION		7508
//#define IDC_PHYS_P_ELASTICITY		7509
#define IDC_PHYS_P_INERTIA			7510
#define IDC_PHYS_P_DAMPING			7511
#define IDC_PHYS_P_ROT_DAMPING		7512
#define IDC_PHYS_MASS				7513
#define IDC_PHYS_QCFILE				7514
#define IDC_PHYS_CON_AXIS_X			7515
#define IDC_PHYS_CON_AXIS_Y			7516
#define IDC_PHYS_CON_AXIS_Z			7517
#define IDC_PHYS_CON_TYPE_FREE		7518
#define IDC_PHYS_CON_TYPE_FIXED		7519
#define IDC_PHYS_CON_TYPE_LIMIT		7520
#define IDC_PHYS_LAST				7599

#define MAX_ANIMS					4
#define IDC_ANIMX					8020 // through 8023 ( MAX_ANIMS )
#define IDC_ANIMY					8030 // through 8033 ( MAX_ANIMS )

class mxTab;
class mxChoice;
class mxCheckBox;
class mxSlider;
class mxLineEdit;
class mxLabel;
class mxButton;
class mxRadioButton;
class MatSysWindow;
class TextureWindow;
class CBoneControlWindow;
class CAttachmentsWindow;


// Return codes from loadModel.
enum LoadModelResult_t
{
	LoadModel_Success = 0,
	LoadModel_LoadFail,
	LoadModel_PostLoadFail,
};


class ControlPanel : public mxWindow
{
	mxWindow *wRender;
	mxTab *tab;
	mxChoice *cRenderMode;
	mxChoice *cHighlightBone;

	mxCheckBox *cbGround;
	mxCheckBox *cbHitBoxes;
	mxCheckBox *cbSequenceBoxes;
	mxCheckBox *cbShadow;
	mxCheckBox *cbMovement;
	mxCheckBox *cbBackground;
	mxCheckBox *cbSoftwareSkin;
	mxCheckBox *cbOverbright2;
	mxCheckBox *cbAttachments;
	mxCheckBox *cbBones;
	mxCheckBox *cbNormals;
	mxCheckBox *cbNormalMap;
	mxCheckBox *cbParallaxMap;
	mxCheckBox *cbSpecular;
	mxCheckBox *cbRunIK;
	mxCheckBox *cbEnableHead;
	mxCheckBox *cbIllumPosition;

	mxChoice *cLODChoice;
	mxCheckBox *cbAutoLOD;
	mxLineEdit *leLODSwitch;
	mxLabel *lLODMetric;
	mxChoice *cSequence[MAX_SEQUENCES];
	mxSlider *slSequence[MAX_SEQUENCES];
	int		iSelectionToSequence[2048]; // selection to sequence
	int		iSequenceToSelection[2048]; // sequence to selection
	mxLabel *laGroundSpeed;
	mxSlider *slSpeedScale;
	mxLabel *laFPS;
	mxLabel *laBlendAmount;

	mxChoice *cPoseParameter[NUM_POSEPARAMETERS];
	mxSlider *slPoseParameter[NUM_POSEPARAMETERS];
	mxLabel *laPoseParameter[NUM_POSEPARAMETERS];
	mxLineEdit *leFOV;

	mxSlider *slBlendTime;
	mxLabel *laBlendTime;
	mxSlider *slForceFrame;
	mxLabel	*lForcedFrame;
	mxRadioButton *rbFrameSelection[MAX_SEQUENCES];
	mxChoice *cBodypart, *cController, *cSubmodel;
	mxSlider *slController;
	mxChoice *cSkin;
	mxLabel *lModelInfo1, *lModelInfo2, *lModelInfo3, *lModelInfo4;
	//mxChoice *cTextures;
	//mxCheckBox *cbChrome;
	//mxLabel *lTexSize;
	//mxLineEdit *leWidth, *leHeight;

	mxLineEdit *leMeshScale, *leBoneScale;

	MatSysWindow *d_MatSysWindow;
	TextureWindow *d_textureWindow;

	mxChoice *cFlex[NUM_FLEX_SLIDERS];
	mxSlider *slFlexScale[NUM_FLEX_SLIDERS];

	mxChoice *cPhysicsBone;
	mxRadioButton *rbConstraintAxis[3];
	mxSlider *slPhysicsFriction;
	mxLabel	 *lPhysicsFriction;

	mxSlider *slPhysicsConMin;
	mxLabel	 *lPhysicsConMin;
	mxCheckBox *cbLinked;		// links min/max sliders

	mxSlider *slPhysicsConMax;
	mxLabel	 *lPhysicsConMax;
	mxSlider *slPhysicsConTest;
	mxLineEdit *leMass;

	mxSlider *slPhysicsParamMassBias;
	mxLabel	 *lPhysicsParamMassBias;
	mxSlider *slPhysicsParamFriction;
	mxLabel	 *lPhysicsParamFriction;
	mxSlider *slPhysicsParamElasticity;
	mxLabel	 *lPhysicsParamElasticity;
	mxSlider *slPhysicsParamInertia;
	mxLabel	 *lPhysicsParamInertia;
	mxSlider *slPhysicsParamDamping;
	mxLabel	 *lPhysicsParamDamping;
	mxSlider *slPhysicsParamRotDamping;
	mxLabel	 *lPhysicsParamRotDamping;
	mxLabel  *lPhysicsMaterial;

	CBoneControlWindow* m_pBoneWindow;
	CAttachmentsWindow* m_pAttachmentsWindow;

public:
	// CREATORS
	ControlPanel (mxWindow *parent);
	virtual ~ControlPanel ();

	// MANIPULATORS
	int handleEvent (mxEvent *event);

	int handlePhysicsEvent( mxEvent *event );
	void UpdateConstraintSliders( int clamp );
	void setupPhysics( void );
	void setupPhysicsBone( int boneIndex );
	void setupPhysicsAxis( int boneIndex, int axis );
	int getPhysicsAxis( void );
	void setPhysicsAxis( int axisIndex );
	void writePhysicsData( void );
	void handlePhysicsKey( mxEvent *event );
//	void readPhysicsMaterials( mxChoice *pList );

	void dumpModelInfo ();
	LoadModelResult_t loadModel(const char *filename);
	LoadModelResult_t loadModel(const char *filename, int slot );

	void resetControlPanel( void );
	void setRenderMode (int mode);
	void setShowGround (bool b);
	void setShowMovement (bool b);
	void setShowBackground (bool b);
	void setHighlightBone( int index );
	void setLOD( int index, bool setLODchoice, bool force );
	void setAutoLOD( bool b );
	void setSoftwareSkin( bool b );
	void setOverbright( bool b );
	void setLODMetric( float metric );
	void setPolycount( int polycount );
	void setTransparent( bool isTransparent );
	void updatePoseParameters( void );
	void setFOV( float fov );

	void initSequenceChoices();
	void setSequence(int index);
	void updateGroundSpeed( void );
	void setOverlaySequence(int num, int index, float weight);
	void updateTransitionAmount();
	void startBlending( void );
	void setSpeedScale ( float scale );
	void updateSpeedScale( void );
	void setBlend(int index, float value );

	int	getFrameSelection( void );
	void setFrame( float frame );
	void updateFrameSelection( void );
	void updateFrameSlider( void );

	void initBodypartChoices();
	void setBodypart (int index);
	void setSubmodel (int index);

	void initBoneControllers ();
	void setBoneController (int index);
	void setBoneControllerValue (int index, float value);

	void initSkinChoices();

	void setModelInfo ();

	void initPhysicsBones();
	
	void initLODs();

	void centerView ();
	void viewmodelView();

	void fullscreen ();

	void setMatSysWindow (MatSysWindow *window) { d_MatSysWindow = window; }

	void initFlexes ();

	int		GetCurrentHitboxSet( void );
public:
	// Sets up the main tabs
	void SetupBoneControlWindow( mxTab* pTab );
	void SetupBodyWindow( mxTab* pTab );
	void SetupFlexWindow( mxTab* pTab );
	void SetupPhysicsWindow( mxTab* pTab );
	void SetupAttachmentsWindow( mxTab *pTab );
};


extern ControlPanel *g_ControlPanel;



#endif // INCLUDED_CONTROLPANEL