#pragma once

class WAMGame
{
public:
	WAMGame(void);
	~WAMGame(void);

//contain all of the game pieces
	//1 rectangle as board back ground
	//1 hole redrawn over 9 vertices
	//1 mole redrawn over 9 locations
	void Init	(ID3D11Device* d3dDevice,
						UINT newRows, 
						UINT newCols, 
						float newMargin, 
						float newHoleRadius,
						float newPopUpStartZ,
						float newPopUpEndZ,
						float newPopUpSpeed,
						float newMoleRadius,
						float newTimeStep,
						UINT newHoleDegreeDetail);

	void UpdateBoard();
	void DrawBoard();

private:
		//board parameters
		UINT gameX;
		UINT gameY;
		float margin;
		UINT cols;
		UINT rows;
		float holeRadius;
		UINT holeDetailDegree;

		//Mole Parameters
		float popUpEndZ;
		float popUpStartZ;
		float popUpSpeed;
		float moleRadius;

		float mTimeStep;

		ID3D11Buffer *mGameBoardVB;
		ID3D11Buffer *mGameBoardIB;

		ID3D11Buffer *mHoleVB;
		ID3D11Buffer *mHoleIB;

		ID3D11Buffer *mMoleVB;
		ID3D11Buffer *mMoleIB;

		void ReleaseBuffers();
		void BuildGeometryBuffers(ID3D11Device* md3dDevice);
		void InitBoard(ID3D11Device* md3dDevice);
		void InitHoles(ID3D11Device* md3dDevice);
		void InitMoles(ID3D11Device* md3dDevice);
};
