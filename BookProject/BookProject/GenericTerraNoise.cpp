#include "GenericTerraNoise.h"
#include "accidentalnoise\include\anl.h"

#include <iostream>
#include <fstream>
#include <windows.h>
#include <noise/noise.h>
#include "block.h"

#include <vector>


GenericTerraNoise::GenericTerraNoise(int x, int y, int z) : xSize(x), ySize(y), zSize(z)
	
											// : ground_shape_fractal(anl::BILLOW, anl::GRADIENT, anl::QUINTIC),
										 //auto_correct(0,25),
										 //ground_scale(0.125, -0.45)
{
	/*
		anl::CImplicitFractal ground_shape_fractal(anl::FBM, anl::GRADIENT, anl::QUINTIC);
		anl::CImplicitAutoCorrect auto_correct(-1,1);
		anl::CImplicitScaleOffset ground_scale(0.25, 0);

		anl::CImplicitGradient ground_gradient;
		anl::CImplicitScaleDomain ground_scale_y;
		anl::CImplicitTranslateDomain ground_perturb;
		anl::CImplicitSelect ground_select;
		*/
}

GenericTerraNoise::~GenericTerraNoise()
{
	delete [] blockArrayPointer;
}

void GenericTerraNoise::fillBlockArray()
{
	blockArrayPointer = new blockType[xSize * ySize * ySize];

	noise::module::Perlin perlin;
	perlin.SetOctaveCount(6);
	perlin.SetFrequency(0.25);

	for(float x = 0; x < xSize; x++)
	{
		for(float y = 0; y < ySize; y++)
		{
			for(float z = 0; z < zSize; z++)
			{

				float noiseValue = perlin.GetValue(x/10,y/10,z/10);
				int idx = x + y*xSize + z*xSize*ySize;

				if(noiseValue >= 0.8f && noiseValue <= 0.9f && y < chunkSizeY / 2)
				{
					blockArrayPointer[idx] = DIRT;
				}
				else if(noiseValue > 0.9f && y < chunkSizeY / 2)
				{
					blockArrayPointer[idx] = STONE;
				}
				else
				{
					blockArrayPointer[idx] = AIR;
				}
			}
		}
	}
}

blockType GenericTerraNoise::blockTypeAtLocation(int x, int y, int z)
{
	if(x < 0 || y < 0 || z < 0)
	{
		return AIR;
	}
	if(x > chunkSizeX || y > chunkSizeY || z > chunkSizeZ)
		return AIR;
	int idx = x + y*xSize + z*xSize*ySize;
	return blockArrayPointer[idx];
	//blockType type = (blockType)blockArrayPointer[idx];
	//return blockArrayPointer[idx];
} 
/*
Build one mesh per chunk per texture
 * Add all cubesides that have the same texture to the same mesh, so they can be drawn with one drawcall
 * Make a list of lists, add a list for each new texture
 * Use an array of TextureIdx's (enum) to keep track of which textures are present in the chunk
 *penis
*/
void GenericTerraNoise::buildCubeSideDrawLists()
{
	//FIRST, find out what should we save to dem lists.
	//We want a "mesh" (in unity terms) that has all the cubesides of a chunk in one list.
	//sooooo.... we need 4 vertices(with positions relative to chunk) per side
	//
	for(int x = 0; x < chunkSizeX; x++) //Not sure why we're looping here again
    {									//SHould probably include this stuff in fillBlockArray(...)'s loop
        for(int y = 0; y < chunkSizeY; y++)
        {
            for(int z = 0; z < chunkSizeZ; z++)
			{
				blockType bt = blockTypeAtLocation(x,y,z);
				if(bt == AIR)
					continue;

				BlockNeighbours bn = GetBlockNeighbours(x,y,z);
				if(bn.xPlus != AIR)
				{
					penislistTJSP.add(getSide(xPlus));
				}
			}
		}
	}
}

BlockNeighbours GenericTerraNoise::GetBlockNeighbours(int x, int y, int z)
{
	BlockNeighbours bn;
	bn.xPlus = blockTypeAtLocation(x+1,y,z);
	bn.xMinus = blockTypeAtLocation(x-1,y,z);

	bn.yPlus = blockTypeAtLocation(x,y+1,z);
	bn.yMinus = blockTypeAtLocation(x,y-1,z);

	bn.zPlus = blockTypeAtLocation(x,y,z+1);
	bn.zMinus = blockTypeAtLocation(x,y,z-1);

	return bn;
}

CubeSideWithTextureIdx GenericTerraNoise::getSide(blockSideIdx side)
{
	CubeSideWithTextureIdx cswti;

	if(side == xPlus)
	{
		cswti.topLeft	   = Vertex(0.5f,-0.5f,-0.5f,         1.0f,0.0f, 0.0f,  0,1);
		cswti.topRight	   = Vertex(0.5f,-0.5f,0.5f,      1.0f, 0.0f, 0.0f,  1,1);
		cswti.bottomLeft   = Vertex(0.5f,0.5f,-0.5f,      1.0f, 0.0f, 0.0f,  0,0);
		cswti.bottomRight  = Vertex(0.5f,0.5f,0.5f,   1.0f, 0.0f, 0.0f,  1,0);
	}

	if(side == xMinus)
	{
		cswti.topLeft	  = Vertex(-0.5f,-0.5f,0.5f,        -1.0f, 0.0f, 0.0f,  0,1);
		cswti.topRight	  = Vertex(-0.5f,-0.5f,-0.5f,           -1.0f, 0.0f, 0.0f,  1,1);
		cswti.bottomLeft  = Vertex(-0.5f,0.5f,0.5f,     -1.0f, 0.0f, 0.0f,  0,0);
		cswti.bottomRight = Vertex(-0.5f,0.5f,-0.5f,        -1.0f, 0.0f, 0.0f,  1,0);
	}

	if(side == yPlus)
	{
		cswti.topLeft	   = Vertex(-0.5f,0.5f,-0.5f,        0.0f, 1.0f, 0.0f,  0,1);
		cswti.topRight	   = Vertex(0.5f,0.5f,-0.5f,     0.0f, 1.0f, 0.0f,  1,1);
		cswti.bottomLeft   = Vertex(-0.5f,0.5f,0.5f,     0.0f, 1.0f, 0.0f,  0,0);
		cswti.bottomRight  = Vertex(0.5f,0.5f,0.5f,  0.0f, 1.0f, 0.0f,  1,0);
	}

	if(side == yMinus)
	{
		cswti.topLeft	 = Vertex(-0.5f,-0.5f,0.5f,         0.0f, -1.0f, 0.0f,  0,1);
		cswti.topRight	 = Vertex(0.5f,-0.5f,0.5f,      0.0f, -1.0f, 0.0f,  1,1);
		cswti.bottomLeft  = Vertex(-0.5f,-0.5f,-0.5f,           0.0f, -1.0f, 0.0f, 0,0);
		cswti.bottomRight = Vertex(0.5f,-0.5f,-0.5f,        0.0f, -1.0f, 0.0f, 1,0);
	}

	if(side == zPlus)
	{
		cswti.topLeft	  = Vertex(0.5f,-0.5f,0.5f,     0.0f, 0.0f, 1.0f,  0,1);
		cswti.topRight	  = Vertex(-0.5f,-0.5f,0.5f,        0.0f, 0.0f, 1.0f,  1,1);
		cswti.bottomLeft  = Vertex(0.5f,0.5f,0.5f,  0.0f, 0.0f, 1.0f,  0,0);
		cswti.bottomRight = Vertex(-0.5f,0.5f,0.5f,     0.0f, 0.0f, 1.0f,  1,0);
	}

	if(side == zMinus)
	{
		cswti.topLeft	    = Vertex(-0.5f,-0.5f,-0.5f,            0.0f, 0.0f, -1.0f, 0,1);
		cswti.topRight	    = Vertex(0.5f,-0.5f,-0.5f,         0.0f, 0.0f, -1.0f, 1,1);
		cswti.bottomLeft    = Vertex(-0.5f,0.5f,-0.5f,         0.0f, 0.0f, -1.0f, 0,0);
		cswti.bottomRight   = Vertex(0.5f,0.5f,-0.5f,      0.0f, 0.0f, -1.0f, 1,0);
	}
	return cswti;
}


/*
    Vertex v[24];

    //Front zMinus
    v[0] = Vertex(-0.5f,-0.5f,-0.5f,            0.0f, 0.0f, -1.0f, 0,1);
    v[1] = Vertex(0.5f,-0.5f,-0.5f,         0.0f, 0.0f, -1.0f, 1,1);
    v[2] = Vertex(-0.5f,0.5f,-0.5f,         0.0f, 0.0f, -1.0f, 0,0);
    v[3] = Vertex(0.5f,0.5f,-0.5f,      0.0f, 0.0f, -1.0f, 1,0);
    //Right xPlus
    v[4] = Vertex(0.5f,-0.5f,-0.5f,         1.0f,0.0f, 0.0f,  0,1);
    v[5] = Vertex(0.5f,-0.5f,0.5f,      1.0f, 0.0f, 0.0f,  1,1);
    v[6] = Vertex(0.5f,0.5f,-0.5f,      1.0f, 0.0f, 0.0f,  0,0);
    v[7] = Vertex(0.5f,0.5f,0.5f,   1.0f, 0.0f, 0.0f,  1,0);
    //Bottom yMinus
    v[8] = Vertex(-0.5f,-0.5f,0.5f,         0.0f, -1.0f, 0.0f,  0,1);
    v[9] = Vertex(0.5f,-0.5f,0.5f,      0.0f, -1.0f, 0.0f,  1,1);
    v[10] = Vertex(-0.5f,-0.5f,-0.5f,           0.0f, -1.0f, 0.0f, 0,0);
    v[11] = Vertex(0.5f,-0.5f,-0.5f,        0.0f, -1.0f, 0.0f, 1,0);
    //Back zPlus
    v[12] = Vertex(0.5f,-0.5f,0.5f,     0.0f, 0.0f, 1.0f,  0,1);
    v[13] = Vertex(-0.5f,-0.5f,0.5f,        0.0f, 0.0f, 1.0f,  1,1);
    v[14] = Vertex(0.5f,0.5f,0.5f,  0.0f, 0.0f, 1.0f,  0,0);
    v[15] = Vertex(-0.5f,0.5f,0.5f,     0.0f, 0.0f, 1.0f,  1,0);
    //Left xMinus
    v[16] = Vertex(-0.5f,-0.5f,0.5f,        -1.0f, 0.0f, 0.0f,  0,1);
    v[17] = Vertex(-0.5f,-0.5f,-0.5f,           -1.0f, 0.0f, 0.0f,  1,1);
    v[18] = Vertex(-0.5f,0.5f,0.5f,     -1.0f, 0.0f, 0.0f,  0,0);
    v[19] = Vertex(-0.5f,0.5f,-0.5f,        -1.0f, 0.0f, 0.0f,  1,0);
    //Top yPlus
    v[20] = Vertex(-0.5f,0.5f,-0.5f,        0.0f, 1.0f, 0.0f,  0,1);
    v[21] = Vertex(0.5f,0.5f,-0.5f,     0.0f, 1.0f, 0.0f,  1,1);
    v[22] = Vertex(-0.5f,0.5f,0.5f,     0.0f, 1.0f, 0.0f,  0,0);
    v[23] = Vertex(0.5f,0.5f,0.5f,  0.0f, 1.0f, 0.0f,  1,0);
*/


void GenericTerraNoise::HihiiKakka()
{
	   anl::CMWC4096 rnd;
    rnd.setSeedTime();
    anl::CImplicitFractal frac1(anl::FBM, anl::GRADIENT, anl::QUINTIC);
    anl::CImplicitFractal frac2(anl::FBM, anl::GRADIENT, anl::QUINTIC);
    anl::CImplicitFractal frac3(anl::FBM, anl::GRADIENT, anl::QUINTIC);
    anl::CImplicitFractal frac4(anl::RIDGEDMULTI, anl::GRADIENT, anl::QUINTIC);
    anl::CImplicitFractal frac5(anl::FBM, anl::GRADIENT, anl::QUINTIC);
    anl::CImplicitFractal frac6(anl::FBM, anl::GRADIENT, anl::QUINTIC);
    anl::CImplicitFractal frac7(anl::FBM, anl::GRADIENT, anl::QUINTIC);

    frac1.setSeed(rnd.get());
    frac2.setSeed(rnd.get());
    frac3.setSeed(rnd.get());
    frac4.setSeed(rnd.get());
    frac5.setSeed(rnd.get());
    frac6.setSeed(rnd.get());
    frac7.setSeed(rnd.get());


    anl::CImplicitAutoCorrect ac1(0.0, 1.0), ac2(0.0,1.0), ac3(0,1.0),
		ac4(0.0, 360.0), ac5(-1.0,1.0), ac6(-1.0,1.0), ac7(-1.0,1.0);
    ac1.setSource(&frac1);
    ac2.setSource(&frac2);
    ac3.setSource(&frac3);
    ac4.setSource(&frac4);
    ac5.setSource(&frac5);
    ac6.setSource(&frac6);
    ac7.setSource(&frac7);

    anl::CRGBACompositeChannels compose1(anl::RGB);

    compose1.setRedSource(&ac1);
    compose1.setGreenSource(&ac2);
    compose1.setBlueSource(&ac3);
    compose1.setAlphaSource(1.0);


    anl::CRGBARotateColor rot;
    rot.setAngle(&ac4);
    rot.setAxisX(&ac5);
    rot.setAxisY(&ac6);
    rot.setAxisZ(&ac7);
    rot.setNormalizeAxis(true);
    rot.setSource(&compose1);

	
}


float GenericTerraNoise::getBasicTerrain(float x, float y, float z)
{

	noise::module::Perlin perlin;
	perlin.SetOctaveCount(6);
	perlin.SetFrequency(0.25);
	noise::module::Multiply kakka;

	float noiseValue = perlin.GetValue(x/10,y/10,z/10);

	return noiseValue;

}

void GenericTerraNoise::drawBMP()
{
	 HANDLE file;
     BITMAPFILEHEADER fileHeader;
     BITMAPINFOHEADER fileInfo;
     RGBTRIPLE *image;
     DWORD write = 0;
     image = new RGBTRIPLE[512*512];
 
     file = CreateFile(L"example.bmp",GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);  //Sets up the new bmp to be written to
 
	 //Sets our type to BM or bmp
     fileHeader.bfType = 19778;      

     //Sets the size equal to the size of the header struct
     fileHeader.bfSize = sizeof(fileHeader.bfOffBits) + sizeof(RGBTRIPLE);

	 //sets the reserves to 0
     fileHeader.bfReserved1 = 0;   
     fileHeader.bfReserved2 = 0;

	 //Sets offbits equal to the size of file and info header
     fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);   
 
     fileInfo.biSize = sizeof(BITMAPINFOHEADER);
     fileInfo.biWidth = 512;
     fileInfo.biHeight = 512;
     fileInfo.biPlanes = 1;
     fileInfo.biBitCount = 24;
     fileInfo.biCompression = BI_RGB;
	 fileInfo.biSizeImage = 512 * 512 * sizeof(RGBTRIPLE);
     fileInfo.biXPelsPerMeter = 2400;
     fileInfo.biYPelsPerMeter = 2400;
     fileInfo.biClrImportant = 0;
     fileInfo.biClrUsed = 0;
 
     WriteFile(file,&fileHeader,sizeof(fileHeader),&write,NULL);
     WriteFile(file,&fileInfo,sizeof(fileInfo),&write,NULL);
 
     for (int i = 0; i < 512*512; i++)
     {
         image[i].rgbtBlue = 255;
         image[i].rgbtGreen = 0;
         image[i].rgbtRed = 0;
     }
 
     WriteFile(file, image, fileInfo.biSizeImage, &write, NULL);
 
     CloseHandle(file);
 
}

float GenericTerraNoise::getFractal(float x, float y, float z)
{
	
	using namespace anl;

	

	CImplicitGradient ground_gradient;
	ground_gradient.setGradient(0,0,0,1);

	CImplicitFractal ground_shape_fractal(FBM, GRADIENT, QUINTIC);
	ground_shape_fractal.setNumOctaves(6);
	ground_shape_fractal.setFrequency(2);

	CImplicitScaleOffset ground_scale(0.5, 0);
	ground_scale.setSource(&ground_shape_fractal);

	CImplicitScaleDomain ground_scale_y;
	ground_scale_y.setSource(&ground_scale);
	ground_scale_y.setYScale(0.0);

	CImplicitTranslateDomain ground_perturb;
	ground_perturb.setSource(&ground_gradient);
	ground_perturb.setYAxisSource(&ground_scale_y);


	CImplicitSelect ground_select;
	ground_select.setLowSource(0.0);
	ground_select.setHighSource(1);
	ground_select.setThreshold(0.5);
	ground_select.setControlSource(&ground_perturb);


	return ground_select.get(x,y,z);
}