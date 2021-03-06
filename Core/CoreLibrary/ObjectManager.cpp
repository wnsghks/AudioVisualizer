#include "StdAfx.hpp"
#include "ObjectManager.h"
#include "DxManager.h"
#include "DefaultCamera.h"
#include "UICamera.h"

void ObjectManager::PrevInit()
{
	Camera* MainCam = new DefaultCamera();
	MainCam->Create( DxManager::Get()->GetClientRect().right, DxManager::Get()->GetClientRect().bottom );
	cameras.insert( std::make_pair( ECamera::Main, MainCam ) );

	Camera* UICam = new UICamera();
	UICam->Create( DxManager::Get()->GetClientRect().right, DxManager::Get()->GetClientRect().bottom );
	cameras.insert( std::make_pair( ECamera::UI, UICam ) );
}

void ObjectManager::PostInit()
{
	for ( std::pair<ECamera, Camera*> cam : cameras )
	{
		cam.second->Init();
	}

	for ( std::pair<EObject, std::list<GameObject*>> lists : objects )
	{
		for ( GameObject* oneObject : lists.second )
		{
			oneObject->Init();
		}
	}
}
	
void ObjectManager::Frame()
{
	for ( std::pair<ECamera, Camera*> cam : cameras )
	{
		cam.second->Frame();
	}

	for ( std::pair<EObject, std::list<GameObject*>> lists : objects )
	{
		for ( GameObject* oneObject : lists.second )
		{
			oneObject->Frame();
		}
	}
}

void ObjectManager::Render( ID3D11DeviceContext* context )
{
	for ( std::pair<EObject, std::list<GameObject*>> lists : objects )
	{
		for ( GameObject* oneObject : lists.second )
		{
			oneObject->Render( context );
		}
	}
}

void ObjectManager::Release()
{
	for ( std::map<ECamera, Camera*>::iterator camera = std::begin( cameras );
		  camera != std::end( cameras ); camera++ )
	{
		SafeRelease( camera->second, true );
	}
	cameras.clear();

	for ( std::map<EObject, std::list<GameObject*>>::iterator outiter = std::begin( objects );
		  outiter != std::end( objects ); outiter++ )
	{
		for ( GameObject*& oneObject : outiter->second )
		{
			SafeRelease( oneObject, true );
		}
	}
	objects.clear();
}

void ObjectManager::AddObject( GameObject* _obj )
{
	if ( _obj == nullptr ) 
		throw NullPointer( __FUNCTION__" - the argument is null reference.\n" );

	for ( std::pair<EObject, std::list<GameObject*>> lists : objects )
	{
		for ( GameObject* oneObject : lists.second )
		{
			if ( oneObject == _obj ) return;
		}
	}

	const EObject& type( _obj->objectType );
	const std::map<EObject, std::list<GameObject*>>::iterator& iter( objects.find( type ) );
	if ( iter == std::cend( objects ) )
	{
		std::list<GameObject*> newlist;
		newlist.emplace_back( _obj );
		objects.insert( std::make_pair( type, newlist ) );

		return;
	}

	std::list<GameObject*>& _list( iter->second );
	_list.emplace_back( _obj );
}

void ObjectManager::RemoveObject( GameObject* _obj )
{
	if ( _obj == nullptr ) 
		throw NullPointer( __FUNCTION__" - the argument is null reference.\n" );

	const EObject& type( _obj->objectType );
	const std::map<EObject, std::list<GameObject*>>::iterator& iter = objects.find( type );
	if ( iter == objects.cend() ) 
		throw EmptyData( __FUNCTION__" - the objects initerator has reached the end.\n" );

	std::list<GameObject*>& _list( iter->second );
	const std::list<GameObject*>::iterator& _iter( std::find( std::begin( _list ), std::end( _list ), _obj ) );
	if ( _iter == std::end( _list ) ) 
		throw EmptyData( __FUNCTION__" - the objects outiterator has reached the end.\n" );

	GameObject*& delObject( *_iter );
	SafeRelease( delObject );
	_list.erase( _iter );
}

Camera* ObjectManager::GetCamera( ECamera _camType )
{
	const std::map<ECamera, Camera*>::const_iterator& iter( cameras.find( _camType ) );
	if ( iter == std::cend( cameras ) )
	{
		return curCamera;
	}

	return iter->second;
}

void ObjectManager::ResizeClient( UINT width, UINT height )
{
	for ( const std::pair<ECamera, Camera*>& cam : cameras )
	{
		cam.second->CreateProjMatrix( width, height );
	}
}