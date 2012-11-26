#pragma once
#include "d3dapp.h"

class wam :
	public D3DApp
{
public:
	wam(HINSTANCE hInstance);
	~wam(void);
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
	int MolesInMotion();
	void Pick(int sx, int sy);

private:

	WAMGame gamePieces; //the game pieces
	//additional class to handle game play

	ID3D11Buffer* mVB;
	ID3D11Buffer* mIB;

	ID3DX11Effect* mFX;
	ID3DX11EffectTechnique* mTech;
	ID3DX11EffectMatrixVariable* mfxWorldViewProj;
	ID3D11InputLayout* mInputLayout;
	ID3D11RasterizerState* mWireframeRS;

	XMFLOAT4X4 mSphereWorld[10];
	XMFLOAT4X4 mCylWorld[10];
	XMFLOAT4X4 mGridWorld;
	XMFLOAT4X4 mCenterSphere;

	//Save these for the pick matrix?
	GeometryGenerator::MeshData grid;
	GeometryGenerator::MeshData sphere;
	GeometryGenerator::MeshData cylinder;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	int mBoxVertexOffset;
	int mGridVertexOffset;
	int mSphereVertexOffset;
	int mCylinderVertexOffset;

	UINT mGridIndexOffset;
	UINT mSphereIndexOffset;
	UINT mCylinderIndexOffset;

	UINT mGridIndexCount;
	UINT mSphereIndexCount;
	UINT mCylinderIndexCount;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;

	//GAME parameters
	float mStartY;
	float mEndY;
	float mTimeStepMovement;
	float mTimeStepInitiateMovement;
	float mSpeed;
	float mMoleSpeeds[10];
	XMFLOAT3 mMoleLocations[10];
	
	Camera mCam;

};
