#include "stdafx.h"

WAMGame::WAMGame(void):
		mGameBoardVB(0),
		mGameBoardIB(0),
		mHoleVB(0),
		mHoleIB(0),
		mMoleVB(0),
		mMoleIB(0)
{
}

WAMGame::~WAMGame(void)
{
	ReleaseBuffers();
}

void WAMGame::ReleaseBuffers()
{

	if(mGameBoardVB)
		mGameBoardVB->Release();
	if(mGameBoardIB)
		mGameBoardIB->Release();
	if(mHoleVB)
		mHoleVB->Release();
	if(mHoleIB)
		mHoleIB->Release();
	if(mMoleVB)
		mMoleVB->Release();
	if(mMoleIB)
		mMoleIB->Release();
}


void WAMGame::Init(ID3D11Device* d3dDevice,
						UINT newRows, 
						UINT newCols, 
						float newMargin, 
						float newHoleRadius,
						float newPopUpStartZ,
						float newPopUpEndZ,
						float newPopUpSpeed,
						float newMoleRadius,
						float newTimeStep,
						UINT newholeDetailDegree)
{
	ReleaseBuffers();
	rows = newRows;
	cols = newCols;
	margin = newMargin;
	holeRadius = newHoleRadius;
	popUpStartZ = newPopUpStartZ;
	popUpEndZ = newPopUpEndZ;
	popUpSpeed  = newPopUpSpeed;
	moleRadius = newMoleRadius;
	mTimeStep = newTimeStep;
	holeDetailDegree =newholeDetailDegree;
	//buildVertexBuffers
	BuildGeometryBuffers(d3dDevice);


}



void WAMGame::UpdateBoard()
{

}

void WAMGame::DrawBoard()
{
}


void WAMGame::BuildGeometryBuffers(ID3D11Device* md3dDevice)
{
	InitHoles(md3dDevice);
	InitMoles(md3dDevice);
	InitBoard(md3dDevice);


}

void WAMGame::InitHoles(ID3D11Device* md3dDevice)
{

	std::vector<Vertex> holeVertices(holeDetailDegree+1);
	Vertex centre;
	centre.Pos =  XMFLOAT3(0.0f,0.0f,popUpStartZ);
	centre.Color =  XMFLOAT4(0.0f,0.0f,0.0f,1.0f);
	holeVertices.push_back(centre);
	holeDetailDegree = holeDetailDegree<8?8:holeDetailDegree;
	float step = (XM_2PI)/holeDetailDegree;
	float startAngle=0;
	UINT indice =1;
	while (indice<(holeDetailDegree+1))
	{
		holeVertices[indice].Pos.x = centre.Pos.x + holeRadius* cosf(startAngle) + holeRadius*sinf(startAngle);
		holeVertices[indice].Pos.y = centre.Pos.y + holeRadius* sinf(startAngle) + holeRadius*cosf(startAngle);
		startAngle +=step;
		indice++;
	}

	D3D11_BUFFER_DESC vbdHole;
	vbdHole.Usage  = D3D11_USAGE_IMMUTABLE;
	vbdHole.CPUAccessFlags=0;
	vbdHole.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbdHole.MiscFlags=0;
	vbdHole.ByteWidth = sizeof(Vertex) * holeDetailDegree +1;
	vbdHole.StructureByteStride=0;
	
	D3D11_SUBRESOURCE_DATA vbdInit;
	vbdInit.pSysMem = &holeVertices[0];
	HRESULT hr =md3dDevice->CreateBuffer(&vbdHole,&vbdInit,&mHoleVB);

	std::vector<UINT> holeIndices(3 * holeDetailDegree +3);
	UINT startIndices = 0;
	while(startIndices < (holeDetailDegree) )
	{
		holeIndices.push_back(0);
		holeIndices.push_back((startIndices+1)% ( holeDetailDegree +1));
		holeIndices.push_back((startIndices+2)% ( holeDetailDegree +1));		
		startIndices++;
	}

	D3D11_BUFFER_DESC idbHole;
	idbHole.Usage = D3D11_USAGE_IMMUTABLE;
	idbHole.CPUAccessFlags =0;
	idbHole.BindFlags= D3D11_BIND_INDEX_BUFFER;
	idbHole.MiscFlags =0;
	idbHole.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA idbInit;
	idbInit.pSysMem = &holeIndices[0];
	hr = md3dDevice->CreateBuffer(&idbHole,&idbInit,&mHoleIB);
	
}

void WAMGame::InitBoard(ID3D11Device* md3dDevice)
{

}

void WAMGame::InitMoles(ID3D11Device* md3dDevice)
{

}