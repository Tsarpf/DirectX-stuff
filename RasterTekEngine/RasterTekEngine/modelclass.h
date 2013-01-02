#ifndef _MODELCLASS_H_
#define _MODELCLASS_H_

#include <D3D11.h>
#include <D3DX10math.h>


#include "textureclass.h"


class ModelClass
{
private:
	struct VertexType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texture;
	};

	struct InstanceType
	{
		D3DXVECTOR3 position;
	};

public:
	ModelClass();
	ModelClass(const ModelClass&);
	~ModelClass();


	bool Initialize(ID3D11Device*, WCHAR*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetVertexCount();
	int GetInstanceCount();

	ID3D11ShaderResourceView * GetTexture();

private:
	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadTexture(ID3D11Device*, WCHAR*);
	void ReleaseTexture();


private:
	ID3D11Buffer *m_vertexBuffer, *m_instanceBuffer;
	int m_vertexCount, m_instanceCount;

	TextureClass * m_Texture;
};

#endif // !_MODELCLASS_H_
