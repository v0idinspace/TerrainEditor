/**
----------------------------------------------------------------------------
  File:              terrain.cl

  Language:          C++

  License:           

  Author:            Tasos Giannakopoulos (dotvoidd@gmail.com)
  Copyright:         
  Date:              01 Nov 2012

  Description:       Voxel Terrain Generation system

----------------------------------------------------------------------------
*/


#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable

// UnitCube sampler
__constant sampler_t gNoiseSampler    = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_REPEAT | CLK_FILTER_LINEAR;
__constant sampler_t gTerrainSampler  = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;


//__kernel void BuildTerrainPatchFromHeightmap(
//                
//                )
//{
//}

/*
 * Returns a 3D image that holds the density values
 * of the general terrain
 * input: Terrain size, ground y, control curves, 
 *        control points, noise 3D texture, 
 *        octaves fBm values
 * out:   3D texture size of (terrain size)^3
 */
__kernel void BuildTerrainPatch(
                __write_only image3d_t destImage, 
                uint size,
                uint ground,
                uint hmGenerate,
                uint hmSampleY,
                uint hmNumOctaves,
                float hmPersistance,
                float hmIntensity,
                float gradientIntensity,
                uint gradientStart,
                float hardGroundIntensity,
                uint hardGroundStart,
                __constant float4* noiseLayers,
                uint noiseLayersSize,
                __constant float8* metaballs,
                uint numMetaballs,
                __read_only image3d_t noisePrefab0,   // Noise prefab 0
                uint noisePrefab0Size,
                __read_only image3d_t noisePrefab1,   // Noise prefab 1
                uint noisePrefab1Size,
                __read_only image3d_t noisePrefab2,   // Noise prefab 2
                uint noisePrefab2Size) 
{
  int x = get_global_id(0);
  int y = 0;
  int z = get_global_id(1);

  // Host should calculate that once and pass it...
  float invSize = 1.0f / size;
  
  // Generate the general terrain
  for(int i = 0; i < size; i++)
  {
    // Setup the noise samples coords (scale over terrain)
    // PrefabIndex | Scale | Intensity
    y = i;
    int4 	coord	= (int4)(x, y, z, 0);
    float4 	coordf	= (float4)(x, y, z, 0);
	
    // --- Density ---
    float density = 0;

    // A. Ground
    density = (float)(y<ground)*(1.0f);

    /*
    bool genHeightmap = true;
    
    // B. Generate height map
    if(hmGenerate)
    {
      // sample from prefab 0 y = rand
      float hm = 0;

      // Multioctave terrain - noise synthesis
      for(uint oct=0; oct<hmNumOctaves; oct++)
      {
        float hmScale = pow(2.0f, (float)oct);
        float hmInt   = pow(hmPersistance, (float)oct);

        // Where to sample from
        float4 hmCoord = (float4)((float)x         / size,
                                  (float)hmSampleY / size,
                                  (float)z         / size, 0);

        float nSample = read_imagef(noisePrefab0, gNoiseSampler, hmCoord*hmScale).x;
        
        hm += nSample * hmInt;
      }
      
      float th = ground + (hm * (size - ground));
      
      density += (y<th) * hmIntensity;
    }
    */
    
    

    // B. Metaballs	
	  for(uint itM=0; itM<numMetaballs; itM++)
	  {
      float4 pos = (float4)(metaballs[itM].s0, metaballs[itM].s1, metaballs[itM].s2, 0);
      float  r   = metaballs[itM].s3;
      float  i   = metaballs[itM].s4;

      // Calculate current voxel's distance from metaball position
		  float4 nn = pos - coordf; // same as coord but float numbers
		  float d   = length(nn);
		
		  density += r / pow(d, i); // d: length of current voxel from metaball position
	  }
	  //\ Eof blobs
    
    // C. Add noise layers
	  float mfNoise = 0;
	
    for(uint itNL=0; itNL<noiseLayersSize; itNL++)
    {
      uint  prefabIndex = (uint)noiseLayers[itNL].x;  // x: Prefab index
      uint  type        = (uint)noiseLayers[itNL].y;  // y: Type [ADD, SUB]
      float nScale      = noiseLayers[itNL].z;        // z: Scale
      float nIntensity  = noiseLayers[itNL].w;        // w: Intensity

      // Noise raw sample
      float4 nSample = 0;

      // Get the noise sample -- depending on the prefab chosen
      if(prefabIndex == 0)
      {
        float4 noiseCoordf = (float4)((float)x / noisePrefab0Size,//noisePrefab0Size, 
                                      (float)y / noisePrefab0Size,//noisePrefab0Size, 
                                      (float)z / noisePrefab0Size, 0);//noisePrefab0Size, 0);  // Normalized coords

        nSample = read_imagef(noisePrefab0, gNoiseSampler, noiseCoordf * nScale);
      }
      else if(prefabIndex == 1)
      {
        float4 noiseCoordf = (float4)((float)x / noisePrefab1Size, 
                                      (float)y / noisePrefab1Size, 
                                      (float)z / noisePrefab1Size, 0);  // Normalized coords

        nSample = read_imagef(noisePrefab1, gNoiseSampler, noiseCoordf * nScale);
      }
      else if(prefabIndex == 2)
      {
        float4 noiseCoordf = (float4)((float)x / noisePrefab2Size, 
                                      (float)y / noisePrefab2Size, 
                                      (float)z / noisePrefab2Size, 0);  // Normalized coords

        nSample = read_imagef(noisePrefab2, gNoiseSampler, noiseCoordf * nScale);
      }
      
      // ADD or SUB depending on the noise layer type
      if(type == 0)       // SUB
        mfNoise -=  nIntensity * nSample.x;
      else if(type == 1)  // ADD
        mfNoise +=  nIntensity * nSample.x;
    }
	
	  density = density + mfNoise;
	
    // D. Height gradient
    float g = (y-gradientStart)*invSize;  // size: size of terrain patch
    density -=  (y>=gradientStart) * (g) * gradientIntensity;
    
    // E. Hard ground
    uint hardFloorY = hardGroundStart;
    float hf = 1 - y * invSize; // size: size of terrain patch
    density -= (y>=hardFloorY) * (hf) * hardGroundIntensity;
    

    // TEMP -- Clear the sides (block look)
    if(x < 1 || x == (size-1))
      density = 0;
    if(y < 1 || y == (size-1))
      density = 0;
    if(z < 1 || z == (size-1))
      density = 0;
    //\ Eof clear the sides

    write_imagef(destImage, coord, density); 
  }
}

// temp -- helpers
__kernel void ClearVBO(__global float4* pos, __global float4* norm, uint size)
{
  uint i = get_global_id(0);

  if(i <= size)
  {
    pos[i] = (float4)(0, 0, 0, 0);
    norm[i] = (float4)(0, 0, 0, 0);
  }
}

__kernel void ClearVolume(
                __write_only image3d_t destImage,
                uint size)
{
  int x = get_global_id(0);
  int z = get_global_id(1);

  float density = 0;

  // Clear the volume
  for(int y = 0; y < size; y++)
  {
    int4 coord        = (int4)(x, y, z, 0);

    if( (x < size) && (y < size) && (z < size) )
      write_imagef(destImage, coord, density);
  }
}