#pragma once

#include "DXUT.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsdlg.h"
#include "SDKmisc.h"
#include "SDKmesh.h"
#include "Strsafe.h"
#include <algorithm>
#include <limits>

#define USE_POINT_LIGHT 

// Setting the shadow-map resolution..
// Any resolution may be used as long as the shadow maps fit in video memory.
#define CUBEMAP_RES 2048

// Number of sides constant for cubemap
#define CUBE_NUM_OF_SIDES 6


//--------------------------------------------------------------------------------------
// Depth texture array with fp32 precision (D32_FLOAT)
//--------------------------------------------------------------------------------------
class Cubemap
{
public:
    ID3D11Texture2D* pTexture;
    ID3D11ShaderResourceView* pSRV;
    ID3D11DepthStencilView* pDSV;
    ID3D11DepthStencilView* ppDSVs[CUBE_NUM_OF_SIDES];

    Cubemap()
        : pTexture(NULL)
        , pSRV(NULL)
        , pDSV(NULL)
    {
    }

    Cubemap(ID3D11Device* pd3dDevice)
    {
        HRESULT hr;

        // D3D11 allows the width and height to be in the range [1, 16384].
        // However, CreateTexture2D may fail if the texture array does not fit in video memory.
        D3D11_TEXTURE2D_DESC texDesc;
        texDesc.Width = CUBEMAP_RES;
        texDesc.Height = CUBEMAP_RES;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = CUBE_NUM_OF_SIDES;
        texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = NULL;
        texDesc.MiscFlags = NULL;
        V(pd3dDevice->CreateTexture2D(&texDesc, NULL, &pTexture));

        // Create a depth-stencil view for the whole texture array
        // This DSV is needed for clearing all the shadow maps at once.
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.Flags = 0; // new in D3D11
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
        dsvDesc.Texture2DArray.MipSlice = 0;
        dsvDesc.Texture2DArray.FirstArraySlice = 0;
        dsvDesc.Texture2DArray.ArraySize = texDesc.ArraySize;
        V(pd3dDevice->CreateDepthStencilView(pTexture, &dsvDesc, &pDSV));

        // Create a shader resource view for the whole texture array
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        srvDesc.Texture2DArray.MostDetailedMip = 0;
        srvDesc.Texture2DArray.MipLevels = 1;
        srvDesc.Texture2DArray.FirstArraySlice = 0;
        srvDesc.Texture2DArray.ArraySize = texDesc.ArraySize;
        V(pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &pSRV));

        // Create depth-stencil views for the individual layers
        D3D11_DEPTH_STENCIL_VIEW_DESC layerDsvDesc;
        pDSV->GetDesc(&layerDsvDesc);
        layerDsvDesc.Texture2DArray.MipSlice = 0;
        layerDsvDesc.Texture2DArray.ArraySize = 1;
        for (UINT i = 0; i < CUBE_NUM_OF_SIDES; ++i)
        {
            layerDsvDesc.Texture2DArray.FirstArraySlice = i;
            V(pd3dDevice->CreateDepthStencilView(pTexture, &layerDsvDesc, &ppDSVs[i]));
        }
    }

    ~Cubemap()
    {
        SAFE_RELEASE(pTexture);
        SAFE_RELEASE(pDSV);
        SAFE_RELEASE(pSRV);

        for (UINT i = 0; i < CUBE_NUM_OF_SIDES; ++i)
        {
            SAFE_RELEASE(ppDSVs[i]);
        }
    }
};


//--------------------------------------------------------------------------------------
// Shadow-map depth-buffer array and viewport
//--------------------------------------------------------------------------------------
class SceneShadowCubemap
{
public:
    SceneShadowCubemap()
        : pDepthRT(NULL)
    {
    }

    void CreateResources(ID3D11Device* pd3dDevice)
    {
        SAFE_DELETE(pDepthRT);
        pDepthRT = new Cubemap(pd3dDevice);

        Viewport.Width = CUBEMAP_RES;
        Viewport.Height = CUBEMAP_RES;
        Viewport.MinDepth = 0.f;
        Viewport.MaxDepth = 1.f;
        Viewport.TopLeftX = 0.f;
        Viewport.TopLeftY = 0.f;
    }

    void ReleaseResouces()
    {
        SAFE_DELETE(pDepthRT);
    }

    Cubemap* pDepthRT;
    D3D11_VIEWPORT Viewport;
};