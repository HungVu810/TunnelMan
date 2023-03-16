#include "Actor.h"
#include "StudentWorld.h"

// Students:  Add code to this file (if you wish), Actor.h, StudentWorld.h, and StudentWorld.cpp

// Entity
bool Entity::getActiveState() const noexcept { return activeStatus; }
void Entity::setActiveState(const bool&& state) noexcept { activeStatus = state; }
void Entity::disable() { // Object is no longer being displayed to the screen but the Object still exist
	setVisible(false);
	setActiveState(false);
}

// Actor
int Actor::getHitPoints() const noexcept { return hitPoints; }
void Actor::setHitPoints(const int& amt) noexcept { hitPoints = amt; }
void Actor::modifiyHitPoints(const int&& amt) noexcept {
	if (hitPoints + amt > 0) hitPoints += amt;
	else hitPoints = 0;
}
int Actor::getGold()const noexcept { return goldAmt; }
void Actor::modifyGold(const int& amt) { goldAmt += amt; }

// object
object::object(const int id, int x, int y, Direction dir, double sz, unsigned int d) :
	Entity(id, x, y, dir, sz, d) {};

// TunnelMan
tunnelMan::tunnelMan(int x, int y, const int id, Direction dir, double sz, unsigned int d) 
	: health( 100 ), waterCharge(5), sonarCharge( 1 ), barrelCollected( 0 ), spriteRotated( false ),
	  up_dirBlock( false ), down_dirBlock( false ), left_dirBlock( false ), right_dirBlock( false ), Actor::Actor( id, x, y, dir, sz, d ){
	setVisible(true);
	setHitPoints(10);
	setActiveState(true);
}
bool tunnelMan::rotated() const noexcept { return spriteRotated; }
bool tunnelMan::dirLocked() const noexcept {
	if (up_dirBlock || down_dirBlock || left_dirBlock || right_dirBlock) return true;
	return false;
}
void tunnelMan::freeDirLock() noexcept { up_dirBlock = down_dirBlock = left_dirBlock = right_dirBlock = false; }
void tunnelMan::lockDir(int&& up, int&& down, int&& left, int&& right) noexcept { // -1 preserve the current lock status of the direction
	if (up != -1) up_dirBlock = up; // create an enum of dirNotLock, dirNotModify, dirLock ...
	if (down != -1) down_dirBlock = down;
	if (left != -1) left_dirBlock = left;
	if (right != -1) right_dirBlock = right;
}
void tunnelMan::reset_isRotated() noexcept { spriteRotated = false; }
void tunnelMan::doSomething(const int& keyHit, const int& maxX, const int& maxY) {
	// ****** TunnelMan's coord is at the lower left of the sprite
	// TunnelMan sprite size is 4
	int currX{ getX() }, currY{ getY() };
	switch (keyHit) {
	case KEY_PRESS_UP:
		if (getDirection() == up && currY < maxY && !up_dirBlock) moveTo(getX(), getY() + 1); // yAxis [0, maxY] inclusive
		else {
			setDirection(up);
			spriteRotated = true;
		}
		break;
	case KEY_PRESS_DOWN:
		if (getDirection() == down && currY > 0 && !down_dirBlock) moveTo(getX(), getY() - 1);
		else {
			setDirection(down);
			spriteRotated = true;
		}
		break;
	case KEY_PRESS_LEFT:
		if (getDirection() == left && currX > 0 && !left_dirBlock) moveTo(getX() - 1, getY());
		else {
			setDirection(left);
			spriteRotated = true;
		}
		break;
	case KEY_PRESS_RIGHT:
		if (getDirection() == right && currX < maxX - 4 && !right_dirBlock) moveTo(getX() + 1, getY());  // Minus 4 to fit the spirt within earth bound
		else {
			setDirection(right);
			spriteRotated = true;
		}
		break;
	default: break;
	}
}
void tunnelMan::modify_infos(const int& object_type, const int&& modif_type) noexcept { // modify type = k to increase, -k to decrease
	switch (object_type) {
	case TID_PLAYER: health += modif_type; break;
	case TID_BARREL: barrelCollected += modif_type; break;
	case TID_GOLD: modifyGold(modif_type); break;
	case TID_WATER_SPURT: waterCharge += modif_type; break;
	case TID_SONAR: sonarCharge += modif_type; break;
	default: break;
	}
}
int tunnelMan::getInfos(const int& object_type) const noexcept {
	switch (object_type) {
	case TID_PLAYER: return health;
	case TID_BARREL: return barrelCollected;
	case TID_GOLD: return getGold();
	case TID_WATER_SPURT: return waterCharge;
	case TID_SONAR:  return sonarCharge;
	}
}

// Regular protester
regularProtester::regularProtester(int thisTick, int x, int y, const int id, Direction dir, double sz, unsigned int d) :
	object( id, x, y, dir, sz, d ), Actor( id, x, y, dir, sz, d ), currDir( dir ), distCurrDir( 60 ), thisMovementDelayTick( thisTick ){ // go to the left of the screen
	Entity::setVisible(true);
	Entity::setActiveState(true);
	setHitPoints(5);
};
void regularProtester::doSomething(const int& playerX, const int& playerY)  { // override object, for adjusting current dir if detects player on path
	destinationReached = true; // reset destination every time encounter player
	if (Entity::getDirection() != encounterDir) {
		Entity::setDirection(encounterDir);
	}
	else {
		switch (encounterDir) {
		case none: break;
		case up: {
			if (Entity::getY() + SPRITE_XY < playerY) {
				Entity::moveTo(Entity::getX(), Entity::getY() + 1);
			}
		}break;
		case down: {
			if (Entity::getY() > playerY + SPRITE_XY) {
				Entity::moveTo(Entity::getX(), Entity::getY() - 1);
			}
		}break;
		case left: {
			if (Entity::getX() > playerX + SPRITE_XY) {
				Entity::moveTo(Entity::getX() - 1, Entity::getY());
			}
		}break;
		case right: {
			if (Entity::getX() + SPRITE_XY < playerX) {
				Entity::moveTo(Entity::getX() + 1, Entity::getY());
			}
		} break;
		}
	}
};
void regularProtester::doSomething(const int& maxX, const int& maxY, const int& tick)  { // override Actor, for normal movement
	int npcX{ Entity::getX() }, npcY{ Entity::getY() };
	if (npcX == maxX - SPRITE_XY && npcY == maxY && isLeaving) Entity::disable();
	else {
		if (Entity::getDirection() != currDir) {
			Entity::setDirection(currDir);
		}
		switch (currDir) { // if destinationReached, updateEntityState() will update new path
		case up: {
			if (npcY < maxY && npcY < destination)Entity::moveTo(npcX, npcY + 1);
			else destinationReached = true;
		}break;
		case down: {
			if (npcY > 0 && npcY > destination) Entity::moveTo(npcX, npcY - 1);
			else destinationReached = true;
		} break;
		case left: {
			if (npcX > 0 && npcX > destination) Entity::moveTo(npcX - 1, npcY);
			else destinationReached = true;
		}break;
		case right: {
			if (npcX < maxX - SPRITE_XY && npcX < destination)	 Entity::moveTo(npcX + 1, npcY);
			else destinationReached = true;
		}break;
		}
	}
};
bool regularProtester::reachedDest() const noexcept { return destinationReached; }
void regularProtester::resetReachedDest() noexcept { destinationReached = false; }
void regularProtester::increaseNonRestTick() noexcept { thisNonRestTick++; }
void regularProtester::resetNonRestTick() noexcept { thisNonRestTick = 0; }
void regularProtester::enableLeaving() noexcept {
	isLeaving = true;
	Entity::setActiveState(false);
	destinationReached = true;
}

void  regularProtester::setNewDistAndDir(const int& newDist, const int& newDir) { // 0: up, 1: down, 2: left, 3: right
	distCurrDir = newDist;
	switch (newDir) {
	case 0: currDir = up; destination = Entity::getY() + distCurrDir; break;
	case 1: currDir = down; destination = Entity::getY() - distCurrDir; break;
	case 2: currDir = left; destination = Entity::getX() - distCurrDir; break;
	case 3: currDir = right; destination = Entity::getX() + distCurrDir; break;
	}
}
void regularProtester::setMovementDelayTick(const int& whatTick) noexcept { thisMovementDelayTick = whatTick; }
void regularProtester::setShoutingStatus(const bool&& b) noexcept { isShouting = b; }
void regularProtester::setEncounterStatus(const bool&& b, const Direction& whatDir) noexcept {
	encounterPlayer = b;
	encounterDir = whatDir;
}

bool regularProtester::getEncounterStatus() const noexcept { return encounterPlayer; }
bool regularProtester::getShoutingStatus()const noexcept { return isShouting; }
int regularProtester::getMovementDelayTick()const noexcept { return thisMovementDelayTick; }
bool regularProtester::getLeavingStatus() const noexcept { return isLeaving; }
int regularProtester::getNonRestTick() const noexcept { return thisNonRestTick; }

// Hardcore protester
hardcoreProtester::hardcoreProtester(unsigned int thisTick, int x, int y, const int id, Direction dir, double sz, unsigned int d) :
	regularProtester( thisTick, x, y, id, dir, sz, d ){
	Entity::setVisible(true);
	Entity::setActiveState(true);
	setHitPoints(20);
};
void hardcoreProtester::setMovementDelay(const int& thisTick) {
	goldDelayTick = thisTick;
}

// Earth
Earth::Earth(int x, int y, const int id, Direction dir, double sz, unsigned int d) :
	object( id, x, y, dir, sz, d ) {
	setVisible(true);
};
void Earth::doSomething(const int& x, const int& y) {};

// Boulder
Boulder::Boulder(int x, int y, const int id, Direction dir, double sz, unsigned int d) :
	object(id, x, y, dir, sz, d), isStable(true) {
	setVisible(true);
};
void Boulder::doSomething(const int& lastestTick, const int& maxY)  {
	currentTick = lastestTick;
	if (!isStable && currentTick - unstableTick >= REQ_DELAY_TICK) {// if boulder is not in stable stage
		if (getActiveState()) { // is active
			getY() ? moveTo(getX(), getY() - 1) : disable(); // keep move down if y != 0
		}
	}
};
void Boulder::setStability(const bool&& state) noexcept { isStable = state; }
bool Boulder::getStability() const noexcept { return isStable; }
void Boulder::setInitUnstableTick(const int& tick) noexcept { unstableTick = tick; }
bool Boulder::meetRequiredDelay() {
	return currentTick - unstableTick == REQ_DELAY_TICK;
}

// Water squirt
waterSquirt::waterSquirt(int x, int y, int maxTravelDistance, GraphObject::Direction whatDir, const int id, double sz, unsigned int d)
	: object(id, x, y, whatDir, sz, d), traversalDirection(whatDir) {
	setVisible(true); // maxTravelDistance - sprite_length because the squirt sprite is the location of tunelMan + 4
	switch (traversalDirection) {
	case up: destination = y + maxTravelDistance; break;
	case down: destination = y - maxTravelDistance; break;
	case left: destination = x - maxTravelDistance; break;
	case right: destination = x + maxTravelDistance; break;
	}
};
void waterSquirt::doSomething(const int& maxX, const int& maxY) {
	int squirtX{ getX() }, squirtY{ getY() };
	if (getActiveState()) {
		switch (traversalDirection) {
		case up: {
			if (squirtY < maxY && squirtY < destination) moveTo(squirtX, squirtY + 1);
			else disable();
		}break;
		case down: {
			if (squirtY > 0 && squirtY > destination) moveTo(squirtX, squirtY - 1);
			else disable();
		} break;
		case left: {
			if (squirtX > 0 && squirtX > destination) moveTo(squirtX - 1, squirtY);
			else disable();
		}break;
		case right: {
			if (squirtX < maxX - SPRITE_WIDTH && squirtX < destination) moveTo(squirtX + 1, squirtY);
			else disable();
		}break;
		}
	}
}

// Static Consumable
int static_consumable::REQ_VISIBLE = 4;
bool static_consumable::distVisIsModified = false;
static_consumable::static_consumable(const int id, int x, int y, Direction dir, double sz, unsigned int d) :
	object( id, x, y, dir, sz, d ) {
	setVisible(false);
};
void static_consumable::doSomething(const int& tnlManX, const int& tnlManY) noexcept {
	if (getActiveState()) {// gold must be active
		double dist_obj2tnlMan = std::sqrt(std::pow(getX() - tnlManX, 2) + std::pow(getY() - tnlManY, 2));
		// NOTE: req_dist_vis changed if use sonar
		if (dist_obj2tnlMan <= REQ_VISIBLE && getActiveState()) {
			setVisible(true);
		}
	}
}
void static_consumable::modifyRadVisibile(const int dist) noexcept {
	REQ_VISIBLE = dist;
	distVisIsModified = true;
}
bool static_consumable::distVisibleIsModified() noexcept {
	return distVisIsModified;
}

// Gold Nugget
goldNugget::goldNugget(int x, int y, const int id, Direction dir, double sz, unsigned int d) :
	static_consumable(id, x, y, dir, sz, d) {};

// Oil Barrel
barrelsOfOil::barrelsOfOil(int x, int y, const int id, Direction dir, double sz, unsigned int d) :
	static_consumable(id, x, y, dir, sz, d) {};


// Dynamic consumable
dynamic_consumable::dynamic_consumable(const int id, int x, int y, Direction dir, double sz, unsigned int d, unsigned int init_tick, int existance_tick) :
	REQ_WAIT{ existance_tick }, initialTick{ (int)init_tick }, static_consumable{ id, x, y, dir, sz, d } {
	setVisible(true);
};
void dynamic_consumable::doSomething(const int& latestTick, const int& arg2) noexcept {
	if (getActiveState()) { // is active
		if (latestTick - initialTick == REQ_WAIT) {
			disable();
		}
	}
}

// Water pool
waterPool::waterPool(int x, int y, unsigned int CURR_TICK, int REQ_EXIST_TICK, const int id, Direction dir, double sz, unsigned int d)
	: dynamic_consumable{ id, x, y, dir, sz, d, CURR_TICK, REQ_EXIST_TICK } {};

// Sonar kit
sonarKit::sonarKit(int x, int y, unsigned int CURR_TICK, int REQ_EXIST_TICK, const int id, Direction dir, double sz, unsigned int d) : 
	dynamic_consumable{ id, x, y, dir, sz, d , CURR_TICK, REQ_EXIST_TICK } {};

dynamicGold::dynamicGold(int x, int y, unsigned int CURR_TICK, int REQ_EXIST_TICK, const int id, Direction dir , double sz, unsigned int d): 
	dynamic_consumable{ id, x, y, dir, sz, d , CURR_TICK, REQ_EXIST_TICK } {};