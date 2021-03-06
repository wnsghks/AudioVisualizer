//#include "..\\..\\..\\Standard\Standard.hpp"
#include "RenderTarget.h"
#include "..\\..\\..\\Standard\Base\BaseUtility.hpp"
#include "..\\..\\..\\Standard\Base\BaseConst.hpp"

RenderTarget::RenderTarget() : countViewPort( 0 ) { }

HRESULT	RenderTarget::Create( ID3D11Device* d3dDevice, FLOAT Width, FLOAT Height, ID3D11Texture2D* pTex )
{
	HRESULT hr( S_OK );
	D3D11_TEXTURE2D_DESC desc = { 0, };
	ViewPortSet( 0, 0, Width, Height );

	desc.Width = static_cast< UINT >( Width );
	desc.Height = static_cast< UINT >( Height );
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.ArraySize = 1;

	ID3D11Texture2D* pTexture( nullptr );

	if ( FAILED( hr = d3dDevice->CreateTexture2D( &desc, nullptr, &pTexture ) ) )
	{
		return hr;
	}
	if ( pTex == nullptr )
	{
		if ( FAILED( hr = d3dDevice->CreateShaderResourceView( pTexture, nullptr, &resRTV ) ) )
		{
			return hr;
		}
		if ( FAILED( hr = d3dDevice->CreateRenderTargetView( pTexture, nullptr, &curRTView ) ) )
		{
			return hr;
		}
	}
	else
	{
		if ( FAILED( hr = d3dDevice->CreateRenderTargetView( pTex, nullptr, &curRTView ) ) )
		{
			return hr;
		}
	}
	if ( FAILED( hr = UpdateDepthStencilView( d3dDevice, ( FLOAT )Width, ( FLOAT )Height ) ) )
	{
		return hr;
	}
	return hr;
}
void RenderTarget::ViewPortSet( FLOAT TopLeftX, FLOAT TopLeftY, FLOAT Width, FLOAT Height )
{
	curViewPort.TopLeftX = TopLeftX;
	curViewPort.TopLeftY = TopLeftY;
	curViewPort.Width = Width;
	curViewPort.Height = Height;
	curViewPort.MinDepth = 0;
	curViewPort.MaxDepth = 1;
}
HRESULT RenderTarget::UpdateDepthStencilView( ID3D11Device* pDevice, FLOAT iWidth, FLOAT iHeight )
{
	HRESULT hr( S_OK );
	if ( curDSView != nullptr )
	{
		SafeRelease( curDSView );
	}

	curViewPort.Width = iWidth;
	curViewPort.Height = iHeight;

	ID3D11Texture2D* pTexture( nullptr );
	CD3D11_TEXTURE2D_DESC DescDepth;
	DescDepth.Width = ( UINT )iWidth;
	DescDepth.Height = ( UINT )iHeight;
	DescDepth.MipLevels = 1;
	DescDepth.ArraySize = 1;
	DescDepth.Format = DXGI_FORMAT_R32_TYPELESS;
	DescDepth.SampleDesc.Count = 8;
	DescDepth.SampleDesc.Quality = 0;
	DescDepth.Usage = D3D11_USAGE_DEFAULT;

	if ( DescDepth.Format == DXGI_FORMAT_D24_UNORM_S8_UINT )
		DescDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	else
		DescDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	DescDepth.CPUAccessFlags = 0;
	DescDepth.MiscFlags = 0;
	if ( FAILED( hr = pDevice->CreateTexture2D( &DescDepth, nullptr, &pTexture ) ) )
	{
		return hr;
	}

	///// ���̴� ���ҽ� ���� : ���� �� �����쿡�� ����Ѵ�. ///
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory( &dsvDesc, sizeof( D3D11_DEPTH_STENCIL_VIEW_DESC ) );
	ZeroMemory( &srvDesc, sizeof( D3D11_SHADER_RESOURCE_VIEW_DESC ) );

	switch ( DescDepth.Format )
	{
	case DXGI_FORMAT_R32_TYPELESS:
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		break;
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		break;
	}
	if ( Setting::AntiAliasing == true ) 
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS; 
	else 
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	
	//pDevice->CreateDepthStencilView(m_pTexture, &dsvDesc, &curDSView);
	//D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
	//ZeroMemory(&dsvd, sizeof(dsvd));
	//dsvd.Format = DescDepth.Format;
	//dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	//dsvd.Texture2D.MipSlice = 0;

	if ( FAILED( hr = pDevice->CreateDepthStencilView( pTexture, &dsvDesc, &curDSView ) ) )
	{
		return hr;
	}

	if ( DescDepth.Format == DXGI_FORMAT_R32_TYPELESS )
	{
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		pDevice->CreateShaderResourceView( pTexture, &srvDesc, &resDSV );
	}
	curDSView->GetDesc( &descDepthStencil );
	return hr;
}
void	RenderTarget::Apply( ID3D11DeviceContext* d3dContext, ID3D11RenderTargetView* pRTV, ID3D11DepthStencilView* pDSV, bool bDepth, bool bStencil )
{
	ID3D11RenderTargetView* pNullRTV( nullptr );
	d3dContext->OMSetRenderTargets( 1, &pNullRTV, nullptr );

	if ( pRTV != nullptr )
	{
		if ( pDSV != nullptr )
			d3dContext->OMSetRenderTargets( 1, &pRTV, pDSV );
		else
			d3dContext->OMSetRenderTargets( 1, &pRTV, curDSView );
	}
	else
	{
		if ( pDSV != nullptr )
			d3dContext->OMSetRenderTargets( 1, &curRTView, pDSV );
		else
			d3dContext->OMSetRenderTargets( 1, &curRTView, curDSView );
	}
	if ( bDepth == true )
	{
		if ( pDSV != nullptr )
			d3dContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );
		else
			d3dContext->ClearDepthStencilView( curDSView, D3D11_CLEAR_DEPTH, 1.0, 0 );

	}
	if ( bStencil == true )
	{
		if ( pDSV != nullptr )
			d3dContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_STENCIL, 1.0, 0 );
		else
			d3dContext->ClearDepthStencilView( curDSView, D3D11_CLEAR_STENCIL, 1.0, 0 );
	}
	d3dContext->RSSetViewports( 1, &curViewPort );

}
bool	RenderTarget::Begin( ID3D11DeviceContext* d3dContext, D3DXVECTOR4 vColor, bool bTarget, bool bDepth, bool bStencil )
{
	countViewPort = 1;
	d3dContext->RSGetViewports( &countViewPort, &oldViewPort );
	d3dContext->OMGetRenderTargets( 1, &oldRTView, &oldDSView );

	ID3D11RenderTargetView* pNullRTV( nullptr );
	ID3D11DepthStencilView* pNullDSV( nullptr );
	d3dContext->OMSetRenderTargets( 1, &pNullRTV, pNullDSV );
	d3dContext->OMSetRenderTargets( 1, &curRTView, curDSView );

	Clear( d3dContext, vColor, bTarget, bDepth, bStencil );
	//d3dContext->RSSetViewports(1, &curViewPort);
	return true;
}
bool	RenderTarget::Clear( ID3D11DeviceContext* d3dContext, D3DXVECTOR4 vColor, bool bTarget, bool bDepth, bool bStencil )
{
	if ( bTarget  == true && curRTView != nullptr )
	{
		d3dContext->ClearRenderTargetView( curRTView, vColor );
	}
	if ( bDepth == true && curDSView != nullptr )
	{
		d3dContext->ClearDepthStencilView( curDSView, D3D11_CLEAR_DEPTH, 1.0, 0 );
	}
	if ( bStencil == true && curDSView != nullptr )
	{
		d3dContext->ClearDepthStencilView( curDSView, D3D11_CLEAR_STENCIL, 1.0, 0 );
	}
	d3dContext->RSSetViewports( 1, &curViewPort );

	return true;
}
bool	RenderTarget::End( ID3D11DeviceContext* d3dContext )
{
	d3dContext->RSSetViewports( countViewPort, &oldViewPort );
	d3dContext->OMSetRenderTargets( 1, &oldRTView, oldDSView );
	SafeRelease( oldRTView );
	SafeRelease( oldDSView );
	return true;
}

void RenderTarget::Release()
{
	SafeRelease( curRTView );
	SafeRelease( curDSView );
	SafeRelease( oldRTView );
	SafeRelease( oldDSView );
	SafeRelease( resRTV );
	SafeRelease( resDSV );
}