#ifndef GENERICBLOCK_H_
#define GENERICBLOCK_H_

enum blockType : char
{
	AIR = 0,
	DIRT = 1,
	GRASS = 2,
	STONE = 3
};

struct block
{
	blockType typepenis;
};


#endif