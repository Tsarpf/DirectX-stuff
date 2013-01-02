#include "accidentalnoise\include\anl.h"
#include "block.h"
#include "Globals.h"
#include <vector>
#include "d3dApp.h"



#ifndef NOISEUTILITIES_H_
#define NOISEUTILITIES_H_

enum TextureIdx : char
{
	DirtAllSides = 0,
	StoneAllSides = 1,
	GrassTop = 2,
	GrassBottom = DirtAllSides,
	GrassSide = 3
};
struct Vertex
{
    XMFLOAT3 Pos;
    XMFLOAT3 Normal;
    XMFLOAT2 TexCoord;
	Vertex()
    {}
    Vertex( float px, float py, float pz, //position
            float nx, float ny, float nz, //normal
            float u, float v)             //texture
            : Pos(px,py,pz),
            Normal(nx, ny, nz),
            TexCoord(u,v)
    {}
};
struct CubeSideWithTextureIdx
{
	Vertex topLeft;
	Vertex topRight;
	Vertex bottomLeft;
	Vertex bottomRight;
	TextureIdx textureIdx;
};
struct BlockNeighbours
{
	blockType xPlus;
	blockType xMinus;

	blockType yPlus;
	blockType yMinus;

	blockType zPlus;
	blockType zMinus;
};
enum blockSideIdx
{
	xPlus,
	xMinus,
	yPlus,
	yMinus,
	zPlus,
	zMinus
};
class GenericTerraNoise
{
public:

	GenericTerraNoise(int x, int y, int z);
	~GenericTerraNoise();

	blockType * blockArrayPointer;
	void fillBlockArray();
	blockType blockTypeAtLocation(int x, int y, int z);
	void buildCubeSideDrawLists();
	BlockNeighbours GetBlockNeighbours(int x, int y, int z);
	CubeSideWithTextureIdx getSide(blockSideIdx side);


	float getFractal(float, float, float);
	void HihiiKakka();
	void drawBMP();
	float getBasicTerrain(float x, float y, float z);
	void initializeBasicTerrain();
private:
		const int xSize,ySize,zSize;
		//vector<

	/*
		anl::CImplicitGradient ground_gradient;
		anl::CImplicitFractal ground_shape_fractal;
		anl::CImplicitAutoCorrect auto_correct;	
		anl::CImplicitScaleOffset ground_scale;		
		anl::CImplicitScaleDomain ground_scale_y;
		anl::CImplicitTranslateDomain ground_perturb;
		anl::CImplicitSelect ground_select;
		*/
};




#endif