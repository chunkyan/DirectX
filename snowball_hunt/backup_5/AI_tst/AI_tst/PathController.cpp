#include "stdafx.h"

int round(double number)
{
return static_cast<int> (floor(number+0.5));
}


PathController::PathController(void):currMap(0),character(0)
{

}

PathController::PathController(	Map *newMap,
					Enemy *newCharacter,
					Vehicle *newOpponent
					):currTarget(0),maxPath(0)
{
	character = newCharacter;
	currMap = newMap;
	opponent = newOpponent;
}

PathController::~PathController(void)
{

}
	
void PathController::setNextTarget()
{
	vector<MapNode*> targets = currMap->getTargets();
	int numTargets = targets.size();
	currTarget = (currTarget+1) % numTargets;
}


Point PathController::adjustVelocity(Point pt1, Point pt2,float speed)
{
	//calculate velocity 
	//facing angle vs angle to nextPath
	Point tmp = makeUnitVector(Point(pt2.x - pt1.x,0,pt2.z  - pt1.z));
	tmp.x = tmp.x*speed; //adjust for speed of object
	tmp.y = tmp.y*speed;
	tmp.z = tmp.z*speed;
	return tmp;
}


Point PathController::makeUnitVector(Point newPt)
{
	GLfloat magnitude = sqrt(newPt.x*newPt.x + newPt.y*newPt.y +newPt.z*newPt.z);
	return Point (newPt.x/magnitude,newPt.y/magnitude,newPt.z/magnitude);
}

void PathController::deletePath()
{
	vector<PathNode*>::iterator pIter;
	pIter= path.begin();
	while(pIter != path.end())
		delete *pIter;
	path.clear();
}

void PathController::setPath()
{
	deleteVisited();
	deletePath();
	if(currTargetNode->getNodeIndex() != currMap->getNode(character->getOrigin())->getNodeIndex())
	{
		setPathNodes(currTargetNode);
	}else
	{
		setNextTarget(); //set the target
		setPathNodes(currMap->getTargets()[currTarget]);
	}
}

void PathController::advancePath()
{
	try
	{
		if(!path.empty())
			path.erase(path.begin()); 		

		if(path.empty())
			setPath();

		//reset the facing angle &velocity of object
		if (!path.empty())
			character->setVelocity(adjustVelocity(character->getOrigin(),path[0]->getLocation(),character->getSpeed() ));
 	} catch(std::exception err)
	{
		cout <<"Error PathController ::advancePath "<<err.what()<<endl;
	}

}



void PathController::advanceCharacter()
{

	try
	{
		npcAction oldState= character->getCurrentState();
		//Is Opponent in viewable range  && has snowball && opponent not within firing range then reset path to opponent
		//	if opponent has snowball and within firing range then clear path  then throw snowball
		//is Opponent within viewable range && !have snowball then runfor life --reset path to closest hiding place

		setCharacterStates();
		character->UpdateCurrentState();

		switch(character->getCurrentState())
		{
			case IDLE:
			case CARRY_SNOWBALL:
				if(path.empty()) //if current path has not been set
					setPath();
				else
					moveCharacter();
				break;
			case RUN_FOR_LIFE:
				//if previous state !=RUN_FOR_LIFE... set  new path
				//oldState
				if(oldState!=RUN_FOR_LIFE)
					findHidingPlace();
				moveCharacter();
				break;
			//default do nothing as these states require the character to do nothing
		}//end of switch
	}catch(std::exception err)
	{
		cout <<" Exception PathController::moveCharacter "<<err.what()<<endl;
	}

}//end of advanceCharacter


void PathController::setPathNodes(MapNode* target)
{
	//get node that current object resides in 
	//check all nodes around object
	//find node closest distanct to object that has a security rating
	//add to path target 
	//increment path count
	//repeat until nodeindex==targetindex point
	currTargetNode = target;

	MapNode *nextNode=0;
	MapNode *currNode =currMap->getNode(character->getOrigin());
	MapNode *enemyNode =currMap->getNode(opponent->getOrigin());
	Point nodeIndex = currNode->getNodeIndex();
	set<MapNode*> currTested;
	int Gcost =0;
	int Hcost =0;
	int securityRating=0;
	int currCost=-1;
	int currSecurityRating=0;
	int currHCost=0;
	int currGCost=0;
	try
	{

	while(nodeIndex!=target->getNodeIndex())
	{
		if((int)path.size()==maxPath) //exit out so we don't spend all day looking for a path
			return;
		int minX=nodeIndex.x-1<0?0:nodeIndex.x-1;
		int maxX=nodeIndex.x+1>currMap->getX()?currMap->getX():nodeIndex.x+1;
		for(int i =minX;i<=maxX;i++)
		{
			int minZ=nodeIndex.z-1<0?0:nodeIndex.z-1;
			int maxZ=nodeIndex.z+1>currMap->getZ()?currMap->getZ():nodeIndex.z+1;
			for(int j = minZ;j<=maxZ;j++)
			{	
				MapNode *adjNode = currMap->getNode(i,0,j);
				int distanceFromEnemy =  abs( enemyNode->getNodeIndex().x-i) + abs(enemyNode->getNodeIndex().z-j);
				if (nodeIsAvailable(adjNode) &&
					distanceFromEnemy>3) //ensure that we don't run over the enemy
				{
					if(adjNode->getNodeIndex()!=nodeIndex) //if not the square we occupy
					{							
						//get the distance to final destination;
						Hcost = target==adjNode ?0:(    abs( (target->getNodeIndex().x-i)) + abs((target->getNodeIndex().z-j)) )*10;
						if(i==nodeIndex.x || j==nodeIndex.z)
							Gcost = 10;
						else
							Gcost = 14;

						securityRating= 0;//CalcSecurityRating();
						
						if(currCost==-1 ||  (Gcost+Hcost+securityRating)<currCost )
						{
							nextNode = adjNode;
							currHCost = Hcost;
							currGCost = Gcost;
							currSecurityRating;
							currCost = Gcost+Hcost+securityRating;
						}
						currTested.insert(adjNode);
					}
				}//end of if nodeIsAvailable
			}//end of for Z
		}//end of for X
		if(nextNode!=0)
		{
			currCost =-1;
			PathNode *newPath = new PathNode(currHCost,currGCost, currSecurityRating,*nextNode);
			path.push_back(newPath);
			nodeIndex = nextNode->getNodeIndex();//re-assess the next node
			nextNode=0;
		}else
		{
			
			currTested.clear();
			setNextTarget(); //set the target
			return; //prevent infinite loop if no nodes available
		}
	}//end of while
	} catch(std::exception err)
	{
		cout <<"Error PathController ::setPathNodes  "<<err.what()<<endl;
	}
	currTested.clear();
}//end of function


///
//verify that this node has not been visited
//that there are no objects on this node
bool PathController::nodeIsAvailable(MapNode *node)
{
	if (node==0)
		return false;

	vector<PathNode*>::iterator pIter = path.begin();
	while(pIter!=path.end())
	{
		if((*pIter)->getNodeIndex() == node->getNodeIndex())
			return false;
		pIter++;
	}
	switch( node->getState())
	{
	case EMPTY:
	case GIFT:
		return true;
	}

	return false;
}

bool PathController::nodeAlreadyVisited(MapNode *node)
{
	return false;
}

void PathController::deleteVisited()
{
	visited.clear();
}



float PathController::distanceFromNode(Point node1,Point node2)
{
	return abs(node1.x-node2.x) + abs(node1.z-node2.z);
}


//return the obstacle found				
MapNode* PathController::ObstacleExistsBetween(Point node1,Point node2)
{
	int stepsX = node2.x-node1.x;
	int stepsZ = node2.z-node1.z;

	int sX=0;
	int sZ=0;
	int incrX = node1.x<node2.x?1:-1;
	int incrZ = node1.z<node2.z?1:-1;
	float ratio=0;
	if( stepsX!=0 )
		ratio =  abs(stepsZ)>abs(stepsX)?abs(stepsX/stepsZ) :abs(stepsZ/stepsX);
	float stepTracker =0;

	int steps1Max =  abs(stepsZ)>abs(stepsX)?stepsZ:stepsX;
	int step1=0;

	while(abs(step1)<abs(steps1Max) )
	{
		step1 += abs(stepsZ)>abs(stepsX)?incrZ:incrX; //advance to the next row of values to check
		stepTracker += abs(stepsZ)>abs(stepsX)?incrX*ratio:incrZ*ratio;

		MapNode *tmp;
		MapNode *tmp2;
		MapNode *tmp3; 
		MapNode *tmp4; 
		MapNode *tmp5; 
		if(abs(stepsZ)>abs(stepsX))
		{
			tmp		= currMap->getNode(round(stepTracker)+node1.x  ,0, step1+node1.z);
			tmp2	= currMap->getNode(round(stepTracker)+node1.x-1,0, step1+node1.z);
			tmp3	= currMap->getNode(round(stepTracker)+node1.x+1,0, step1+node1.z);
			tmp4	= currMap->getNode(round(stepTracker)+node1.x+2,0, step1+node1.z);
			tmp5	= currMap->getNode(round(stepTracker)+node1.x-2,0, step1+node1.z);
		}
		else
		{
			tmp		= currMap->getNode(step1+node1.x,0,round(stepTracker)+node1.z);
			tmp2	= currMap->getNode(step1+node1.x,0,round(stepTracker)+node1.z-1);
			tmp3	= currMap->getNode(step1+node1.x,0,round(stepTracker)+node1.z+1);
			tmp4	= currMap->getNode(step1+node1.x,0,round(stepTracker)+node1.z+2);
			tmp5	= currMap->getNode(step1+node1.x,0,round(stepTracker)+node1.z-2);
		}
		if(tmp!=0)
		{
			try
			{
				if(tmp->getState()== OBSTACLE)
				{
					if  ((tmp2 !=0 && tmp2->getState() == OBSTACLE) &&
						 (tmp3 !=0 && tmp3->getState() == OBSTACLE) &&
						 (tmp4 !=0 && tmp4->getState() == OBSTACLE) //&&
						 //(tmp5 !=0 && tmp5->getState() == OBSTACLE)
						 )
						return tmp;
				}
			} catch(std::exception err)
			{
				cout <<"Error PathController ::ObstacleExistsBetween "<<err.what()<<endl;
			}

		}
	}//end of steps
	return 0;
}


//scan through the map and find the closest hiding place from the NPC
//then call set path with the correct target to find the shortest path
void PathController::findHidingPlace()
{
	 //find open spaces that are separated from the enemy by at least one obstacle square
	//line scan each block till we find an obstacle find the 
	
	
	Point currentNodeIx(0,0,0); 
	Point enemyIx(0,0,0);
	MapNode* tmp;
	if((tmp=currMap->getNode(character->getOrigin())) ) {currentNodeIx	=tmp->getNodeIndex();}	
	if((tmp=currMap->getNode(opponent->getOrigin())) ){enemyIx			=tmp->getNodeIndex();}

	MapNode *hidingNode=0;

	float currDistance = -1;
	float tmpDistanceOpponent=-1;

	for(int i=0;i<currMap->getX();i++)
	{
		for(int j=0;i<currMap->getZ();i++)
		{
			//examine each node in map how close is it to enemy
			//how close is it to player
			//does an Obstacle exist between enemy and empty square
			MapNode *tmpNode = currMap->getNode(i,0,j);
			if(	tmpNode->getState()==EMPTY)
			{
				tmpDistanceOpponent = distanceFromNode(currentNodeIx, tmpNode->getNodeIndex() );
				if(ObstacleExistsBetween(tmpNode->getNodeIndex(),enemyIx)
					&& tmpDistanceOpponent <=distanceFromNode(enemyIx,tmpNode->getNodeIndex())
					&& enemyIx!=tmpNode->getNodeIndex() //ensure we don't run over the enemy
					&& (tmpDistanceOpponent <currDistance || currDistance==-1))
				{
					hidingNode=tmpNode;
					currDistance = tmpDistanceOpponent;
					//tmpNode->setState(OPPONENT);
					//currMap->printState();
					//tmpNode->setState(EMPTY);
				}
			}
		}//end of for j z node search
	}//end of for i x node search
	if(hidingNode!=0)
		setPathNodes(hidingNode);



}



void PathController::moveCharacter()
{
	try
	{
		if(!path.empty())
		{
			if( path[0]->itemInNode(character->getOrigin()) )//check to see if character in the node of the next 
			{
				visited.push_back(path[0]);
				advancePath();
			}else
			{ //set velocity to reach the next node
				character->setVelocity(adjustVelocity(character->getOrigin(),path[0]->getLocation(),character->getSpeed()));
			}
			character->setOrigin(character->getOrigin()+ character->getVelocity());
		}
	}catch(std::exception err)
	{
		cout <<" Exception PathController::moveCharacter "<<err.what()<<endl;
	}
}


void PathController::setCharacterStates()
{
	Point currentNodeIx(0,0,0);
	Point enemyIx(0,0,0);

	MapNode *tmp;
	if((tmp=currMap->getNode(character->getOrigin())) ) {currentNodeIx	=tmp->getNodeIndex();}	
	if((tmp=currMap->getNode(opponent->getOrigin())) ){enemyIx			=tmp->getNodeIndex();}

	float tmpDistanceOpponent = distanceFromNode(currentNodeIx, enemyIx );

	MapNode *tmp2 = ObstacleExistsBetween(currentNodeIx,enemyIx);
	if( !tmp2 
		&& tmpDistanceOpponent<opponent->getStrikeRange())
		character->setInDanger(true);
	else
	{
		character->setInDanger(false);
	//	tmp2->setState(OPPONENT);
	//	currMap->printState();
	//	tmp2->setState(OBSTACLE);
	}

	if(tmpDistanceOpponent<character->getThrowRange())
		character->setInRange(true);
	else
		character->setInRange(false);
}


void PathController::setMaxPath(int newMaxPath)
{
	maxPath = newMaxPath;
}


int PathController::getMaxPath()
{
	return maxPath;
}
