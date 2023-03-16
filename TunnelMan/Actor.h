#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include <list>
#include <memory>
#include <vector>
#include <math.h>
#include <bitset>

class Entity : public GraphObject {
private:
	bool activeStatus;
public:
	Entity(const int id = 0, int x = 0, int y = 0, Direction dir = right, double sz = 1.0, unsigned int d = 1.0) :
		GraphObject{ id, x, y, dir, sz, d }, activeStatus{ true }{};
	bool getActiveState() const noexcept;
	void setActiveState(const bool&& state) noexcept;
	void disable();
	~Entity() {};
};
class Actor : public Entity {
private:
	int hitPoints, goldAmt;
public:
	Actor(const int id = 0, int x = 0, int y = 0, Direction dir = right, double sz = 1.0, unsigned int d = 1.0) :
		Entity{ id, x, y, dir, sz, d }, goldAmt{ 0 }, hitPoints{ 0 }{};
	virtual ~Actor() {};
	int getHitPoints() const noexcept;
	void setHitPoints(const int& amt) noexcept;
	void modifiyHitPoints(const int&& amt) noexcept;
	int getGold()const noexcept;
	void modifyGold(const int& amt);
	virtual void doSomething(const int&, const int&, const int&) = 0;
	//virtual void disable() = 0; // make sure to move to the edge corner (for the protestor) first before disable
};
class object : public Entity {
private:
public:
	object(const int id = 0, int x = 0, int y = 0, Direction dir = right, double sz = 1.0, unsigned int d = 1.0);
	virtual ~object() {};
	virtual void doSomething(const int&, const int&) = 0;
};

class tunnelMan : public Actor {
private:
	unsigned int health;
	unsigned int waterCharge, sonarCharge, barrelCollected;
	bool spriteRotated;
	bool up_dirBlock, down_dirBlock, left_dirBlock, right_dirBlock;
public:
	tunnelMan(int x = 0, int y = 0, const int id = TID_PLAYER, Direction dir = right, double sz = 1.0, unsigned int d = 0);
	~tunnelMan() {};
	bool rotated() const noexcept;
	bool dirLocked() const noexcept;
	void freeDirLock() noexcept;
	void lockDir(int&& up, int&& down, int&& left, int&& right) noexcept;
	void reset_isRotated() noexcept;
	void doSomething(const int& keyHit, const int& maxX, const int& maxY);
	void modify_infos(const int& object_type, const int&& modif_type) noexcept;
	int getInfos(const int& object_type) const noexcept;
};

class regularProtester : public Actor, public object {
private:
	struct traversedLoc { int x, y; traversedLoc(const int& argX, const int& argY) : x{ argX }, y{ argY }{}; };
	std::vector<traversedLoc> pathTaken;
	Direction currDir, encounterDir;
	int SPRITE_XY{ 4 };
	int  distCurrDir, destination, thisMovementDelayTick{ 0 }, thisNonRestTick{ 0 };
	bool isMoving{ true }, isLeaving{ false }, isBribed{ false }, isIrritated{ false }, isShouting{ false };
	bool destinationReached{ false }, encounterPlayer{ false };
public:
	regularProtester(int thisTick = 0, int x = 60, int y = 60, const int id = TID_PROTESTER, Direction dir = left, double sz = 1.0, unsigned int d = 0);
	virtual ~regularProtester() {};

	void doSomething(const int& playerX, const int& playerY) override;
	void doSomething(const int& maxX, const int& maxY, const int& tick) override;
	bool reachedDest() const noexcept;
	void resetReachedDest() noexcept;
	void enableLeaving() noexcept;
	void increaseNonRestTick() noexcept;
	void resetNonRestTick() noexcept;

	// NOTE: remove bribing and irritating, if these are true set active status to false
	void setNewDistAndDir(const int& newDist, const int& newDir);
	void setMovementDelayTick(const int& whatTick) noexcept;
	void setShoutingStatus(const bool&& b) noexcept;
	void setEncounterStatus(const bool&& b, const Direction& whatDir) noexcept;

	bool getEncounterStatus() const noexcept;
	bool getShoutingStatus()const noexcept;
	int getMovementDelayTick()const noexcept;
	bool getLeavingStatus() const noexcept;
	int getNonRestTick() const noexcept;


};
class hardcoreProtester : public regularProtester {
private:
	int goldDelayTick{ 0 };
public:
	hardcoreProtester(unsigned int thisTick = 0, int x = 60, int y = 60, const int id = TID_HARD_CORE_PROTESTER, Direction dir = left, double sz = 1.0, unsigned int d = 0);
	~hardcoreProtester() {};
	void setMovementDelay(const int& thisTick);
};

class Earth : public object {
private:
public:
	Earth(int x = 0, int y = 0, const int id = TID_EARTH, Direction dir = right, double sz = 0.25, unsigned int d = 3.0);
	~Earth() {}
	void doSomething(const int& x, const int& y) override;
};
class Boulder : public object {
private:
	unsigned int unstableTick{ 0 }, currentTick{ 0 }, REQ_DELAY_TICK{ 30 };
	bool isStable;
public:
	Boulder(int x = 0, int y = 0, const int id = TID_BOULDER, Direction dir = down, double sz = 1.0, unsigned int d = 1.0);
	virtual ~Boulder() {};
	void doSomething(const int& lastestTick, const int& maxY);
	void setStability(const bool&& state) noexcept;
	bool getStability() const noexcept;
	void setInitUnstableTick(const int& tick) noexcept;
	bool meetRequiredDelay();
};
class waterSquirt : public object {
private:
	int destination{ 0 };
	GraphObject::Direction traversalDirection;
public:
	waterSquirt(int x = 0, int y = 0, int maxTravelDistance = 0, GraphObject::Direction whatDir = right, const int id = TID_WATER_SPURT, double sz = 1.0, unsigned int d = 1.0);
	virtual ~waterSquirt() {};
	void doSomething(const int& maxX, const int& maxY) override;
};

class static_consumable : public object { // NOTE : defaut argument  static_consumable dir (right), sz (1.0), d(2)
private:
	static int REQ_VISIBLE; // required distance to make static_consumable object visible
	static bool distVisIsModified;
public:
	static_consumable(const int id = 0, int x = 0, int y = 0, Direction dir = right, double sz = 1.0, unsigned int d = 2.0);
	virtual ~static_consumable() {}
	static void modifyRadVisibile(const int dist) noexcept;
	static bool distVisibleIsModified() noexcept;
	virtual void doSomething(const int& tnlManX, const int& tnlManY) noexcept override;
};

class goldNugget : public static_consumable {
private:
public:
	goldNugget(int x = 0, int y = 0, const int id = TID_GOLD, Direction dir = right, double sz = 1.0, unsigned int d = 2.0);
	virtual ~goldNugget() {};
};
class barrelsOfOil : public static_consumable {
private:
public:
	barrelsOfOil(int x = 0, int y = 0, const int id = TID_BARREL, Direction dir = right, double sz = 1.0, unsigned int d = 2.0);
	virtual ~barrelsOfOil() {};
};

class dynamic_consumable : public static_consumable {
private:
	int initialTick, REQ_WAIT; // wait for 30 ticks before disappear
public:
	dynamic_consumable(const int id = 0, int x = 0, int y = 0, Direction dir = right, double sz = 1.0, unsigned int d = 1.0, unsigned int init_tick = 0, int existance_tick = 0);
	virtual ~dynamic_consumable() {};
	void doSomething(const int& latestTick, const int& arg2) noexcept override;
};
class waterPool : public dynamic_consumable {
private:
public:
	waterPool(int x = 0, int y = 0, unsigned int CURR_TICK = 0, int REQ_EXIST_TICK = 0, const int id = TID_WATER_POOL, Direction dir = right, double sz = 1.0, unsigned int d = 2.0);
	virtual ~waterPool() {};
};
class sonarKit : public dynamic_consumable {
private:
public:
	sonarKit(int x = 0, int y = 0, unsigned int CURR_TICK = 0, int REQ_EXIST_TICK = 0, const int id = TID_SONAR, Direction dir = right, double sz = 1.0, unsigned int d = 2.0);
	virtual ~sonarKit() {};
};
class dynamicGold : public dynamic_consumable {
private:
public:
	dynamicGold(int x = 0, int y = 0, unsigned int CURR_TICK = 0, int REQ_EXIST_TICK = 0, const int id = TID_GOLD, Direction dir = right, double sz = 1.0, unsigned int d = 2.0);
	virtual ~dynamicGold() {};
};



//The Tunnelman
//Regular Protesters
// Hardcore Protesters
// Squirts of water(that can be shot by the Tunnelman)
// Barrels of oil
// Boulders
// Gold Nuggets
// Sonar kits
// Water refills(that can be picked up to refill the Tunnelman’s squirt gun)
// Earth
// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

#endif // ACTOR_H_
