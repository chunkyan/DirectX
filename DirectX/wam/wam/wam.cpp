#include "stdafx.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	wam theApp(hInstance);
	
	if( !theApp.Init() )
		return 0;
	
	return theApp.Run();
}

wam::wam(HINSTANCE hInstance)
:D3DApp(hInstance),
	mFX(0), 
	mTech(0),
	mfxWorldViewProj(0), 
	mInputLayout(0), 
	mWireframeRS(0),
	//mTheta(1.5f*MathHelper::Pi), 
	//mPhi(0.1f*MathHelper::Pi), 
	//mRadius(15.0f),
	mTheta(7.8575),
	mPhi(0.1),
	mRadius(17),
	mStartY(0), //resting mole height
	mEndY(5.0f),//popup mole height
	mTimeStepMovement(0.03f),	//time between mole animations
	mTimeStepInitiateMovement(2.0f), //time to randomly choose a mole to  pop up
	mSpeed(0.5f)  //speed of the moles
	
{
	mMainWndCaption = L"Bump-Your-Bank";
	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mGridWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	for(int i = 0; i < 3; ++i)
	{
		XMStoreFloat4x4(&mCylWorld[i*3+0], XMMatrixTranslation(-5.0f,-3.0f, -5.0f + i*5.0f));
		XMStoreFloat4x4(&mCylWorld[i*3+1], XMMatrixTranslation(0.0f, -3.0f, -5.0f + i*5.0f));
		XMStoreFloat4x4(&mCylWorld[i*3+2], XMMatrixTranslation(5.0f, -3.0f, -5.0f + i*5.0f));

		XMStoreFloat4x4(&mSphereWorld[i*3+0], XMMatrixTranslation(-5.0f, -1.0f, -5.0f + i*5.0f));
		XMStoreFloat4x4(&mSphereWorld[i*3+1], XMMatrixTranslation(0.0f, -1.0f, -5.0f + i*5.0f));
		XMStoreFloat4x4(&mSphereWorld[i*3+2], XMMatrixTranslation(5.0f, -1.0f, -5.0f + i*5.0f));

	}

	ZeroMemory(&mMoleSpeeds,sizeof(float)*10);
	ZeroMemory(&mMoleLocations,sizeof(XMFLOAT3)*10);

	//Set the camera position
	mCam.SetPosition(0,-1,-5);
}

wam::~wam(void)
{
	ReleaseCOM(mFX);
	ReleaseCOM(mInputLayout);
	ReleaseCOM(mWireframeRS);
}

bool wam::Init()
{
	if(!D3DApp::Init())
		return false;

	BuildGeometryBuffers();
	BuildFX();
	BuildVertexLayout();

	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;

	HR(md3dDevice->CreateRasterizerState(&wireframeDesc, &mWireframeRS));

	return true;

}

void wam::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	mCam.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void wam::UpdateScene(float dt)
{

	if( GetAsyncKeyState('P') & 0x8000 )
		mPhi += 0.01f;
	if( GetAsyncKeyState('p') & 0x8000 )
		mPhi -= 0.01f;
	if( GetAsyncKeyState('O') & 0x8000 )
		mTheta += 0.01f;
	if( GetAsyncKeyState('o') & 0x8000 )
		mTheta -= 0.01f;

	// Convert Spherical to Cartesian coordinates.
	float x = mRadius*sinf(mPhi)*cosf(mTheta);
	float z = mRadius*sinf(mPhi)*sinf(mTheta);
	float y = mRadius*cosf(mPhi) ;

	// Build the view matrix.
	XMVECTOR pos    = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
//	XMStoreFloat4x4(&mView, V);
	mCam.LookAt(pos,target,up);

	if( GetAsyncKeyState('W') & 0x8000 )
		mCam.Walk(10.0f*dt);

	if( GetAsyncKeyState('S') & 0x8000 )
		mCam.Walk(-10.0f*dt);

	if( GetAsyncKeyState('A') & 0x8000 )
		mCam.Strafe(-10.0f*dt);

	if( GetAsyncKeyState('D') & 0x8000 )
		mCam.Strafe(10.0f*dt);



	static float tM=0;
	static float tR=0;
	tM+=dt;
	tR+=dt;
	//update the moles in action
	if(tM > mTimeStepMovement)
	{	
		for(int i=0;i<9;i++)
		{
			float yTarget =mMoleSpeeds[i]>0?mEndY:mStartY;
			if(mMoleSpeeds[i]!=0)
			{
				if((mMoleLocations[i].y + mMoleSpeeds[i])>=mEndY)
					mMoleSpeeds[i]= -mSpeed;
				if((mMoleLocations[i].y + mMoleSpeeds[i])<=mStartY)
					mMoleSpeeds[i] = 0;
				mMoleLocations[i].y += mMoleSpeeds[i]; 
			}
		}
		tM=0.0f;
	}

	if(tR>mTimeStepInitiateMovement)
	{
		bool acceptablePick=false;
		int indexPick=0;
		int guessPick=0;
		while(!acceptablePick && MolesInMotion()<3)
		{
			indexPick = (int)MathHelper::RandF(0,9);
			if(mMoleSpeeds[indexPick]==0)
			{
				acceptablePick=true;
				mMoleSpeeds[indexPick]=mSpeed;
			}
			guessPick++; //ensure that we don't hit an infinite loop if a lot of moles are in action
		}
		tR=MathHelper::RandF(0.7f,3.0f);
	}

	//if appropriate time passed
	//put a new mole into action

}

void wam::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(mInputLayout);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	md3dImmediateContext->RSSetState(mWireframeRS);

	UINT stride = sizeof(Vertex);
    UINT offset = 0;
    md3dImmediateContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);

	// Set constants
	mCam.UpdateViewMatrix();
	//XMMATRIX view  = XMLoadFloat4x4(&mView);
	//XMMATRIX proj  = XMLoadFloat4x4(&mProj);
	//XMMATRIX viewProj = view*proj;

	XMMATRIX view  = mCam.View();
	XMMATRIX proj  = mCam.Proj();
	XMMATRIX viewProj = mCam.ViewProj();

    D3DX11_TECHNIQUE_DESC techDesc;
    mTech->GetDesc( &techDesc );
    for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		// Draw the grid.
		XMMATRIX world = XMLoadFloat4x4(&mGridWorld);
		mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&(world*viewProj)));
		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

		// Draw the cylinders.
		for(int i = 0; i <9; i++)
		{
			world = XMLoadFloat4x4(&mCylWorld[i]);
			XMMATRIX translation = XMMatrixTranslation(mMoleLocations[i].x , mMoleLocations[i].y, mMoleLocations[i].z);
			world = world * translation;
			mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&(world*viewProj)));
			mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
			md3dImmediateContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
		}

		// Draw the spheres.
		for(int i = 0; i < 9; ++i)
		{
			world = XMLoadFloat4x4(&mSphereWorld[i]);
			XMMATRIX translation = XMMatrixTranslation(mMoleLocations[i].x , mMoleLocations[i].y, mMoleLocations[i].z);
			world = world * translation;
			mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&(world*viewProj)));
			mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
			md3dImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
		}
    }

	HR(mSwapChain->Present(0, 0));
}

void wam::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;
	Pick(x,y);
	SetCapture(mhMainWnd);
}

void wam::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void wam::OnMouseMove(WPARAM btnState, int x, int y)
{
	if( (btnState & MK_LBUTTON) != 0 )
	{
/*		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi   += dy;

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi-0.1f);
		*/
	}
	else if( (btnState & MK_RBUTTON) != 0 )
	{
/*		// Make each pixel correspond to 0.01 unit in the scene.
		float dx = 0.01f*static_cast<float>(x - mLastMousePos.x);
		float dy = 0.01f*static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 2.0f, 200.0f);
		*/
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void wam::BuildGeometryBuffers()
{


	GeometryGenerator geoGen;

	geoGen.CreateGrid(20.0f, 30.0f, 60, 40, grid);
	geoGen.CreateSphere(0.5f, 20, 20, sphere);
	//geoGen.CreateGeosphere(0.5f, 2, sphere);
	geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20, cylinder);

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	mGridVertexOffset     = 0;
	mSphereVertexOffset   = mGridVertexOffset + grid.Vertices.size();
	mCylinderVertexOffset = mSphereVertexOffset + sphere.Vertices.size();

	// Cache the index count of each object.
	mGridIndexCount     = grid.Indices.size();
	mSphereIndexCount   = sphere.Indices.size();
	mCylinderIndexCount = cylinder.Indices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	mGridIndexOffset     = 0;
	mSphereIndexOffset   = mGridIndexOffset + mGridIndexCount;
	mCylinderIndexOffset = mSphereIndexOffset + mSphereIndexCount;
	
	UINT totalVertexCount = 
		grid.Vertices.size() + 
		sphere.Vertices.size() +
		cylinder.Vertices.size();

	UINT totalIndexCount = 
		mGridIndexCount + 
		mSphereIndexCount +
		mCylinderIndexCount;

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	std::vector<Vertex> vertices(totalVertexCount);

	XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);

	UINT k = 0;

	for(size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos   = grid.Vertices[i].Position;
		vertices[k].Color = black;
	}

	for(size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos   = sphere.Vertices[i].Position;
		vertices[k].Color = black;
	}

	for(size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos   = cylinder.Vertices[i].Position;
		vertices[k].Color = black;
	}

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex) * totalVertexCount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	std::vector<UINT> indices;
	indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());
	indices.insert(indices.end(), sphere.Indices.begin(), sphere.Indices.end());
	indices.insert(indices.end(), cylinder.Indices.begin(), cylinder.Indices.end());

	D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &indices[0];
    HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mIB));

}

void wam::BuildFX()
{
	std::ifstream fin("fx/color.fxo", std::ios::binary);

	fin.seekg(0, std::ios_base::end);
	int size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	std::vector<char> compiledShader(size);

	fin.read(&compiledShader[0], size);
	fin.close();
	
	HR(D3DX11CreateEffectFromMemory(&compiledShader[0], size, 
		0, md3dDevice, &mFX));

	mTech    = mFX->GetTechniqueByName("ColorTech");
	mfxWorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();

}

void wam::BuildVertexLayout()
{
	// Create the vertex input layout.
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	// Create the input layout
    D3DX11_PASS_DESC passDesc;
    mTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(md3dDevice->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature, 
		passDesc.IAInputSignatureSize, &mInputLayout));

}


int wam::MolesInMotion()
{
	int molesInActionCount=0;
	for(int i=0;i<9;i++)
		molesInActionCount +=mMoleSpeeds[i]!=0?1:0;
	return molesInActionCount;
}


void wam::Pick(int sx, int sy)
{
	
XMMATRIX P = mCam.Proj();

	// Compute picking ray in view space.
	float vx = (+2.0f*sx/mClientWidth  - 1.0f)/P(0,0);
	float vy = (-2.0f*sy/mClientHeight + 1.0f)/P(1,1);

	// Ray definition in view space.
	XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR rayDir    = XMVectorSet(vx, vy, 1.0f, 0.0f);

	// Tranform ray to local space of Mesh.
	XMMATRIX V = mCam.View();
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(V), V);

	XMMATRIX W = XMLoadFloat4x4(&mGridWorld);
	XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);

	XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

	rayOrigin = XMVector3TransformCoord(rayOrigin, toLocal);
	rayDir = XMVector3TransformNormal(rayDir, toLocal);

	// Make the ray direction unit length for the intersection tests.
	rayDir = XMVector3Normalize(rayDir);
	
		XNA::Sphere tmpSphere11,tmpSphere12,tmpSphere13,tmpSphere21,tmpSphere22,tmpSphere23,tmpSphere31,tmpSphere32,tmpSphere33;
		float radius =0.5f;
		tmpSphere11.Radius = radius;
		tmpSphere12.Radius = radius;
		tmpSphere13.Radius = radius;
		tmpSphere21.Radius = radius;
		tmpSphere22.Radius = radius;
		tmpSphere23.Radius = radius;
		tmpSphere31.Radius = radius;
		tmpSphere32.Radius = radius;
		tmpSphere33.Radius = radius;

		tmpSphere11.Center = XMFLOAT3(-5.0f,-1.0f,-5.0f);
		tmpSphere12.Center = XMFLOAT3( 0.0f,-1.0f,-5.0f);
		tmpSphere13.Center = XMFLOAT3( 5.0f,-1.0f,-5.0f);
		tmpSphere21.Center = XMFLOAT3(-5.0f,-1.0f, 0.0f);
		tmpSphere22.Center = XMFLOAT3( 0.0f,-1.0f, 0.0f);
		tmpSphere23.Center = XMFLOAT3( 5.0f,-1.0f, 0.0f);
		tmpSphere31.Center = XMFLOAT3(-5.0f,-1.0f, 5.0f);
		tmpSphere32.Center = XMFLOAT3( 0.0f,-1.0f, 5.0f);
		tmpSphere33.Center = XMFLOAT3( 5.0f,-1.0f, 5.0f);

		UINT PickedSphere=0;
		FLOAT tmpDist;
		//IntersectRaySphere( FXMVECTOR Origin, FXMVECTOR Direction, const Sphere* pVolume, FLOAT* pDist );

		if(XNA::IntersectRaySphere( rayOrigin, rayDir, &tmpSphere11, &tmpDist ))
			PickedSphere= 11;
		if(XNA::IntersectRaySphere( rayOrigin, rayDir, &tmpSphere12, &tmpDist ))
			PickedSphere= 12;
		if(XNA::IntersectRaySphere( rayOrigin, rayDir, &tmpSphere13, &tmpDist ))
			PickedSphere= 13;

		if(XNA::IntersectRaySphere( rayOrigin, rayDir, &tmpSphere21, &tmpDist ))
			PickedSphere= 21;
		if(XNA::IntersectRaySphere( rayOrigin, rayDir, &tmpSphere22, &tmpDist ))
			PickedSphere= 22;
		if(XNA::IntersectRaySphere( rayOrigin, rayDir, &tmpSphere23, &tmpDist ))
			PickedSphere= 23;

		if(XNA::IntersectRaySphere( rayOrigin, rayDir, &tmpSphere31, &tmpDist ))
			PickedSphere= 31;
		if(XNA::IntersectRaySphere( rayOrigin, rayDir, &tmpSphere32, &tmpDist ))
			PickedSphere= 32;
		if(XNA::IntersectRaySphere( rayOrigin, rayDir, &tmpSphere33, &tmpDist ))
			PickedSphere= 33;

		if(PickedSphere!=0)
		{
			int ps =PickedSphere;
		}
	}