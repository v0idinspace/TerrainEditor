/**
----------------------------------------------------------------------------
  File:              noise.cl

  Language:          C++

  License:           

  Author:            Tasos Giannakopoulos (dotvoidd@gmail.com)
  Copyright:         
  Date:              01 Nov 2012

  Description:       Voxel Terrain Generation system

----------------------------------------------------------------------------
*/


#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable


// Cube offsets
__constant int4 cubeOffset[8] = 
{
	{0, 0, 0, 0}, 
	{1, 0, 0, 0}, 
	{0, 0, 1, 0}, 
	{1, 0, 1, 0},
	{0, 1, 0, 0},
	{1, 1, 0, 0},
	{0, 1, 1, 0},
	{1, 1, 1, 0},
}; 

// ------------------------------------------------------------------------
// NOISE, fBm 

#define ONE_F1                 (1.0f)
#define ZERO_F1                (0.0f)
 
//#define USE_IMAGES_FOR_RESULTS (1)  // NOTE: It may be faster to use buffers instead of images
 
static const float4 ZERO_F4 = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
static const float4 ONE_F4 = (float4)(1.0f, 1.0f, 1.0f, 1.0f);
  
__constant int P_MASK = 255;
__constant int P_SIZE = 256;
__constant int P[512] = {
151,160,137,91,90,15,
  131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
  190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
  88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
  77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
  102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
  135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
  5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
  223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
  129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
  251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
  49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
  138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
  151,160,137,91,90,15,
  131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
  190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
  88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
  77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
  102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
  135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
  5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
  223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
  129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
  251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
  49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
  138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,  
};
/*
225, 155, 210, 108, 175, 199, 221, 144, 203, 116, 70, 213, 69, 158, 33, 252,
5, 82, 173, 133, 222, 139, 174, 27, 9, 71, 90, 246, 75, 130, 91, 191,
169, 138, 2, 151, 194, 235, 81, 7, 25, 113, 228, 159, 205, 253, 134, 142,
248, 65, 224, 217, 22, 121, 229, 63, 89, 103, 96, 104, 156, 17, 201, 129,
36, 8, 165, 110, 237, 117, 231, 56, 132, 211, 152, 20, 181, 111, 239, 218,
170, 163, 51, 172, 157, 47, 80, 212, 176, 250, 87, 49, 99, 242, 136, 189,
162, 115, 44, 43, 124, 94, 150, 16, 141, 247, 32, 10, 198, 223, 255, 72,
53, 131, 84, 57, 220, 197, 58, 50, 208, 11, 241, 28, 3, 192, 62, 202,
18, 215, 153, 24, 76, 41, 15, 179, 39, 46, 55, 6, 128, 167, 23, 188,
106, 34, 187, 140, 164, 73, 112, 182, 244, 195, 227, 13, 35, 77, 196, 185,
26, 200, 226, 119, 31, 123, 168, 125, 249, 68, 183, 230, 177, 135, 160, 180,
12, 1, 243, 148, 102, 166, 38, 238, 251, 37, 240, 126, 64, 74, 161, 40,
184, 149, 171, 178, 101, 66, 29, 59, 146, 61, 254, 107, 42, 86, 154, 4,
236, 232, 120, 21, 233, 209, 45, 98, 193, 114, 78, 19, 206, 14, 118, 127,
48, 79, 147, 85, 30, 207, 219, 54, 88, 234, 190, 122, 95, 67, 143, 109,
137, 214, 145, 93, 92, 100, 245, 0, 216, 186, 60, 83, 105, 97, 204, 52
*/
 
__constant int G_MASK = 15;
__constant int G_SIZE = 16;
__constant int G_VECSIZE = 4;
__constant float G[16*4] = 
{
  +ONE_F1,  +ONE_F1, +ZERO_F1, +ZERO_F1, 
  -ONE_F1,  +ONE_F1, +ZERO_F1, +ZERO_F1, 
  +ONE_F1,  -ONE_F1, +ZERO_F1, +ZERO_F1, 
  -ONE_F1,  -ONE_F1, +ZERO_F1, +ZERO_F1,
  +ONE_F1, +ZERO_F1,  +ONE_F1, +ZERO_F1, 
  -ONE_F1, +ZERO_F1,  +ONE_F1, +ZERO_F1, 
  +ONE_F1, +ZERO_F1,  -ONE_F1, +ZERO_F1, 
  -ONE_F1, +ZERO_F1,  -ONE_F1, +ZERO_F1,
  +ZERO_F1,  +ONE_F1,  +ONE_F1, +ZERO_F1, 
  +ZERO_F1,  -ONE_F1,  +ONE_F1, +ZERO_F1, 
  +ZERO_F1,  +ONE_F1,  -ONE_F1, +ZERO_F1, 
  +ZERO_F1,  -ONE_F1,  -ONE_F1, +ZERO_F1,
  +ONE_F1,  +ONE_F1, +ZERO_F1, +ZERO_F1, 
  -ONE_F1,  +ONE_F1, +ZERO_F1, +ZERO_F1, 
  +ZERO_F1,  -ONE_F1,  +ONE_F1, +ZERO_F1, 
  +ZERO_F1,  -ONE_F1,  -ONE_F1, +ZERO_F1
};


// functions
int lattice3d(int4 i)
{
  return P[i.x + P[i.y + P[i.z]]];
}

float mix1d(float a, float b, float t)
{
  float ba = b - a;
  float tba = t * ba;
  float atba = a + tba;
  return atba;    
}
 
float2 mix2d(float2 a, float2 b, float t)
{
  float2 ba = b - a;
  float2 tba = t * ba;
  float2 atba = a + tba;
  return atba;    
}
 
float4 mix3d(float4 a, float4 b, float t)
{
  float4 ba = b - a;
  float4 tba = t * ba;
  float4 atba = a + tba;
  return atba;    
}

float smooth(float t)
{
  return t*t*t*(t*(t*6.0f-15.0f)+10.0f); 
}

float gradient3d(int4 i, float4 v)
{
  int index = (lattice3d(i) & G_MASK) * G_VECSIZE;
  float4 g = (float4)(G[index + 0], G[index + 1], G[index + 2], 1.0f);
  return dot(v, g);
}


// Signed gradient noise 3d
float sgnoise3d(float4 position)
{
  float4  p   = position;
  float4  pf  = floor(p);
  int4    ip  = (int4)((int)pf.x, (int)pf.y, (int)pf.z, 0);
  float4  fp  = p - pf;        
  ip &= P_MASK;
 
  int4 I000 = (int4)(0, 0, 0, 0);
  int4 I001 = (int4)(0, 0, 1, 0);  
  int4 I010 = (int4)(0, 1, 0, 0);
  int4 I011 = (int4)(0, 1, 1, 0);
  int4 I100 = (int4)(1, 0, 0, 0);
  int4 I101 = (int4)(1, 0, 1, 0);
  int4 I110 = (int4)(1, 1, 0, 0);
  int4 I111 = (int4)(1, 1, 1, 0);
    
  float4 F000 = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
  float4 F001 = (float4)(0.0f, 0.0f, 1.0f, 0.0f);
  float4 F010 = (float4)(0.0f, 1.0f, 0.0f, 0.0f);
  float4 F011 = (float4)(0.0f, 1.0f, 1.0f, 0.0f);
  float4 F100 = (float4)(1.0f, 0.0f, 0.0f, 0.0f);
  float4 F101 = (float4)(1.0f, 0.0f, 1.0f, 0.0f);
  float4 F110 = (float4)(1.0f, 1.0f, 0.0f, 0.0f);
  float4 F111 = (float4)(1.0f, 1.0f, 1.0f, 0.0f);
    
  float n000 = gradient3d(ip + I000, fp - F000);
  float n001 = gradient3d(ip + I001, fp - F001);
    
  float n010 = gradient3d(ip + I010, fp - F010);
  float n011 = gradient3d(ip + I011, fp - F011);
    
  float n100 = gradient3d(ip + I100, fp - F100);
  float n101 = gradient3d(ip + I101, fp - F101);
 
  float n110 = gradient3d(ip + I110, fp - F110);
  float n111 = gradient3d(ip + I111, fp - F111);
 
  float4 n40 = (float4)(n000, n001, n010, n011);
  float4 n41 = (float4)(n100, n101, n110, n111);
 
  float4 n4 = mix3d(n40, n41, smooth(fp.x));
  float2 n2 = mix2d(n4.xy, n4.zw, smooth(fp.y));
  float n = mix1d(n2.x, n2.y, smooth(fp.z));

  return n * (1.0f / 0.7f);
}

float sTiledGradientNoise3D(float4 position, float tileEvery, float frequency) // amplitude -- if multifractal
{
  float w   = tileEvery;

  float x   = position.x;
  float y   = position.y;
  float z   = position.z;
  float wx  = w - x;
  float wy  = w - y;
  float wz  = w - z;
  float xw  = x - w;
  float yw  = y - w;
  float zw  = z - w;
    
  float value = 0;                                                                                  // * amplitude
  value =  (sgnoise3d((float4)(x,   y,  z,  0)  * frequency) * wx  * wy  * wz  +                    // * amplitude
            sgnoise3d((float4)(xw,  y,  z,  0)  * frequency) * x   * wy  * wz  +                    // * amplitude
            sgnoise3d((float4)(x,   yw, z,  0)  * frequency) * wx  * y   * wz  +                    // * amplitude
            sgnoise3d((float4)(x,   y,  zw, 0)  * frequency) * wx  * wy  * z   +                    // * amplitude
            sgnoise3d((float4)(xw,  yw, z,  0)  * frequency) * x   * y   * wz  +                    // * amplitude
            sgnoise3d((float4)(xw,  y,  zw, 0)  * frequency) * x   * wy  * z   +                    // * amplitude
            sgnoise3d((float4)(x,   yw, zw, 0)  * frequency) * wx  * y   * z   +                    // * amplitude
            sgnoise3d((float4)(xw,  yw, zw, 0)  * frequency) * x   * y   * z    ) / (pow(w, 3));    // 

  return value;
}
// ------------------------------------------------------------------------

// 3D noise: multifractal multioctave 3D noise
__kernel void GradientNoiseImage3D(
                __write_only image3d_t destImage, 
                const float frequency,
                const uint texSize)
{
  int xCoord = get_global_id(0);  // returns the global id of this work-item 
  int yCoord = get_global_id(1);  // in two dimensions

  for(int i = 0; i < texSize; i++)
  {
    int4 coord  = (int4)(xCoord, i, yCoord, 0);

    //int4 size   = (int4)(get_global_size(0), texSize, get_global_size(1), 0);
    int4 size   = (int4)(texSize, texSize, texSize, 0);
 
    // Narrows down the position to [0, 1]
    float4 position = (float4)((coord.x/(float)size.x), (coord.y/(float)size.y), (coord.z/(float)size.z), 0);
    float4 sample   = position;

    // frequency = 2i
    // amplitude = persistencei
    // float octave      = 2;
    // float persistance = 0.5f;


    // tileable
    float value = 0;
    value += sTiledGradientNoise3D(position, 1.0f, frequency);
    //value += sTiledGradientNoise3D(position, 1.0f, 4.0f, 1.0f);
    //value += sTiledGradientNoise3D(position, 1.0f, 8.0f, 1.0f);

    //float4 color = (float4)(value, value, value, 1.0f);// * amplitude;
    //color.w = 1.0f;
    
    write_imagef(destImage, coord, value);
  }
}