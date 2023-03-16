#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include <string>
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <vector>
#include <math.h>
#include <bitset>
#include <queue>

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

class StudentWorld : public GameWorld
{
private:
	// Main stats
	const unsigned int GOLD_PTS{ 10 }, WTR_PTS{ 100 }, SONAR_PTS{ 75 }, OIL_PTS{ 1000 }, BRIB_NORM_PTS{ 25 }, BRIB_HARD_PTS{ 50 }, BOULDER_PTS{ 500 }, IRTED_NORM_PTS{ 100 }, IRTED_HARD_PTS{ 250 };
	const unsigned int EARTH_X{ 64 }, EARTH_Y{ 60 };
	const int GOLDNOIL_SPWNX{ 64 }, GOLDNOIL_SPWNY{ 55 }; // Area coord of spawning gold and oil, used for generateCoord
	const int BOULDER_SPWNY_INIT{ 20 }, BOULDER_SPWNY_FINAL{ 56 };

	// Important stats
	int CURRENT_TICK{ 0 }; // counting the amount of move() fx call
	int KEY_HIT{ 0 }; // current key hitted
	const int SPRITE_XY{ 4 }; // objects's sprite beside earth object
	const int REQ_BOUND{ 6 }; // minimum distance between generated static_consumable objects
	const int REQ_VISIBILE{ 4 }; // minimum distance to view static consumable object
	const int REQ_CONSUME{ 3 }; // minimum distance to consume an object
	float REQ_MOVEBLOCK{ 4 }; // minimum distance (sprite) to block movement
	const int REQ_BOULDER_HIT{ 3 }; // minimum distance for an unstable boulder to hit an entity
	const int REQ_COLLIDE_PROTESTER{ 4 };

	// Misc stats
	int REQ_TICK_SPWN_protester{ 0 }, REQ_TICK_REST_protester{ 0 }, spwnChance_hardcore{ 0 };
	int thisTickSpwnProtester{ 0 };
	int ratioSonarOrWater{ 5 };
	int spwnChance_goodie{ 0 };
	int tick_goodieAvail{ 0 };
	const int hollowInitX{ 30 }, hollowFinalX{ 34 }, hollowInitY{ 4 }, hollowFinalY{ 60 }; 

	// Object amount and types
	int gold_amt{ 0 }, barrel_amt{ 0 }, boulder_amt{ 0 }; // STATIC OBJECT
	int waterPool_amt{ 0 }, protester_amt{ 0 }; // DYNAMIC OBJECT
	const int static_object_type{ 3 }; // all static object (consumable) + Earth
	const int dynamic_object_type{ 3 };

	// Hollow regions region cntr for water pool allocation
	std::bitset<61 * 61> emptyField{ false }; // 2d array based bitset w*h 0-60
	std::bitset<61 * 61> beenHere{ false }; // for locating protester exit path
	struct traversedLog {
		int x, y;
		traversedLog(const int& xVar, const int& yVar) : x{ xVar }, y{ yVar }{};
		~traversedLog() {};
	};

	// Object container
	tunnelMan* mainChar{ nullptr };
	std::vector<int> vID_cntr;
	std::vector<std::shared_ptr<void>> genericVec; // conversionVec will only works if a specified type is derived from T (eg regularProtestor derived from object -> vector<regProtester*> converts to vector<object*>
	std::vector<std::shared_ptr<object>> goldNgt, barrelOil, boulder, waterPl, sonarK, dymcGold, waterSpurt;
	std::vector<std::shared_ptr<regularProtester>> NPC;
	std::vector<std::vector<Earth*>>oilField;
	enum { FREE_FALL = 10, GOLD_DYNAMIC = 11 };
public:
	StudentWorld(std::string assetDir) : GameWorld(assetDir) {};
	~StudentWorld() {
		cleanUp();
	};

	virtual int init();
	virtual int move();
	virtual void cleanUp();

	// init()
	void constructStaticObjectField();
	void constructEmptyField();
	void constructEarthField();
	void generatePortionalCoord(int& x, int& y, const int& divX_portion, const int& divY_portion, std::vector<std::vector<bool>>& genCoord, bool&& isBoulder);
	void setGlobalIDVector();

	// init() and move()
	void setDisplayText();

	// move()
	void idToVec(const int& entityID);
	int&& XYtoIndex(const int& x, const int& y);
	void IndexToXY(const int& index, int& x, int& y);

	/* ---- NPC functions ---*/
	void spawnProtester(bool);
	void BFS_findPath(const int& npcX, const int& npcY, std::vector<traversedLog>& scoutLog);
	void NPCgenerateNewPath(unsigned int& newDist, unsigned int& newDir, const int& npcX, const int& npcY, const bool& isLeaving);
	void checkPlayerDetection(const int& playerX, const int& playerY, const int& npcX, const int& npcY, bool& canEngagePlayer, GraphObject::Direction& whatDir);

	// move()
	int checkSpecialKey();
	void updateEntityState(const bool&& updateWithoutKeyHit);
	void GenerateRandDynamicObj();
	void updateEmptySpace();
	bool  checkCollision(const int& ID_one, const int& ID_two);
	const int interactSound(const int& targetID_one, const int& targetID_two);
	void modify_info(const int& targetID_one, const int& targetID_two, int manual_var = 0);

	// init()
	void cleanUpObjectAndMisc();
};

#endif // STUDENTWORLD_H_
