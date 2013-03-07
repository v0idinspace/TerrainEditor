

__constant sampler_t gVolumeSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;
__constant sampler_t gVolumeSamplerN = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_REPEAT | CLK_FILTER_LINEAR;


float4 VertexInterp(float isoValue, float4 p1, float4 p2, float densityP1, float densityP2)
{
  float t = (isoValue - densityP1) / (densityP2 - densityP1);
	return mix(p1, p2, t);
}

float4 VertexInterp2(float isoValue, float4 p1, float4 p2, float d1, float d2)
{
  //return p1 - p2;
  float t = (isoValue - d1) / (d2 - d1);
	return mix(p1, p2, t);
}


// 1. Classify voxels
__kernel void ClassifyVoxels(
                __read_only image3d_t blockData,
                uint blockSize,
                float isoValue,
                __global uint* activeVoxels,
                __global uint* numVertsToGenerate,
                __global uint* edgeTable,
                __global int* triTable)
{
  uint i    = get_global_id(0);
  uint j    = get_global_id(1);

  uint blockSizeSqr = blockSize*blockSize;

  for(uint k=0; k<blockSize; k++)
  {
    int4 gridCoord = (int4)(i, j, k, 0);
    uint  voxelID   = i  + (j>0)*(j*blockSizeSqr-1) + k*blockSize;   // ID works
  
    // Curent voxel corner coords
    int4 voxCornCoord0 = gridCoord + (int4)(0, 0, 1, 0);
    int4 voxCornCoord1 = gridCoord + (int4)(1, 0, 1, 0);
    int4 voxCornCoord2 = gridCoord + (int4)(1, 0, 0, 0);
    int4 voxCornCoord3 = gridCoord + (int4)(0, 0, 0, 0);
    int4 voxCornCoord4 = gridCoord + (int4)(0, 1, 1, 0);
    int4 voxCornCoord5 = gridCoord + (int4)(1, 1, 1, 0);
    int4 voxCornCoord6 = gridCoord + (int4)(1, 1, 0, 0);
    int4 voxCornCoord7 = gridCoord + (int4)(0, 1, 0, 0);
    
    // use the above to sample the voxel corenr density values and generate case number
    // Test: output case number 0-255
    float cornerDensity[8];
    cornerDensity[0] = read_imagef(blockData, gVolumeSampler, voxCornCoord0).x;
    cornerDensity[1] = read_imagef(blockData, gVolumeSampler, voxCornCoord1).x;
    cornerDensity[2] = read_imagef(blockData, gVolumeSampler, voxCornCoord2).x;
    cornerDensity[3] = read_imagef(blockData, gVolumeSampler, voxCornCoord3).x;
    cornerDensity[4] = read_imagef(blockData, gVolumeSampler, voxCornCoord4).x;
    cornerDensity[5] = read_imagef(blockData, gVolumeSampler, voxCornCoord5).x;
    cornerDensity[6] = read_imagef(blockData, gVolumeSampler, voxCornCoord6).x;
    cornerDensity[7] = read_imagef(blockData, gVolumeSampler, voxCornCoord7).x;

    // Get the voxel intex
    int voxelCase;
	  voxelCase =  (cornerDensity[0] < isoValue);
	  voxelCase += (cornerDensity[1] < isoValue)*2;
	  voxelCase += (cornerDensity[2] < isoValue)*4;
	  voxelCase += (cornerDensity[3] < isoValue)*8;
	  voxelCase += (cornerDensity[4] < isoValue)*16;
	  voxelCase += (cornerDensity[5] < isoValue)*32;
	  voxelCase += (cornerDensity[6] < isoValue)*64;
	  voxelCase += (cornerDensity[7] < isoValue)*128;

    // a. Store active voxels ID
    if((edgeTable[voxelCase] != 0) && (edgeTable[voxelCase] != 255))
      activeVoxels[voxelID] = voxelCase;

    // b. For each active voxel
    //    calculate how many vertices we will generate
    uint ttIndex = (voxelCase>0)*((voxelCase)*16);        // TriangleTable index -- TODO image2D sampler

    uint numVertices = 0;

    for(int p=0; p<12; p++)
    {
      if(triTable[ttIndex+p] != -1)
        numVertices++;
    }
    numVertsToGenerate[voxelID] = numVertices;
  }
}

// Generate the posBufferOffsets
__kernel void CalculateVertBufferOffsets(
                __global uint* activeVoxels,
                __global uint* numVertsToGenerate,
                __global uint* vertBufferOffset,
                uint numVoxels)
{
  // use single thread for now
  uint prevID = 0;

  // Calculate current offset
  for(int id=0; id<numVoxels; id++)
  {
    if(activeVoxels[id])
    {
      vertBufferOffset[id] = vertBufferOffset[prevID] + numVertsToGenerate[prevID];

      prevID = id;
    }
  }
}

void CreateCornerGradient(float4* cornerGrads, float4* vCoords, float* cornerDensity, __read_only image3d_t volume, float size)
{
  // Get the 3 neighbours density
  // 0
  float4 nbourCoord0X = vCoords[0] + (float4)(-1,  0,  0,  0);
  float  nbour0Xval   = read_imagef(volume, gVolumeSampler, nbourCoord0X).x;
  float4 nbourCoord0Y = vCoords[0] + (float4)( 0,  -1,  0,  0);
  float  nbour0Yval   = read_imagef(volume, gVolumeSampler, nbourCoord0Y).x;
  float4 nbourCoord0Z = vCoords[0] + (float4)( 0,  0,  1,  0);
  float  nbour0Zval   = read_imagef(volume, gVolumeSampler, nbourCoord0Z).x;

  // 1
  float4 nbourCoord1X = vCoords[1] + (float4)(1,  0,   0,  0);
  float  nbour1Xval   = read_imagef(volume, gVolumeSampler, nbourCoord1X).x;
  float4 nbourCoord1Y = vCoords[1] + (float4)( 0,  -1,  0,  0);
  float  nbour1Yval   = read_imagef(volume, gVolumeSampler, nbourCoord1Y).x;
  float4 nbourCoord1Z = vCoords[1] + (float4)( 0,  0,  1,  0);
  float  nbour1Zval   = read_imagef(volume, gVolumeSampler, nbourCoord1Z).x;

  // 2
  float4 nbourCoord2X = vCoords[2] + (float4)(1,  0,  0,  0);
  float  nbour2Xval   = read_imagef(volume, gVolumeSampler, nbourCoord2X).x;
  float4 nbourCoord2Y = vCoords[2] + (float4)( 0,  -1,  0,  0);
  float  nbour2Yval   = read_imagef(volume, gVolumeSampler, nbourCoord2Y).x;
  float4 nbourCoord2Z = vCoords[2] + (float4)( 0,  0,  -1,  0);
  float  nbour2Zval   = read_imagef(volume, gVolumeSampler, nbourCoord2Z).x;

  // 3
  float4 nbourCoord3X = vCoords[3] + (float4)(-1,  0,  0,  0);
  float  nbour3Xval   = read_imagef(volume, gVolumeSampler, nbourCoord3X).x;
  float4 nbourCoord3Y = vCoords[3] + (float4)( 0,  -1,  0,  0);
  float  nbour3Yval   = read_imagef(volume, gVolumeSampler, nbourCoord3Y).x;
  float4 nbourCoord3Z = vCoords[3] + (float4)( 0,  0,  -1,  0);
  float  nbour3Zval   = read_imagef(volume, gVolumeSampler, nbourCoord3Z).x;

  // 4
  float4 nbourCoord4X = vCoords[4] + (float4)(-1,  0,  0,  0);
  float  nbour4Xval   = read_imagef(volume, gVolumeSampler, nbourCoord4X).x;
  float4 nbourCoord4Y = vCoords[4] + (float4)( 0, 1,  0,  0);

  //nbourCoord4Y.y = (float)((int)nbourCoord4Y.y % (int)(size-1));
  float  nbour4Yval   = read_imagef(volume, gVolumeSampler, nbourCoord4Y).x;
  float4 nbourCoord4Z = vCoords[4] + (float4)( 0,  0,  1,  0);
  float  nbour4Zval   = read_imagef(volume, gVolumeSampler, nbourCoord4Z).x;

  // 5
  float4 nbourCoord5X = vCoords[5] + (float4)( 1,  0,  0,  0);
  float  nbour5Xval   = read_imagef(volume, gVolumeSampler, nbourCoord5X).x;
  float4 nbourCoord5Y = vCoords[5] + (float4)( 0, 1,  0,  0);

  //nbourCoord5Y.y = (float)((int)nbourCoord5Y.y % (int)(size-1));
  float  nbour5Yval   = read_imagef(volume, gVolumeSampler, nbourCoord5Y).x;
  float4 nbourCoord5Z = vCoords[5] + (float4)( 0,  0,  1,  0);
  float  nbour5Zval   = read_imagef(volume, gVolumeSampler, nbourCoord5Z).x;

  // 6
  float4 nbourCoord6X = vCoords[6] + (float4)( 1,  0,  0,  0);
  float  nbour6Xval   = read_imagef(volume, gVolumeSampler, nbourCoord6X).x;
  float4 nbourCoord6Y = vCoords[6] + (float4)( 0, 1,  0,  0);

  //nbourCoord6Y.y = (float)((int)nbourCoord6Y.y % (int)(size-1));
  float  nbour6Yval   = read_imagef(volume, gVolumeSampler, nbourCoord6Y).x;
  float4 nbourCoord6Z = vCoords[6] + (float4)( 0,  0, -1,  0);
  float  nbour6Zval   = read_imagef(volume, gVolumeSampler, nbourCoord6Z).x;

  // 7
  float4 nbourCoord7X = vCoords[7] + (float4)(-1,  0,  0,  0);
  float  nbour7Xval   = read_imagef(volume, gVolumeSampler, nbourCoord7X).x;
  float4 nbourCoord7Y = vCoords[7] + (float4)( 0, 1,  0,  0);

  //nbourCoord7Y.y = (float)((int)nbourCoord7Y.y % (int)(size-1));
  float  nbour7Yval   = read_imagef(volume, gVolumeSampler, nbourCoord7Y).x;
  float4 nbourCoord7Z = vCoords[7] + (float4)( 0,  0, -1,  0);
  float  nbour7Zval   = read_imagef(volume, gVolumeSampler, nbourCoord7Z).x;


  // 0
  cornerGrads[0] = (float4)(cornerDensity[1] - nbour0Xval, 
                            nbour0Yval - cornerDensity[4], 
                            nbour0Zval - cornerDensity[3], 0);
  
  // 1
  cornerGrads[1] = (float4)(nbour1Xval - cornerDensity[0], 
                            nbour1Yval - cornerDensity[5], 
                            nbour1Zval - cornerDensity[2], 0);

  // 2
  cornerGrads[2] = (float4)(nbour2Xval - cornerDensity[3], 
                            nbour2Yval - cornerDensity[6], 
                            cornerDensity[1] -nbour2Zval, 0);

  // 3
  cornerGrads[3] = (float4)(cornerDensity[2] - nbour3Xval, 
                            nbour3Yval - cornerDensity[7], 
                            cornerDensity[0] - nbour3Zval, 0);

  // 4
  cornerGrads[4] = (float4)(cornerDensity[5] - nbour4Xval, 
                            cornerDensity[0] - nbour4Yval, 
                            nbour4Zval - cornerDensity[7], 0);
  
  // 5
  cornerGrads[5] = (float4)(nbour5Xval - cornerDensity[4], 
                            cornerDensity[1] - nbour5Yval, 
                            nbour5Zval - cornerDensity[6], 0);

  // 6
  cornerGrads[6] = (float4)(nbour6Xval - cornerDensity[7], 
                            cornerDensity[2] - nbour6Yval, 
                            cornerDensity[5] - nbour6Zval, 0);

  // 7
  cornerGrads[7] = (float4)(cornerDensity[6] - nbour7Xval, 
                            cornerDensity[3] - nbour7Yval, 
                            cornerDensity[4] - nbour7Zval, 0);

  //cornerGrads[4] = (float4)(0, -1, 0, 0);
  //cornerGrads[5] = (float4)(0, -1, 0, 0);
  //cornerGrads[6] = (float4)(0, 0, 0, 0);
  //cornerGrads[7] = (float4)(0, 0, 0, 0);
}

__kernel void GenerateTriangles(
                __read_only image3d_t blockData,
                uint blockSize,
                float isoValue,
                __global float4* pos,
                __global float4* norm,
                __global uint* numVertsToGenerate,
                __global uint* vertBufferOffset,
                __global uint* activeVoxels,
                __global uint* edgeTable,
                __global int* triTable)
{
  uint i    = get_global_id(0);
  uint j    = get_global_id(1);

  uint blockSizeSqr = blockSize*blockSize;

  for(uint k=0; k<blockSize; k++)
  {
    float voxelSize = 1.0f;//0.03125f;
    uint  voxelID   = i  + (j>0)*(j*blockSizeSqr-1) + k*blockSize;   // ID works
    uint  voxelCase = activeVoxels[voxelID];
    uint  edgeCase  = edgeTable[voxelCase];

    // Grid coordinates -- uint used to sample the volume
    int4 gridCoordi   = (int4)(i, j, k, 0);

    // -0.5f is the offset to center the block in 0,0,0
    float4 gridCoordf = (float4)(i*voxelSize, j*voxelSize, k*voxelSize, 1);
    
    // Generate triangles of the active voxels
    if(activeVoxels[voxelID])
    {
      // Curent voxel corner coords -- used to sample the volume
      float4 vCoords[8];
      vCoords[0] = gridCoordf + (float4)(0, 0, 1, 0);
      vCoords[1] = gridCoordf + (float4)(1, 0, 1, 0);
      vCoords[2] = gridCoordf + (float4)(1, 0, 0, 0);
      vCoords[3] = gridCoordf + (float4)(0, 0, 0, 0);
      vCoords[4] = gridCoordf + (float4)(0, 1, 1, 0);
      vCoords[5] = gridCoordf + (float4)(1, 1, 1, 0);
      vCoords[6] = gridCoordf + (float4)(1, 1, 0, 0);
      vCoords[7] = gridCoordf + (float4)(0, 1, 0, 0);
    

      // for the vertices position in the world + we will use the world matrix later
      float4 v[8];
      v[0] = gridCoordf + (float4)(0,         0,          voxelSize,  0);
      v[1] = gridCoordf + (float4)(voxelSize, 0,          voxelSize,  0);
      v[2] = gridCoordf + (float4)(voxelSize, 0,          0,          0);
      v[3] = gridCoordf + (float4)(0,         0,          0,          0);
      v[4] = gridCoordf + (float4)(0,         voxelSize,  voxelSize,  0);
      v[5] = gridCoordf + (float4)(voxelSize, voxelSize,  voxelSize,  0);
      v[6] = gridCoordf + (float4)(voxelSize, voxelSize,  0,          0);
      v[7] = gridCoordf + (float4)(0,         voxelSize,  0,          0);


      // use the above to sample the voxel corenr density values and generate case number
      // Test: output case number 0-255
      float cornerDensity[8];
      cornerDensity[0] = read_imagef(blockData, gVolumeSampler, vCoords[0]).x;
      cornerDensity[1] = read_imagef(blockData, gVolumeSampler, vCoords[1]).x;
      cornerDensity[2] = read_imagef(blockData, gVolumeSampler, vCoords[2]).x;
      cornerDensity[3] = read_imagef(blockData, gVolumeSampler, vCoords[3]).x;
      cornerDensity[4] = read_imagef(blockData, gVolumeSampler, vCoords[4]).x;
      cornerDensity[5] = read_imagef(blockData, gVolumeSampler, vCoords[5]).x;
      cornerDensity[6] = read_imagef(blockData, gVolumeSampler, vCoords[6]).x;
      cornerDensity[7] = read_imagef(blockData, gVolumeSampler, vCoords[7]).x;


      // Now get the density of the neighbours and create the gradients
      float4 cornerGrads[8];
      
      CreateCornerGradient(cornerGrads, vCoords, cornerDensity, blockData, (float)blockSize);

      // Vertex list on each edge
      float4 vertList[12];
      float4 gradList[12];

      if (edgeCase & 1)
      {
        vertList[0] = VertexInterp(isoValue,  v[0], v[1], cornerDensity[0], cornerDensity[1]);
        gradList[0] = VertexInterp2(isoValue, cornerGrads[0], cornerGrads[1], cornerDensity[0], cornerDensity[1]);
      }

      if (edgeCase & 2)                      
      {
        vertList[1] = VertexInterp(isoValue,  v[1], v[2], cornerDensity[1], cornerDensity[2]);
        gradList[1] = VertexInterp2(isoValue, cornerGrads[1], cornerGrads[2], cornerDensity[1], cornerDensity[2]);
      }

      if (edgeCase & 4)                      
      {
        vertList[2] = VertexInterp(isoValue,  v[2], v[3], cornerDensity[2], cornerDensity[3]);
        gradList[2] = VertexInterp2(isoValue, cornerGrads[2], cornerGrads[3], cornerDensity[2], cornerDensity[3]);
      }

      if (edgeCase & 8)                    
      {
        vertList[3] = VertexInterp(isoValue,  v[3], v[0], cornerDensity[3], cornerDensity[0]);
        gradList[3] = VertexInterp2(isoValue, cornerGrads[3], cornerGrads[0], cornerDensity[3], cornerDensity[0]);
      }

      if (edgeCase & 16)                
      {
        vertList[4] = VertexInterp(isoValue,  v[4], v[5], cornerDensity[4], cornerDensity[5]);
        gradList[4] = VertexInterp2(isoValue, cornerGrads[4], cornerGrads[5], cornerDensity[4], cornerDensity[5]);
      }

      if (edgeCase & 32)
      {
        vertList[5] = VertexInterp(isoValue,  v[5], v[6], cornerDensity[5], cornerDensity[6]);
        gradList[5] = VertexInterp2(isoValue, cornerGrads[5], cornerGrads[6], cornerDensity[5], cornerDensity[6]);
      }

      if (edgeCase & 64)
      {
        vertList[6] = VertexInterp(isoValue,  v[6], v[7], cornerDensity[6], cornerDensity[7]);
        gradList[6] = VertexInterp2(isoValue, cornerGrads[6], cornerGrads[7], cornerDensity[6], cornerDensity[7]);
      }

      if (edgeCase & 128)
      {
        vertList[7] = VertexInterp(isoValue,  v[7], v[4], cornerDensity[7], cornerDensity[4]);
        gradList[7] = VertexInterp2(isoValue, cornerGrads[7], cornerGrads[4], cornerDensity[7], cornerDensity[4]);
      }

      if (edgeCase & 256)
      {
        vertList[8] = VertexInterp(isoValue,  v[0], v[4], cornerDensity[0], cornerDensity[4]);
        gradList[8] = VertexInterp2(isoValue, cornerGrads[0], cornerGrads[4], cornerDensity[0], cornerDensity[4]);
      }

      if (edgeCase & 512)
      {
        vertList[9] = VertexInterp(isoValue,  v[1], v[5], cornerDensity[1], cornerDensity[5]);
        gradList[9] = VertexInterp2(isoValue, cornerGrads[1], cornerGrads[5], cornerDensity[1], cornerDensity[5]);
      }

      if (edgeCase & 1024)
      {
        vertList[10] = VertexInterp(isoValue, v[2], v[6], cornerDensity[2], cornerDensity[6]);
        gradList[10] = VertexInterp2(isoValue, cornerGrads[2], cornerGrads[6], cornerDensity[2], cornerDensity[6]);
      }

      if (edgeCase & 2048)
      {
        vertList[11] = VertexInterp(isoValue, v[3], v[7], cornerDensity[3], cornerDensity[7]);
        gradList[11] = VertexInterp2(isoValue, cornerGrads[3], cornerGrads[7], cornerDensity[3], cornerDensity[7]);
      }

      // Generate the actual triangles
      uint vertOffset = vertBufferOffset[voxelID];
      uint ttIndex = (voxelCase>0)*((voxelCase)*16);        // TriangleTable index -- TODO image2D sampler

      // Generate the actual triangles
      for(int ttI=0; triTable[ttIndex+ttI] != -1; ttI+=1)
      {
        pos[vertOffset + ttI]  = vertList[triTable[ttIndex + ttI]]; // Position
        norm[vertOffset + ttI] = normalize(gradList[triTable[ttIndex + ttI]]);//(float4)(0.5f, 1.0f, 0.5f, 0);        // Normal
      }
    }
  }                                                                            
}

__kernel void ClearBuffers(
                __global uint* numVertsToGenerate,
                __global uint* vertBufferOffset,
                __global uint* activeVoxels,
                uint numVoxels)
{
  uint index = get_global_id(0);

  if(index < numVoxels)
  {
    numVertsToGenerate[index] = 0;
    vertBufferOffset[index]   = 0;
    activeVoxels[index]       = 0;
  }
}