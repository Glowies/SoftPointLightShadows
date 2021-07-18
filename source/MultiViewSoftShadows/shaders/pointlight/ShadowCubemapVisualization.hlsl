// Copyright (c) 2011 NVIDIA Corporation. All rights reserved.
//
// TO  THE MAXIMUM  EXTENT PERMITTED  BY APPLICABLE  LAW, THIS SOFTWARE  IS PROVIDED
// *AS IS*  AND NVIDIA AND  ITS SUPPLIERS DISCLAIM  ALL WARRANTIES,  EITHER  EXPRESS
// OR IMPLIED, INCLUDING, BUT NOT LIMITED  TO, NONINFRINGEMENT,IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL  NVIDIA 
// OR ITS SUPPLIERS BE  LIABLE  FOR  ANY  DIRECT, SPECIAL,  INCIDENTAL,  INDIRECT,  OR  
// CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION,  DAMAGES FOR LOSS 
// OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY 
// OTHER PECUNIARY LOSS) ARISING OUT OF THE  USE OF OR INABILITY  TO USE THIS SOFTWARE, 
// EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
//
// Please direct any bugs or questions to SDKFeedback@nvidia.com

#include "../MVSS_Common.hlsl"
#include "../../PointLight_Common.h"

//--------------------------------------------------------------------------------------
// Vertex shader
//--------------------------------------------------------------------------------------

struct PostProc_VSOut
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD;
};

// Vertex shader that generates a full screen triangle with texcoords
// To use, draw 3 vertices with primitive type triangle
PostProc_VSOut CubemapFullScreenTriangleVS(uint id : SV_VertexID)
{
    PostProc_VSOut output = (PostProc_VSOut)0.0f;
    output.uv = float2( (id << 1) & 2, id & 2 );
    output.pos = float4( output.uv * float2( 2.0f, -2.0f ) + float2( -1.0f, 1.0f ), 0.0f, 1.0f );
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel shader
//--------------------------------------------------------------------------------------

float ZClipToZEye(float zClip)
{
    return g_LightZFar*g_LightZNear / (g_LightZFar - zClip*(g_LightZFar-g_LightZNear));
}

float4 VisShadowCubemapPS(PostProc_VSOut IN) : SV_TARGET
{
    const float third = 1.0 / 3.0;

    float z = 0;
    if (IN.uv.y < third && IN.uv.x > 0.25 && IN.uv.x < 0.5) // Top row
    {
        float u = IN.uv.x * 4 - 1; // scale from [.25, .5] to [0, 1]
        float v = IN.uv.y * 3; // scale from [0, .333] to [0, 1]

        z = tShadowMapArray.Sample(PointSampler, float3(u, v, CUBE_SIDE_POS_Y));
    }
    else if(IN.uv.y > 2 * third && IN.uv.x > 0.25 && IN.uv.x < 0.5) // Bottom row
    {
        float u = IN.uv.x * 4 - 1; // scale from [.25, .5] to [0, 1]
        float v = IN.uv.y * 3 - 2; // scale from [.666, .1] to [0, 1]

        z = tShadowMapArray.Sample(PointSampler, float3(u, v, CUBE_SIDE_NEG_Y));
    }
    else if(IN.uv.y < 2 * third && IN.uv.y > third) // Middle row
    {
        float v = IN.uv.y * 3 - 1; // scale from [.333, .666] to [0, 1]

        if (IN.uv.x < 0.25)
        {
            float u = IN.uv.x * 4; // scale from [0, .25] to [0, 1]
            z = tShadowMapArray.Sample(PointSampler, float3(u, v, CUBE_SIDE_NEG_X));
        }
        else if (IN.uv.x < 0.5)
        {
            float u = IN.uv.x * 4 - 1; // scale from [.25, .5] to [0, 1]
            z = tShadowMapArray.Sample(PointSampler, float3(u, v, CUBE_SIDE_POS_Z));
        }
        else if (IN.uv.x < 0.75)
        {
            float u = IN.uv.x * 4 - 2; // scale from [.5, .75] to [0, 1]
            z = tShadowMapArray.Sample(PointSampler, float3(u, v, CUBE_SIDE_POS_X));
        }
        else
        {
            float u = IN.uv.x * 4 - 3; // scale from [.75, 1] to [0, 1]
            z = tShadowMapArray.Sample(PointSampler, float3(u, v, CUBE_SIDE_NEG_Z));
        }
    }

    z = (ZClipToZEye(z) - g_LightZNear) / (g_LightZFar - g_LightZNear);
    return z * 10;
}
