/**
* OutlineSystem.hlsl
 * * @param UV          - The current pixel's screen coordinates (provided by Unreal)
 * @param Width       - Thickness of the outline in pixels
 * @param ViewSize    - Current screen resolution (float2)
 * @param Center      - The Stencil ID of the pixel currently being processed
 * @param CenterDepth - The Custom Depth value of the current pixel
 * @param SceneDepth  - The Scene Depth value of the current pixel
 */

* NOTE: Some IDs may be unavailable depending on your Rendering Path 
 * (Forward vs. Deferred) and Project Settings.
 */

// --- ESSENTIALS ---
#define PPI_SceneColor            0   // The final HDR color (Post-Lighting)
#define PPI_SceneDepth            1   // Distance to the first opaque surface
#define PPI_DiffuseColor          2   // Base color without lighting/shadows

// --- G-BUFFER DATA (Deferred Only) ---
#define PPI_SpecularColor         3   // Specular highlights data
#define PPI_SubsurfaceColor       4   // SSS data for skin/foliage
#define PPI_BaseColor             5   // Raw texture color
#define PPI_Specular              6   // Specular mask
#define PPI_Metallic              7   // Metalness value (0-1)
#define PPI_WorldNormal           8   // Surface direction (X, Y, Z)
#define PPI_AmbientOcclusion      9   // Baked or calculated AO
#define PPI_Opacity               10  // Transparency data
#define PPI_Roughness             11  // Surface smoothness (0-1)

// --- INTERACTION & OUTLINES ---
#define PPI_CustomDepth           14  // Depth of actors with 'Render Custom Depth' enabled
#define PPI_CustomStencil         13  // 8-bit Integer (0-255) used for masks and logic

// --- ADVANCED / MISC ---
#define PPI_SceneColor_AfterDOF   12  // Color buffer after Depth of Field is applied
#define PPI_MaterialAO            15  // Ambient Occlusion from the material input
#define PPI_Metallic_Specular     16  // Combined Metallic and Specular data
#define PPI_PostProcessInput0     17  // The main input for a Post-Process Material
#define PPI_PostProcessInput1     18  // Optional secondary input
#define PPI_PostProcessInput2     19  // Optional tertiary input
#define PPI_PostProcessInput3     20  // Optional quaternary input
#define PPI_DecalMask             21  // Mask for where decals are applied
#define PPI_ShadingModel          22  // ID for Lit, Unlit, Clear Coat, etc.
#define PPI_AmbientOcclusion_SSTP 23  // Screen Space Trace Path AO
#define PPI_Velocity              24  // Motion vectors for TAA and Motion Blur

// the func accepts any of the args externally. 
float GetOutline(float2 UV, float Width, float2 ViewSize, int Center, float CenterDepth, float SceneDepth)
{
	// 1. Calculate the 'Step Size' (Texel) 
	float2 Texel = 1.0 / ViewSize;

	// 2. PERFORMANCE EARLY OUT
	// If we aren't an item (0) AND we have no custom depth, stop.
	if (Center == 0 && CenterDepth <= 0) 
	{
		return 0;
	}

	// 3. OCCLUSION CHECK
	// If the item depth is significantly further than the wall depth, hide it.
	if (CenterDepth > (SceneDepth + 10.0)) 
	{
		return 0;
	}

	// 4. THE COMPASS (8-Directions for smooth corners)
	float2 Offsets[8] = {
		float2(0,1), float2(0,-1), float2(1,0), float2(-1,0),
		float2(1,1), float2(-1,-1), float2(1,-1), float2(-1,1)
	};

	int MaxStencilFound = 0;

	// 5. THE NEIGHBOR SEARCH
	for(int i = 0; i < 8; i++) 
	{
		// Calculate Neighbor coordinate
		float2 NeighborUV = UV + (Offsets[i] * Width * Texel);
        
		// Look up the stencil ID of the neighbor (ID 13 is Custom Stencil)
		int NeighborStencil = SceneTextureLookup(NeighborUV, 13, false).r;
        
		// EDGE DETECTION: If neighbor has an ID and the current pixel is different.
		if (NeighborStencil != Center) 
		{
			// Pick the highest ID (Priority: Enemies > NPCs > Items)
			MaxStencilFound = max(MaxStencilFound, NeighborStencil);
		}
	}

	// 6. Return the ID to be mapped by the Curve Atlas
	return (float)MaxStencilFound;
}