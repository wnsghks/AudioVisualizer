#include "StdAfx.hpp"
#include "Device.h"
#include "Timer.h"
#include "Input.h"

void Device::ResizeClient( const HWND& hWnd, UINT width, UINT height )
{
	if ( d3dDevice == nullptr ) return;

	d3dContext->OMSetRenderTargets( 0, nullptr, nullptr );

	//ID3D11RenderTargetView* rtv( renderTarget.GetRTView() );
	//SafeRelease( rtv );

	SafeRelease( swapChain );
	CreateSwapChain( hWnd, width, height );
}

void Device::Init( const HWND& _hWnd, UINT width, UINT height )
{
	if ( CreateDevice() == E_FAIL
		|| CreateSwapChain( _hWnd, width, height ) == E_FAIL
		//|| SetRenderTargetView() == E_FAIL
		|| dxgiFaxtory->MakeWindowAssociation( _hWnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER ) == E_FAIL )
	{
		throw;
	}
}

void Device::Release()
{
	SafeRelease( d3dDevice );
	SafeRelease( swapChain );
	SafeRelease( dxgiFaxtory );
	SafeRelease( d3dContext );
}

HRESULT Device::CreateFactory()
{
	if ( d3dDevice == NULL ) return E_FAIL;
	HRESULT hr;
	IDXGIDevice * pDXGIDevice;
	hr = d3dDevice->QueryInterface( __uuidof( IDXGIDevice ), ( void ** )&pDXGIDevice );

	IDXGIAdapter * pDXGIAdapter;
	hr = pDXGIDevice->GetParent( __uuidof( IDXGIAdapter ), ( void ** )&pDXGIAdapter );

	//IDXGIFactory * pIDXGIFactory = NULL;
	pDXGIAdapter->GetParent( __uuidof( IDXGIFactory ), ( void ** )&dxgiFaxtory );

	pDXGIDevice->Release();
	pDXGIAdapter->Release();
	return S_OK;

}

HRESULT Device::CreateDevice()
{
	IDXGIAdapter* adapter = nullptr;
	UINT createDeviceFlags( D3D11_CREATE_DEVICE_BGRA_SUPPORT );
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_DRIVER_TYPE driverType[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP ,
		D3D_DRIVER_TYPE_REFERENCE
	};
	HMODULE software = NULL;
	//UINT Flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	const D3D_FEATURE_LEVEL& featureLevel( D3D_FEATURE_LEVEL_11_0 );
	const UINT& featureLevels( 1 );
	const UINT& sdkVersion( D3D11_SDK_VERSION );
	D3D_FEATURE_LEVEL outFeatureLevel;

	const UINT& driverCnt( sizeof( driverType ) / sizeof( driverType[0] ) );

	for ( UINT iIndex = 0; iIndex < driverCnt; iIndex++ )
	{
		if ( D3D11CreateDevice( adapter, driverType[iIndex], software, createDeviceFlags,
			&featureLevel, featureLevels, sdkVersion, &d3dDevice, &outFeatureLevel, &d3dContext ) == E_FAIL )
		{
			return E_FAIL;
		}

		if ( outFeatureLevel < D3D_FEATURE_LEVEL_11_0 )
		{
			continue;
		}

		break;
	}

	return CreateFactory();
}

HRESULT Device::CreateSwapChain( const HWND& _hWnd, UINT width, UINT height )
{
	DXGI_SWAP_CHAIN_DESC desc = { 0, };
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BufferDesc.Width = width;
	desc.BufferDesc.Height = height;
	desc.BufferDesc.RefreshRate.Denominator = 1;
	desc.BufferDesc.RefreshRate.Numerator = 60;
	desc.SampleDesc.Count = 8;
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	desc.BufferCount = 1;
	desc.OutputWindow = _hWnd;
	desc.Windowed = TRUE;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	if ( FAILED( dxgiFaxtory->CreateSwapChain( d3dDevice, &desc, &swapChain ) ) )
	{
		return E_FAIL;
	}

	return S_OK;
}

//HRESULT Device::SetRenderTargetView()
//{
//	DXGI_SWAP_CHAIN_DESC SwapDesc = { 0, };
//	swapChain->GetDesc( &SwapDesc );
//	ID3D11Texture2D* pResource = nullptr;
//	if ( swapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pResource ) == E_FAIL
//		|| renderTarget.Create( d3dDevice, ( FLOAT )SwapDesc.BufferDesc.Width, ( FLOAT )SwapDesc.BufferDesc.Height, pResource ) == E_FAIL )
//	{
//		return E_FAIL;
//	}
//
//	return S_OK;
//}