#include "cbase.h"
#include "baseentity.h"
#include "mapfilter.h"
 
// Constructor
CMapEntityFilter::CMapEntityFilter() 
{
	keepList			= new CUtlSortVector<const char *>( StrLessThan );
	keepTargetnameList	= new CUtlSortVector<const char *>( StrLessThan );
	
	//BG2 - Tjoppen - mapfilter_excluder related logic. also make sure we keep them by adding to the list
	AddKeep( "mapfilter_excluder" );

	CMapEntityFilterExcluder *pExcluder = NULL;

	while( (pExcluder = dynamic_cast<CMapEntityFilterExcluder*>(
			gEntList.FindEntityByClassname( pExcluder, "mapfilter_excluder") )) != NULL )
	{
		if( pExcluder->m_bExcludeByTargetName )
			AddTargetnameKeep( STRING( pExcluder->m_sClassOrTargetName ) );
		else
			AddKeep( STRING( pExcluder->m_sClassOrTargetName ) );
	}
	
}
 
// Deconstructor
CMapEntityFilter::~CMapEntityFilter() 
{
	delete keepList;
}
 
// [bool] ShouldCreateEntity [char]
// Purpose   : Used to check if the passed in entity is on our stored list
// Arguments : The classname of an entity 
// Returns   : Boolean value - if we have it stored, we return false.
 
bool CMapEntityFilter::ShouldCreateEntity( const char *pClassname ) 
{
	//Check if the entity is in our keep list.
	if( keepList->Find( pClassname ) >= 0 )
		return false;
	else
		return true;
}

//BG2 - Tjoppen - targetname version
bool CMapEntityFilter::ShouldCreateEntity( const char *pClassname, const char *pTargetname ) 
{
	//BG2 - Tjoppen - check both lists
	if( (pClassname && keepList->Find( pClassname ) >= 0) || 
			(pTargetname && keepTargetnameList->Find( pTargetname ) >= 0) )
		return false;
	else
		return true;
}
 
// [CBaseEntity] CreateNextEntity [char]
// Purpose   : Creates an entity
// Arguments : The classname of an entity
// Returns   : A pointer to the new entity
 
CBaseEntity* CMapEntityFilter::CreateNextEntity( const char *pClassname ) 
{
	return CreateEntityByName( pClassname);
}
 
// [void] AddKeep [char]
// Purpose   : Adds the passed in value to our list of items to keep
// Arguments : The class name of an entity
// Returns   : Void
 
void CMapEntityFilter::AddKeep( const char *sz) 
{
	keepList->Insert(sz);
}

//BG2 - Tjoppen - AddTargetnameKeep. same as AddKeep, but for targetnames
void CMapEntityFilter::AddTargetnameKeep( const char *sz) 
{
	keepTargetnameList->Insert(sz);
}

//BG2 - Tjoppen - CMapEntityFilterExcluder
BEGIN_DATADESC( CMapEntityFilterExcluder )
	DEFINE_KEYFIELD( m_bExcludeByTargetName, FIELD_BOOLEAN, "ExcludeByTargetName" ),
	DEFINE_KEYFIELD( m_sClassOrTargetName, FIELD_STRING, "ClassOrTargetName" ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( mapfilter_excluder, CMapEntityFilterExcluder );
