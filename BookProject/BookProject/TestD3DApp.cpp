


#include "d3dApp.h"
#include <Waves.h>
#include <vector>
#include "Int3.h"
#include "Effects.h"
#include <D3DX10math.h>
#include "Camera.h"
#include <xnamath.h>

#include <D2D1.h>

#include <noise/noise.h>
#include "block.h"
#include <noise/noiseutils.h>
#include "GenericTerraNoise.h";

#include "TextureArrayClass.h"

#include "Globals.h"


struct InstancedData
{
	XMFLOAT4X4 World;
	XMFLOAT4 Color;
};



class TestD3DApp : public D3DApp
{
public:
	TestD3DApp(HINSTANCE hInstance);
	~TestD3DApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildGeometryBuffers();
	void BuildFX();
	void BuildVertexLayout();
    void BuildCubeTranslationsAndRotations();
    void BuildBlockPositions();
	void BuildInstancedBuffer();
	void BuildInitialMap();

private:
    DirectionalLight mDirLights[3];
   	Material mBoxMat;

	ID3D11Buffer * BoxVertexBuffer;
	ID3D11Buffer * BoxIndexBuffer;
	ID3D11Buffer * mInstancedBuffer;

	ID3DX11Effect* mFX;
	ID3DX11EffectTechnique* mTech;
	ID3DX11EffectMatrixVariable* mfxWorldViewProj;
	ID3DX11EffectScalarVariable* mfxTotalTime;

	ID3D11InputLayout* mInputLayout;

	UINT mVisibleObjectCount;

	XMFLOAT4X4 mWorld;
	//XMFLOAT4X4 mView;
	//XMFLOAT4X4 mProj;

    XMFLOAT4X4 mTexTransform;
	XMFLOAT4X4 mBoxWorld;

    XMFLOAT3 mEyePosW;

	//float mTheta;
	//float mPhi;
	float mRadius;

    float xRotation;
    float yRotation;

	POINT mLastMousePos;

	std::vector<InstancedData> mInstancedData;

    std::vector<XMFLOAT4X4> boxes;
    std::vector<Int3> locations;
    std::vector<Float3> rotations;

    std::vector<int> randomSpeeds;
    
    ID3D11ShaderResourceView* mDiffuseMapSRV;

	TextureArrayClass * m_TextureArray;

    ID3DX11EffectShaderResourceVariable * fxDiffuseMap;

    Camera mCam;

	int mCubeCount;

};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
#if defined(DEBUG) |defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif


	TestD3DApp theApp(hInstance);


	if(!theApp.Init())
		return 0;


	return theApp.Run();
}

TestD3DApp::TestD3DApp(HINSTANCE hInstance) : D3DApp(hInstance), BoxVertexBuffer(0), BoxIndexBuffer(0), mInstancedBuffer(0),
	mFX(0), mTech(0), mfxWorldViewProj(0), mInputLayout(0), mVisibleObjectCount(0), mEyePosW(0.0f, 0.0f, 0.0f), mRadius(5.0f),
	m_TextureArray(0)
{
	mCubeCount = chunkSizeX  * chunkSizeY * chunkSizeZ;
	mMainWndCaption = L"Asdf";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

    mCam.SetPosition(0.0f, 2.0f, -15.0f);

    mDirLights[0].Ambient  = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mDirLights[0].Diffuse  = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
	mDirLights[0].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);
 
	mDirLights[1].Ambient  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[1].Diffuse  = XMFLOAT4(1.4f, 1.4f, 1.4f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 16.0f);
	mDirLights[1].Direction = XMFLOAT3(-0.707f, 0.0f, 0.707f);

    mBoxMat.Ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mBoxMat.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mWorld, I);

	XMMATRIX boxScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	XMMATRIX boxOffset = XMMatrixTranslation(0,1.0f, 0.0f);
	XMStoreFloat4x4(&mBoxWorld, XMMatrixMultiply(boxScale, boxOffset));
	XMStoreFloat4x4(&mTexTransform, I);
	
	//XMStoreFloat4x4(&mBox2, XMMatrixTranslation(-1.0f, 0, 0));

    //XMStoreFloat4x4(&mBox3, XMMatrixTranslation(1.0f, 0, 0));

    //locations.push_back(Int3(0,0,0));
    //locations.push_back(Int3(1,0,0));
    //locations.push_back(Int3(-1,0,0));   

}

TestD3DApp::~TestD3DApp()
{
	ReleaseCOM(BoxVertexBuffer);
	ReleaseCOM(BoxIndexBuffer);
	ReleaseCOM(mInstancedBuffer);
	ReleaseCOM(mFX);
	ReleaseCOM(mInputLayout);

    Effects::DestroyAll();

	delete m_TextureArray;
	m_TextureArray = 0;
}

bool TestD3DApp::Init()
{
	if(!D3DApp::Init())
		return false;

    //D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/dokuDirt.png", 0, 0, &mDiffuseMapSRV, 0);

    Effects::InitAll(md3dDevice);

	BuildGeometryBuffers();
    //BuildFX();
	BuildVertexLayout();
	BuildInstancedBuffer(); //olen ongelma atm 
	BuildInitialMap();

	m_TextureArray = new TextureArrayClass;
	
	if(!m_TextureArray->Initialize(md3dDevice, L"Textures/dokuDirt.png", L"dokuStone.png"))
	{

	}


	return true;
}

void TestD3DApp::OnResize()
{
	D3DApp::OnResize();


    mCam.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	//XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	//XMStoreFloat4x4(&mProj, P);
}

void TestD3DApp::UpdateScene(float dt)
{
    if(GetAsyncKeyState('W') & 0x8000)
        mCam.Walk(10.0f*dt);
    if(GetAsyncKeyState('S') & 0x8000)
        mCam.Walk(-10.0f*dt);
    if(GetAsyncKeyState('A') & 0x8000)
        mCam.Strafe(-10.0f*dt);
    if(GetAsyncKeyState('D') & 0x8000)
        mCam.Strafe(10.0f*dt);
    
		/*
		mCam.UpdateViewMatrix();
		mVisibleObjectCount = 0;

		D3D11_MAPPED_SUBRESOURCE mappedData; 
		md3dImmediateContext->Map(mInstancedBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);

		InstancedData* dataView = reinterpret_cast<InstancedData*>(mappedData.pData);

		for(UINT i = 0; i < mInstancedData.size(); ++i)
		{
			dataView[mVisibleObjectCount++] = mInstancedData[i];
		}

		md3dImmediateContext->Unmap(mInstancedBuffer, 0);
		*/

		mCam.UpdateViewMatrix();
		D3D11_MAPPED_SUBRESOURCE mappedData;
		md3dImmediateContext->Map(mInstancedBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);

		InstancedData * dataView = reinterpret_cast<InstancedData*>(mappedData.pData);

		int i = 0;
		for(std::vector<InstancedData>::iterator it = mInstancedData.begin(); it != mInstancedData.end(); it++)
		{
			dataView[i] = *it; //->World = //XMscalar;
			i++;
		}

		md3dImmediateContext->Unmap(mInstancedBuffer,0);
    //After this it's all about turning em cubes.
    /*
    int i = 0;
    for(std::vector<Int3>::iterator it = locations.begin(); it != locations.end(); ++it)
    {
        rotations[i].x += D3DX_PI / 180.0f;
        rotations[i].y += D3DX_PI / 180.0f;

        rotations[i].x = XMScalarSin(rotations[i].x) * dt * randomSpeeds[i];
        rotations[i].y = XMScalarSin(rotations[i].y) * dt * randomSpeeds[i];

        XMMATRIX xRotationMatrix, yRotationMatrix, baseMatrix;

        xRotationMatrix = XMMatrixRotationX(rotations[i].x);
        yRotationMatrix = XMMatrixRotationY(rotations[i].y);

        XMMATRIX translation = XMLoadFloat4x4(&boxes[i]);
        XMMATRIX newWorld;// = XMMatrixIdentity();

        //XMStoreFloat4x4(&placeholder, XMMatrixTranslation((*it).x - 0.5f, (*it).y - 0.5f, (*it).z - 0.5f));
        newWorld = XMMatrixTranslation(-0.5f,-0.5f,-0.5f);

        newWorld = xRotationMatrix * yRotationMatrix * translation;

        XMStoreFloat4x4(&boxes[i], newWorld);
        i++;
    }
    */
}

void TestD3DApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(mInputLayout);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride[2] = {sizeof(Vertex), sizeof(InstancedData)};
	UINT offset[2] = {0,0};

	ID3D11Buffer * vbs[2] = {BoxVertexBuffer, mInstancedBuffer};
	
    mCam.UpdateViewMatrix();

	XMMATRIX view     = mCam.View();
	XMMATRIX proj     = mCam.Proj();
	XMMATRIX viewProj = mCam.ViewProj();

    Effects::BasicFX->SetDirLights(mDirLights);
	Effects::BasicFX->SetEyePosW(mEyePosW);

	//mfxTotalTime->SetFloat(mTimer.TotalTime());
    
    ID3DX11EffectTechnique* activeTech = Effects::BasicFX->Light2TexTech;

    D3DX11_TECHNIQUE_DESC techDesc;
    activeTech->GetDesc( &techDesc );
    for(UINT p = 0; p < techDesc.Passes; ++p)
    {
			md3dImmediateContext->IASetVertexBuffers(0, 2, vbs, stride, offset);
			md3dImmediateContext->IASetIndexBuffer(BoxIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

			XMMATRIX world = XMLoadFloat4x4(&mBoxWorld);
			XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
			XMMATRIX worldViewProj = world*view*proj;

            Effects::BasicFX->SetWorld(world);
		    Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		    Effects::BasicFX->SetWorldViewProj(worldViewProj);
		    Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&mTexTransform));
		    Effects::BasicFX->SetMaterial(mBoxMat);
		    //Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRV);
			md3dImmediateContext->PSSetShaderResources(0,2,  m_TextureArray->GetTextureArray());
			Effects::BasicFX->SetTextureArrayIndex(2);

			activeTech->GetPassByIndex(p)->Apply(0,md3dImmediateContext);
			md3dImmediateContext->DrawIndexedInstanced(36, mCubeCount, 0, 0, 0);
		/*
		int i = 0;
        for(std::vector<Int3>::iterator it = locations.begin(); it != locations.end(); ++it)
        {
            //XMStoreFloat4x4(&boxes[i], XMMatrixTranslation((*it).x, (*it).y, (*it).z));

            XMMATRIX worldthing = XMLoadFloat4x4(&boxes[i]);
            XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(worldthing);
		    XMMATRIX worldViewProj = worldthing*view*proj;

            Effects::BasicFX->SetWorld(worldthing);
		    Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		    Effects::BasicFX->SetWorldViewProj(worldViewProj);
		    Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&mTexTransform));
		    Effects::BasicFX->SetMaterial(mBoxMat);
		    Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRV);

            //mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&(worldthing*viewProj)));
            //fxDiffuseMap->SetResource(mDiffuseMapSRV);

		    activeTech->GetPassByIndex(p)->Apply(0,md3dImmediateContext);
		    md3dImmediateContext->DrawIndexed(36,0,0);

            i++;
        }
		*/
    }

	HR(mSwapChain->Present(0, 0));
}

void TestD3DApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void TestD3DApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void TestD3DApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if( (btnState & MK_LBUTTON) != 0 )
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		mCam.Pitch(dy);
		mCam.RotateY(dx);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void TestD3DApp::BuildGeometryBuffers()
{
    Vertex v[24];

    //Front
    v[0] = Vertex(-0.5f,-0.5f,-0.5f,            0.0f, 0.0f, -1.0f, 0,1);
    v[1] = Vertex(0.5f,-0.5f,-0.5f,         0.0f, 0.0f, -1.0f, 1,1);
    v[2] = Vertex(-0.5f,0.5f,-0.5f,         0.0f, 0.0f, -1.0f, 0,0);
    v[3] = Vertex(0.5f,0.5f,-0.5f,      0.0f, 0.0f, -1.0f, 1,0);
    //Right
    v[4] = Vertex(0.5f,-0.5f,-0.5f,         1.0f,0.0f, 0.0f,  0,1);
    v[5] = Vertex(0.5f,-0.5f,0.5f,      1.0f, 0.0f, 0.0f,  1,1);
    v[6] = Vertex(0.5f,0.5f,-0.5f,      1.0f, 0.0f, 0.0f,  0,0);
    v[7] = Vertex(0.5f,0.5f,0.5f,   1.0f, 0.0f, 0.0f,  1,0);
    //Bottom
    v[8] = Vertex(-0.5f,-0.5f,0.5f,         0.0f, -1.0f, 0.0f,  0,1);
    v[9] = Vertex(0.5f,-0.5f,0.5f,      0.0f, -1.0f, 0.0f,  1,1);
    v[10] = Vertex(-0.5f,-0.5f,-0.5f,           0.0f, -1.0f, 0.0f, 0,0);
    v[11] = Vertex(0.5f,-0.5f,-0.5f,        0.0f, -1.0f, 0.0f, 1,0);
    //Back
    v[12] = Vertex(0.5f,-0.5f,0.5f,     0.0f, 0.0f, 1.0f,  0,1);
    v[13] = Vertex(-0.5f,-0.5f,0.5f,        0.0f, 0.0f, 1.0f,  1,1);
    v[14] = Vertex(0.5f,0.5f,0.5f,  0.0f, 0.0f, 1.0f,  0,0);
    v[15] = Vertex(-0.5f,0.5f,0.5f,     0.0f, 0.0f, 1.0f,  1,0);
    //Left
    v[16] = Vertex(-0.5f,-0.5f,0.5f,        -1.0f, 0.0f, 0.0f,  0,1);
    v[17] = Vertex(-0.5f,-0.5f,-0.5f,           -1.0f, 0.0f, 0.0f,  1,1);
    v[18] = Vertex(-0.5f,0.5f,0.5f,     -1.0f, 0.0f, 0.0f,  0,0);
    v[19] = Vertex(-0.5f,0.5f,-0.5f,        -1.0f, 0.0f, 0.0f,  1,0);
    //Top
    v[20] = Vertex(-0.5f,0.5f,-0.5f,        0.0f, 1.0f, 0.0f,  0,1);
    v[21] = Vertex(0.5f,0.5f,-0.5f,     0.0f, 1.0f, 0.0f,  1,1);
    v[22] = Vertex(-0.5f,0.5f,0.5f,     0.0f, 1.0f, 0.0f,  0,0);
    v[23] = Vertex(0.5f,0.5f,0.5f,  0.0f, 1.0f, 0.0f,  1,0);



    
#pragma region penis
	//Initializing the vertex buffer description struct.
	D3D11_BUFFER_DESC vertexBuffer;
	

	vertexBuffer.ByteWidth = sizeof(Vertex) * 24;

	vertexBuffer.Usage = D3D11_USAGE_IMMUTABLE;

	vertexBuffer.CPUAccessFlags = 0;

	vertexBuffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	vertexBuffer.MiscFlags = 0;

	vertexBuffer.StructureByteStride = 0;


	//Setting up the vertex buffer actual data struct
	D3D11_SUBRESOURCE_DATA vertexInitData;
	vertexInitData.pSysMem = v;

	//Create the actual buffer according to the description, using the subresource_data and outputting the buffer to BoxVertexBuffer
	md3dDevice->CreateBuffer(&vertexBuffer, &vertexInitData, &BoxVertexBuffer);

#pragma endregion
	//Almost same stuff again for index buffer

    UINT indices[] =
	{
		1, 2, 3,
		2, 1, 0,

		6,5,4,
		5,6,7,

		10,9,8,
		9,10,11,

		14,13,12,
		13,14,15,

		18,17,16,
		17,18,19,

		22,21,20,
		21,22,23
	};
	//Description
	D3D11_BUFFER_DESC indexBuffer;

	indexBuffer.ByteWidth = sizeof(Vertex) * 36;

	indexBuffer.Usage = D3D11_USAGE_IMMUTABLE;

	indexBuffer.BindFlags = D3D11_BIND_INDEX_BUFFER;

	indexBuffer.CPUAccessFlags = 0;

	indexBuffer.MiscFlags = 0;

	indexBuffer.StructureByteStride = 0;


	//Actual data
	D3D11_SUBRESOURCE_DATA indexInitData;
	indexInitData.pSysMem = indices;


	md3dDevice->CreateBuffer(&indexBuffer, &indexInitData, &BoxIndexBuffer);
		
}

void TestD3DApp::BuildInitialMap()
{
	GenericTerraNoise GTN(chunkSizeX,chunkSizeY, chunkSizeZ);
	GTN.fillBlockArray();

}

void TestD3DApp::BuildInstancedBuffer()
{
	//drawBMP();
	//HihiiKakka();
	//mInstancedData.resize(cubeSideCount*cubeSideCount*cubeSideCount);
	/*
	GenericTerraNoise GTN(chunkSizeX,chunkSizeY, chunkSizeZ);
	//GTN.initializeBasicTerrain();
	GTN.fillBlockArray();



	for(int x = 0; x < chunkSizeX; x++)
    {
        for(int y = 0; y < chunkSizeY; y++)
        {
            for(int z = 0; z < chunkSizeZ; z++)
			{
				if(GTN.blockTypeAtLocation(x,y,z)==AIR)
				{
					continue;
				}
				auto matrix = XMMatrixTranslation(x,y,z);

				InstancedData
					BYTE
				float noiseValue = GTN.getBasicTerrain(x,y,z);
				//noiseValue = (int)noiseValue;
				//double noiseValue = getGradient(x,y,z);
                if(noiseValue > 0.8f && y < chunkSizeY / 2) 
                {
					auto matrix = XMMatrixTranslation(x,y,z);

					InstancedData id;
					XMStoreFloat4x4(&id.World, matrix);

					id.Color.x = MathHelper::RandF(0.0f, 1.0f);
					id.Color.y = MathHelper::RandF(0.0f, 1.0f);
					id.Color.z = MathHelper::RandF(0.0f, 1.0f);
					id.Color.w = 1.0f;

					mInstancedData.push_back(id);
				}
				
            }
        }
    }
	*/
	
	D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(InstancedData) * mInstancedData.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	
    HR(md3dDevice->CreateBuffer(&vbd, 0, &mInstancedBuffer));
}

void TestD3DApp::BuildFX()
{
	ID3D10Blob * compiledShader = 0;
	ID3D10Blob * compMsgs = 0;

	HRESULT hr = D3DX11CompileFromFile(L"FX/color.fx",0,0,0,"fx_5_0", 0,
		0, 0, &compiledShader, &compMsgs, 0);

	if( compMsgs != 0 )
	{
		MessageBoxA(0, (char*)compMsgs->GetBufferPointer(), 0, 0);
		ReleaseCOM(compMsgs);
	}

		if(FAILED(hr))
	{
		DXTrace(__FILE__, (DWORD)__LINE__, hr, L"D3DX11CompileFromFile", true);
	}

	D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(),
		0,md3dDevice, &mFX);

	ReleaseCOM(compiledShader);

	mTech = mFX->GetTechniqueByName("ColorTech");
	mfxWorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	mfxTotalTime = mFX->GetVariableByName("gTime")->AsScalar();
    fxDiffuseMap = mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
}

void TestD3DApp::BuildVertexLayout()
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[8] =
	{
		{"POSITION", 0,  DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL",   0,  DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0,  DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"WORLD",	 0,  DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"WORLD",	 1,  DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"WORLD",	 2,  DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"WORLD",	 3,  DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"COLOR",	 0,  DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1}
	};

	D3DX11_PASS_DESC passDesc;
	Effects::BasicFX->Light1Tech->GetPassByIndex(0)->GetDesc(&passDesc);

	md3dDevice->CreateInputLayout(vertexDesc, 8, passDesc.pIAInputSignature, 
		passDesc.IAInputSignatureSize, &mInputLayout);

    //device->CreateInputLayout(InputLayoutDesc::Basic32, 3, passDesc.pIAInputSignature, 
	//	passDesc.IAInputSignatureSize, &Basic32)
}

void TestD3DApp::BuildCubeTranslationsAndRotations()
{
    //So now we have our shitload of locations, save those in matrices to the memory or some shit :-D
    int i = 0;
    for(std::vector<Int3>::iterator it = locations.begin(); it != locations.end(); ++it)
    {
        //Cube translations
        XMFLOAT4X4 placeholder;

		XMMATRIX penisOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
		XMStoreFloat4x4(&placeholder, XMMatrixMultiply(XMMatrixTranslation((*it).x, (*it).y, (*it).z), penisOffset));
        //XMStoreFloat4x4(&placeholder, XMMatrixTranslation((*it).x - 0.5f, (*it).y - 0.5f, (*it).z - 0.5f));

        boxes.push_back(placeholder);

        //Cube rotations
        int randomSpeed = rand() % 100 + 10;
        randomSpeeds.push_back(randomSpeed);


        rotations.push_back(Float3(0,0,0));
        
        i++;
    }

}

void TestD3DApp::BuildBlockPositions()
{
	/*
    noise::module::Perlin myModule;
    for(int x = 0; x < 5; x++)
    {
        for(int y = 0; y < 5; y++)
        {
            for(int z = 0; z < 5; z++)
            {
                double xd = x;
                double yd = y;
                double zd = z;

                double noiseValue = myModule.GetValue(xd/10,yd/10,zd/10);
                if(noiseValue >= 0.5)
                {
                    locations.push_back(Int3(x,y,z));
                }
            }
        }
    }
	*/
	for(int x = 0; x < 5; x++)
    {
        for(int y = 0; y < 1; y++)
        {
            for(int z = 0; z < 1; z++)
            {
				locations.push_back(Int3(x,y,z));
            }
        }
    }
}



