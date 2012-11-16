#pragma once

 class Obstacle{

	GLuint name;
	GLuint listID;
	Point origin;
	Point spaceDimensions; //if objects within the spaceBuffer then collision occurs
	Point velocity;
	bool visible;	
	vector<Point> boundingPolygon;
	GLuint textureID;

	float minX;
	float minY;
	float minZ;
	float maxX;
	float maxY;
	float maxZ;
	//used for display polygons with textures
 protected: 
	vector<Point> calculateTextureCoords(vector<Point> shape);
	void getRange(vector<Point> shape, GLfloat &lowestX, GLfloat &highestX,
							GLfloat &lowestY, GLfloat &highestY,
							GLfloat &lowestZ, GLfloat &highestZ);
	
public:
	Obstacle(void);
	Obstacle(	GLuint newName, 
				Point Origin,
				Point newSpaceDimensions,
				Point newVelocity,
				bool newVisibility=true,
				GLuint newListID=0);
				
	~Obstacle(void);

	Point getSpaceDimensions();
	Point getOrigin();
	Point getVelocity();
	GLuint getName();		
	bool getVisibility();
	GLfloat getBoundingRadius();
	GLuint getListId();
	float getDistanceFromObject(Obstacle testObj);
	float getDistanceFromObject(Point testpt);
	bool objectWithinBoundingBox(Obstacle testObj);
	bool objectWithinRadius(Obstacle testObj);

	void setSpaceDimensions(Point newSpaceDimensions);
	void setOrigin(Point newOrigin);
	void setVelocity(Point newVelocity);
	void setName(GLuint newName);
	void setVisibility(bool newVisibility);
	void setListId(GLuint newListID);
	void setBoundingBox(float newMinX,float newMaxX,
						float newMinY,float newMaxY,
						float newMinZ,float newMaxZ);
	void setMinY(float newMinY);
	void setMinX(float newMinX);
	void setMinZ(float newMinZ);
	void setMaxY(float newMaxY);
	void setMaxX(float newMaxX);
	void setMaxZ(float newMaxZ);

	float getMinY();
	float getMinX();
	float getMinZ();
	float getMaxY();
	float getMaxX();
	float getMaxZ();

	void display();
	
};
