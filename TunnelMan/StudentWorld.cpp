#include "StudentWorld.h"
#include <string>
using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

int StudentWorld::init() {
	// setup object amount
	srand(time(NULL));
	oilField.reserve(EARTH_Y);
	gold_amt = std::max(static_cast<int>(5 - (getLevel() / 2)), 2);						goldNgt.reserve(gold_amt);
	barrel_amt = std::min(static_cast<int>(2 + getLevel()), 21);							barrelOil.reserve(barrel_amt);
	boulder_amt = std::min(static_cast<int>(getLevel() / 2 + 2), 9);					boulder.reserve(boulder_amt);
	tick_goodieAvail = std::max(100, static_cast<int>(300 - (10 * getLevel())));

	REQ_TICK_SPWN_protester = std::max(25, static_cast<int>(200 - getLevel()));
	REQ_TICK_REST_protester = std::max(0, 3 - static_cast<int>(getLevel() / 4));
	protester_amt = std::min(15, static_cast<int>(2 + getLevel() * 1.5));
	spwnChance_hardcore = std::min(90, static_cast<int>(90, getLevel() * 10 + 30));
	spwnChance_goodie = getLevel() * 25 + 300;

	mainChar = new tunnelMan{ 30, 60 };
	setGlobalIDVector();
	constructStaticObjectField();
	constructEmptyField();
	constructEarthField();
	setDisplayText();
	return GWSTATUS_CONTINUE_GAME;
}
 int StudentWorld::move() {
	// GET KEY FIRST, THEN MOVE OBJECT & PLAY SOUND SIMUTANEOUSLY
	CURRENT_TICK++;
	if (mainChar->getHitPoints() && mainChar->getInfos(TID_PLAYER)) {
		if (mainChar->getInfos(TID_BARREL) != barrel_amt) {
			spawnProtester(false);				// update without keyhit 
				updateEntityState(true);		// update without keyhit
				GenerateRandDynamicObj();		// update without keyhit
				if (getKey(KEY_HIT)) {
					int status = checkSpecialKey();			// check for special key, normal movement keys are dealt with already
						if (status != -1)  return status;
						updateEntityState(false);	// update only if keyhit
						updateEmptySpace();
				}
			for (int i{ 0 }; i < vID_cntr.size(); i++) {
				for (int k{ 0 }; k < vID_cntr.size(); k++) {
					if (k != i && checkCollision(vID_cntr[i], vID_cntr[k])) {
						modify_info(vID_cntr[i], vID_cntr[k]);
						playSound(interactSound(vID_cntr[i], vID_cntr[k]));
					}
				}
			}
			setDisplayText();
			cleanUpObjectAndMisc(); // DELETE NON ACTIVE OBJECT
			return GWSTATUS_CONTINUE_GAME;
		}
		return GWSTATUS_FINISHED_LEVEL;
	}
	else {
		decLives();
		return GWSTATUS_PLAYER_DIED;
	}
};
 void  StudentWorld::cleanUp() {
	 emptyField.reset();
	 beenHere.reset();
	 mainChar->setActiveState(false);
	 mainChar->disable();
	 for (int i{ 0 }; i < vID_cntr.size(); i++) {
		 idToVec(vID_cntr[i]);
		 for (int k{ 0 }; k < genericVec.size(); k++) {
			 static_cast<Entity*>(genericVec[k].get())->Entity::setActiveState(false);
			 static_cast<Entity*>(genericVec[k].get())->Entity::disable();
		 }
	 }
	 for (auto i{ oilField.begin() }; i != oilField.end(); i++) {
		 for (auto k{ i->begin() }; k != i->end(); k++) {
			 (*k)->setActiveState(false);
			 (*k)->disable();
		 }
	 }
	 delete mainChar;
	 vID_cntr.clear();
	 genericVec.clear();
	 goldNgt.clear(), barrelOil.clear(), boulder.clear(), waterPl.clear(), sonarK.clear(), dymcGold.clear(), waterSpurt.clear();
	 NPC.clear();
	 for (auto i{ oilField.begin() }; i != oilField.end(); i++) {
		 auto k{ *i };
		 for (auto j{ k.begin() }; j != k.begin(); j++) {
			 delete* j;
		 }
		 k.clear();
	 }
	 oilField.clear();
 };


 // init ()
 void StudentWorld::constructStaticObjectField() {
	 int divX_portion{ GOLDNOIL_SPWNX / REQ_BOUND };
	 int divY_portion{ GOLDNOIL_SPWNY / REQ_BOUND };
	 int divY_boulderPortion{ (BOULDER_SPWNY_FINAL - BOULDER_SPWNY_INIT) / REQ_BOUND };
	 std::vector<std::vector<bool>> portionalCoord; // portional coordinates of statically generated objects
	 for (int i{ 0 }; i < divY_portion; i++, portionalCoord.push_back(std::vector<bool>(divX_portion, false))); // initialize genCoord 2d vec
	 for (int hollowedX{ 5 }, hollowedY{ 1 }; hollowedY < divY_portion; hollowedY++) { portionalCoord[hollowedY][hollowedX] = true; }; // occupy hollowed region
	 for (int i{ 0 }, object_amt{ 0 }; i < static_object_type; i++) { // excluding earth
		 switch (i) {
		 case 0: object_amt = boulder_amt; break;
		 case 1: object_amt = gold_amt; break;
		 case 2: object_amt = barrel_amt; break;
		 }
		 for (int k{ 0 }; k < object_amt; k++) {
			 int portionalX, portionalY;
			 generatePortionalCoord(portionalX, portionalY, divX_portion, (i == 0 ? divY_boulderPortion : divY_portion), portionalCoord, i == 0);
			 switch (i) {
			 case 0: boulder.push_back(std::make_shared<Boulder>(REQ_BOUND * portionalX, REQ_BOUND * portionalY)); break;
			 case 1: goldNgt.push_back(std::make_shared<goldNugget>(REQ_BOUND * portionalX, REQ_BOUND * portionalY)); break;
			 case 2: barrelOil.push_back(std::make_shared<barrelsOfOil>(REQ_BOUND * portionalX, REQ_BOUND * portionalY)); break;
			 }
		 }
	 }
 }
 void StudentWorld::constructEmptyField() {
	 for (int i{ hollowInitY }; i <= hollowFinalY; i++) {
		 if (i == hollowFinalY) {
			 for (int k{ 0 }; k <= EARTH_X - SPRITE_XY; k++) {
				 emptyField[XYtoIndex(k, i)] = true;
			 }
		 }
		 else emptyField[XYtoIndex(hollowInitX, i)] = true;
	 }
 }
 void StudentWorld::constructEarthField() { // CHECK FOR BAD ALLOC
	 for (int yAxis{ 0 }; yAxis < EARTH_Y; yAxis++) {
		 oilField.push_back(std::vector<Earth*>(EARTH_X));
		 for (int xAxis{ 0 }; xAxis < EARTH_X; xAxis++) {
			 oilField[yAxis][xAxis] = new Earth{ xAxis, yAxis };
			 for (auto iterO{ boulder.begin() }; iterO != boulder.end(); iterO++) {
				 if (0 <= yAxis - (*iterO)->getY() && yAxis - (*iterO)->getY() <= SPRITE_XY - 1) {
					 if (0 <= xAxis - (*iterO)->getX() && xAxis - (*iterO)->getX() <= SPRITE_XY - 1) {
						 oilField[yAxis][xAxis]->disable();
					 }
				 }
			 }
		 }
	 }
	 for (int i{ hollowInitY }; i < hollowFinalY; i++) {
		 for (int k{ hollowInitX }; k < hollowFinalX; k++) {
			 oilField[i][k]->disable();
		 }
	 }
 }
 void StudentWorld::generatePortionalCoord(int& x, int& y, const int& divX_portion, const int& divY_portion, std::vector<std::vector<bool>>& genCoord, bool&& isBoulder) {
	 x = rand() % divX_portion;
	 if (isBoulder) {
		 y = (rand() % divY_portion) + (BOULDER_SPWNY_INIT / REQ_BOUND);
	 }
	 else y = rand() % divY_portion;
	 bool coordNotFound{ true };
	 while (coordNotFound) {
		 if (!genCoord[y][x]) {// not occupied
			 genCoord[y][x] = true;
			 coordNotFound = false;
		 }
		 else {
			 x = (x + 1) % divX_portion;
			 if (x == 0) { // change y if x return to the starting position
				 if (isBoulder) {
					 y = BOULDER_SPWNY_INIT;
				 }
				 else y = (y + 1) % divY_portion;
			 }
		 }
	 }
 }
 void StudentWorld::setGlobalIDVector() {
	 vID_cntr.push_back(TID_EARTH);
	 vID_cntr.push_back(TID_PLAYER);
	 vID_cntr.push_back(TID_PROTESTER);
	 vID_cntr.push_back(TID_HARD_CORE_PROTESTER);
	 vID_cntr.push_back(TID_WATER_SPURT);
	 vID_cntr.push_back(TID_WATER_POOL);
	 vID_cntr.push_back(TID_SONAR);
	 vID_cntr.push_back(GOLD_DYNAMIC);
	 vID_cntr.push_back(TID_BOULDER);
	 vID_cntr.push_back(TID_BARREL);
	 vID_cntr.push_back(TID_GOLD);
 }


 // init() and move()
 void StudentWorld::setDisplayText() {
	 std::string prompt{ "" };
	 const int FIELD_NAME{ 8 };
	 int level = getLevel();																		// 2 digit long
	 int lives = getLives();																		// 1 digit long
	 int health = mainChar->getInfos(TID_PLAYER);						// 3 digit long w/ %
	 int squirts = mainChar->getInfos(TID_WATER_SPURT);		// 2 digit long
	 int gold = mainChar->getInfos(TID_GOLD);								// 2 digit long
	 int barrelsLeft = barrel_amt;															// 2 digit long
	 int sonar = mainChar->getInfos(TID_SONAR);							// 2 digit long
	 int score = getScore();																		// 6 digit long with leading 0s
	 auto str_format = [](const std::string&& field, const int& val, bool&& fill_space, const std::string&& xtra, const int&& digit_span)-> std::string {
		 std::stringstream curr_field_stream{};
		 if (fill_space) curr_field_stream << field << std::setfill('0') << std::setw(digit_span) << val << xtra;
		 else curr_field_stream << field << std::setw(digit_span) << val << xtra;
		 return curr_field_stream.str();
	 };
	 prompt.append(str_format("Lvl: ", level, false, " ", 2));
	 prompt.append(str_format("Lives: ", lives, false, " ", 1));
	 prompt.append(str_format("Hlth: ", health, false, "% ", 3));
	 prompt.append(str_format("Wtr: ", squirts, false, " ", 2));
	 prompt.append(str_format("Gold: ", gold, false, " ", 2));
	 prompt.append(str_format("Oil Left: ", barrelsLeft - mainChar->getInfos(TID_BARREL), false, " ", 2));
	 prompt.append(str_format("Sonar: ", sonar, false, " ", 2));
	 prompt.append(str_format("Score: ", score, true, "", 6));
	 setGameStatText(prompt); //display game stats
 }


 // move()
 void StudentWorld::idToVec(const int& entityID) { // use shared ptr in here
	 switch (entityID) {
	 case TID_WATER_SPURT: {
		 genericVec.reserve(waterSpurt.size());
		 for (int i{ 0 }; i < waterSpurt.size(); i++) genericVec.push_back(std::shared_ptr<void>(waterSpurt[i]));
	 } break;
	 case TID_BOULDER: {
		 genericVec.reserve(boulder.size());
		 for (int i{ 0 }; i < boulder.size(); i++) genericVec.push_back(std::shared_ptr<void>(boulder[i]));
	 } break;
	 case TID_BARREL: {
		 genericVec.reserve(barrelOil.size());
		 for (int i{ 0 }; i < barrelOil.size(); i++) genericVec.push_back(std::shared_ptr<void>(barrelOil[i]));
	 } break;
	 case TID_GOLD: {
		 genericVec.reserve(goldNgt.size());
		 for (int i{ 0 }; i < goldNgt.size(); i++) genericVec.push_back(std::shared_ptr<void>(goldNgt[i]));
	 } break;
	 case TID_SONAR: {
		 genericVec.reserve(sonarK.size());
		 for (int i{ 0 }; i < sonarK.size(); i++) genericVec.push_back(std::shared_ptr<void>(sonarK[i]));
	 } break;
	 case TID_WATER_POOL: {
		 genericVec.reserve(waterPl.size());
		 for (int i{ 0 }; i < waterPl.size(); i++) genericVec.push_back(std::shared_ptr<void>(waterPl[i]));
	 } break;
	 case GOLD_DYNAMIC: {
		 genericVec.reserve(dymcGold.size());
		 for (int i{ 0 }; i < dymcGold.size(); i++) genericVec.push_back(std::shared_ptr<void>(dymcGold[i]));
	 } break;
	 case TID_PROTESTER: // refere to hard_core_protestor
	 case TID_HARD_CORE_PROTESTER: {
		 genericVec.reserve(NPC.size());
		 for (int i{ 0 }; i < NPC.size(); i++) genericVec.push_back(std::shared_ptr<void>(NPC[i]));
	 }break;
	 }
 }
 int&& StudentWorld::XYtoIndex(const int& x, const int& y) {
	 int index = x + y * (EARTH_Y + 1);
	 if (index >= emptyField.size() || index < 0) {
		 return -1;
	 }
	 else return std::move(index);
 } // function convert x,y to index for the bitset 
 void StudentWorld::IndexToXY(const int& index, int& x, int& y) {
	 y = std::round(index / (EARTH_Y + 1)); // round down to prevent going to the next y
	 x = index - y * (EARTH_X - SPRITE_XY + 1);
 }


 /* ---- NPC functions ---*/
 void StudentWorld::spawnProtester(bool forceSpawn) {
	 if (forceSpawn) {
	 spawnNPC: thisTickSpwnProtester = CURRENT_TICK;
		 int randNum = rand() % spwnChance_hardcore; // chances of 1 number meet the below if statement from 0 to spwnChance_hardcore - 1
		 if (!randNum) { // is 0
			 NPC.push_back(std::make_shared<hardcoreProtester>(CURRENT_TICK));
		 }
		 else {
			 NPC.push_back(std::make_shared<regularProtester>(CURRENT_TICK));
		 }
	 }
	 else {
		 if (NPC.size() < protester_amt && CURRENT_TICK - thisTickSpwnProtester >= REQ_TICK_SPWN_protester) goto spawnNPC; // if num protesters doesn't meet the threshold and meet the required tick
	 }
 }
 void StudentWorld::BFS_findPath(const int& npcX, const int& npcY, std::vector<traversedLog>& scoutLog) { // contain index to navigate the emptyField
	 int xScout{ 60 }, yScout{ 60 };
	 int distDir[4]{ 100, 100, 100, 100 };
	 while (true) {
		 scoutLog.push_back(traversedLog{ xScout, yScout });
		 if (xScout == npcX && yScout == npcY) return;
		 for (int i{ 0 }; i < 4; i++) {
			 int index{ -1 };
			 switch (i) {
			 case 0: index = XYtoIndex(xScout, yScout + 1); break; // up
			 case 1: index = XYtoIndex(xScout, yScout - 1); break; // down
			 case 2: index = XYtoIndex(xScout - 1, yScout);  break; // left
			 case 3: index = XYtoIndex(xScout + 1, yScout);  break; // right
			 }
			 if (0 <= index && index < emptyField.size() && !beenHere[index]) distDir[i] = std::sqrt(std::pow(xScout - npcX, 2) + std::pow(yScout - npcY, 2));
		 }
		 int smallestDist = distDir[0];
		 int smallestIndex = 0;
		 for (int i{ 1 }; i < 4; i++) {
			 if (smallestDist > distDir[i]) {
				 smallestDist = distDir[i];
				 smallestIndex = i;
			 }
		 }
		 switch (smallestIndex) {
		 case 0: scoutLog.push_back(traversedLog{ xScout, ++yScout }); break;
		 case 1: scoutLog.push_back(traversedLog{ xScout, --yScout }); break;
		 case 2: scoutLog.push_back(traversedLog{ --xScout, yScout }); break;
		 case 3: scoutLog.push_back(traversedLog{ ++xScout, yScout }); break;
		 }
		 beenHere[XYtoIndex(xScout, yScout)] = true;
	 }
 }
 void StudentWorld::NPCgenerateNewPath(unsigned int& newDist, unsigned int& newDir, const int& npcX, const int& npcY, const bool& isLeaving) {
	 // IF THE GAME FREEZE, IT MEANS THAT THE PROTESTER IS TRYING TO MOVE OUT OF BOUND
	 if (isLeaving) { // generate path to get out of the oil field, breadth first search
		 /*std::vector<traversedLog> scoutLog;
		 BFS_findPath(npcX, npcY, scoutLog);*/
	 }
	 else { // generate path to move normally in the oil field
		 bool arrDir[4]{ false, false, false, false };
		 int minDist{ 8 }, maxDist{ 60 };
		 int optMinDist{ 1 }, optMaxDist{ 7 };
		 do {
			 newDist = (rand() % (maxDist - minDist + 1)) + minDist; // maxDist - minDist + 1 for inclusive  0 to maxDist - minDist 
			 for (int i = 0; i < 4; i++) {  // 0: up, 1: down, 2: left, 3: right
				 int distanceWithNoEarth{ 0 };
				 switch (i) {
				 case 0: {
					 if (npcY + newDist <= EARTH_Y && emptyField[XYtoIndex(npcX, npcY + 1)]) {
						 for (int j{ npcY + 1 }; j <= npcY + newDist; j++) {
							 if (emptyField[XYtoIndex(npcX, j)]) distanceWithNoEarth++;
							 else break;
						 }
					 }
				 } break;
				 case 1: {
					 if (npcY - newDist >= 0 && emptyField[XYtoIndex(npcX, npcY - 1)]) {
						 for (int j{ npcY - 1 }; j >= npcY - newDist; j--) {
							 if (emptyField[XYtoIndex(npcX, j)]) distanceWithNoEarth++;
							 else break;
						 }
					 }
				 } break;
				 case 2: {
					 if (npcX - newDist >= 0 && emptyField[XYtoIndex(npcX - 1, npcY)]) {
						 for (int j{ npcX - 1 }; j >= npcX - newDist; j--) {
							 if (emptyField[XYtoIndex(j, npcY)]) distanceWithNoEarth++;
							 else break;
						 }
					 }
				 } break;
				 case 3: {
					 if (npcX + newDist <= EARTH_X - SPRITE_XY && emptyField[XYtoIndex(npcX + 1, npcY)]) {
						 for (int j{ npcX + 1 }; j <= npcX + newDist; j++) {
							 if (emptyField[XYtoIndex(j, npcY)]) distanceWithNoEarth++;
							 else break;
						 }
					 }
				 } break;
				 }
				 if (optMinDist <= distanceWithNoEarth && distanceWithNoEarth <= newDist) {
					 if (distanceWithNoEarth < newDist) {
						 newDist = distanceWithNoEarth;
					 }
					 arrDir[i] = true;
				 } 
			 }
		 } while (arrDir[0] == false && arrDir[1] == false && arrDir[2] == false && arrDir[3] == false); // generate new dist. if no path works
		 int randDir = rand() % 4;
		 bool dirNotFound = true;
		 do {
			 if (arrDir[randDir]) {
				 newDir = randDir;
				 dirNotFound = false;
			 }
			 else randDir = (randDir + 1) % 4;
		 } while (dirNotFound);
	 }
 }
 void StudentWorld::checkPlayerDetection(const int& playerX, const int& playerY, const int& npcX, const int& npcY, bool& canEngagePlayer, GraphObject::Direction& whatDir) {
	 // check if protester contacts with player in a certain dir
	 double distNPC2Player = std::sqrt(std::pow(npcX - playerX, 2) + std::pow(npcY - playerY, 2));
	 if (distNPC2Player == 4) {
		 if (playerY > npcY) whatDir = GraphObject::Direction::up;
		 else if (playerY < npcY) whatDir = GraphObject::Direction::down;
		 else if (playerX < npcX) whatDir = GraphObject::Direction::left;
		 else if (playerX > npcX) whatDir = GraphObject::Direction::right;
		 canEngagePlayer = true;
	 }
	 else if (distNPC2Player > 4) {
		 // scan if path to player is traverable, if true then switch direction and go that way
		 // the environment probing is with respect to the player rather than the npc
		 if (playerY > npcY && playerX == npcX) { // if the player is above
			 for (int k{ playerY - 1 }; k >= npcY + SPRITE_XY; k--) {
				 if (!emptyField[XYtoIndex(playerX, k)]) return; // path blocked
			 }
			 whatDir = GraphObject::Direction::up;
			 canEngagePlayer = true;
		 }
		 else if (playerY < npcY && playerX == npcX) {// if the player is below
			 for (int k{ playerY + SPRITE_XY }; k <= npcY - 1; k++) {
				 if (!emptyField[XYtoIndex(playerX, k)]) return; // path blocked
			 }
			 whatDir = GraphObject::Direction::down;
			 canEngagePlayer = true;
		 }
		 else if (playerX < npcX && playerY == npcY) {// if the player at the left
			 for (int k{ playerX + SPRITE_XY }; k <= npcX - 1; k++) {
				 if (!emptyField[XYtoIndex(k, playerY)]) return; // path blocked
			 }
			 whatDir = GraphObject::Direction::left;
			 canEngagePlayer = true;
		 }
		 else if (playerX > npcX && playerY == npcY) {// if the player is at the right
			 for (int k{ playerX - 1 }; k >= npcX + SPRITE_XY; k--) {
				 if (!emptyField[XYtoIndex(k, playerY)]) return; // path blocked
			 }
			 whatDir = GraphObject::Direction::right;
			 canEngagePlayer = true;
		 }
	 }
 }


 // move()
 int StudentWorld::checkSpecialKey() {
	 switch (KEY_HIT) {
	 case KEY_PRESS_ESCAPE: {
		 decLives();  // lose 1 lifes
		 return GWSTATUS_PLAYER_DIED;
	 } break;
	 case KEY_PRESS_SPACE: // squirt
		 if (mainChar->getInfos(TID_WATER_SPURT) != 0) { // not 0
			 modify_info(TID_PLAYER, TID_WATER_SPURT);
			 int playerX{ mainChar->getX() }, playerY{ mainChar->getY() };
			 GraphObject::Direction playerDir{ mainChar->getDirection() };
			 bool canSquirt{ true };
			 if (playerX < SPRITE_XY && playerDir == GraphObject::Direction::left
				 || playerY < SPRITE_XY && playerDir == GraphObject::Direction::down
				 || playerY == EARTH_Y && playerDir == GraphObject::Direction::up
				 || playerX >= EARTH_X - SPRITE_XY && playerDir == GraphObject::Direction::right) {
				 canSquirt = false; // condition to make sure squirt object not going out of bound
			 }
			 if (canSquirt) {
				 int maxTravelDist{ SPRITE_XY };
				 if (playerDir == GraphObject::Direction::up || playerDir == GraphObject::Direction::down) { // up down
					 waterSpurt.push_back(std::make_shared<waterSquirt>(playerX, playerY + (playerDir == GraphObject::Direction::up ? 1 : -1) * SPRITE_XY, maxTravelDist, playerDir));
				 } // left right
				 else waterSpurt.push_back(std::make_shared<waterSquirt>(playerX + (playerDir == GraphObject::Direction::right ? 1 : -1) * SPRITE_XY, playerY, maxTravelDist, playerDir));
			 }
			 playSound(SOUND_PLAYER_SQUIRT);
		 }  break;
	 case KEY_PRESS_TAB: // drop gold
		 if (mainChar->getGold()) { // not 0
			 mainChar->modify_infos(TID_GOLD, -1);
			 dymcGold.push_back(std::make_shared<dynamicGold>(mainChar->getX(), mainChar->getY(), CURRENT_TICK, tick_goodieAvail));
		 }
		 break;
	 }
	 if (KEY_HIT == 'Z' || KEY_HIT == 'z') {
		 if (mainChar->getInfos(TID_SONAR)) { // not 0
			 const int modifiedRadius{ 12 };
			 mainChar->modify_infos(TID_SONAR, -1);
			 static_consumable::modifyRadVisibile(modifiedRadius);
		 }
	 }
	 return -1;
 }
 void StudentWorld::updateEntityState(const bool&& updateWithoutKeyHit) {
	 for (int i{ 0 }; i < vID_cntr.size(); i++) {
		 if (updateWithoutKeyHit) { // These Entities updated regardless if of key hit (dynamic)
			 switch (vID_cntr[i]) {
			 case TID_BOULDER:	// refer to TID_WATER_POOL
			 case TID_SONAR:  // refer to TID_WATER_POOL
			 case GOLD_DYNAMIC:
			 case TID_WATER_POOL: {
				 idToVec(vID_cntr[i]);
				 std::shared_ptr<object> tempPtr;
				 for (int j{ 0 }; j < genericVec.size(); j++) {
					 tempPtr = std::static_pointer_cast<object>(genericVec[j]);
					 if (tempPtr && tempPtr->getActiveState()) { // not null and active
						 tempPtr->doSomething(CURRENT_TICK, (vID_cntr[i] == TID_BOULDER ? EARTH_Y : 0));
						 if (vID_cntr[i] == TID_BOULDER && !(std::static_pointer_cast<Boulder>(tempPtr)->getStability()) && std::static_pointer_cast<Boulder>(tempPtr)->meetRequiredDelay()) { // is boulder, not stable and meet required delay
							 playSound(SOUND_FALLING_ROCK);
							 emptyField[XYtoIndex(tempPtr->getX(), tempPtr->getY())] = true;
						 }
					 }
				 }
			 } break;
			 case TID_WATER_SPURT: {
				 for (int j{ 0 }; j < waterSpurt.size(); j++) {
					 if (waterSpurt[j] && waterSpurt[j]->getActiveState()) { // is not null and is active
						 waterSpurt[j]->doSomething(EARTH_X, EARTH_Y);
					 }
				 }
			 }break;
			 case TID_PROTESTER: // refer to hard core
			 case TID_HARD_CORE_PROTESTER: {
				 for (int j{ 0 }; j < NPC.size(); j++) {
					 if (NPC[j]->reachedDest()) {
						 unsigned int newDist{ 0 }, newDir{ 0 };
						 NPCgenerateNewPath(newDist, newDir, NPC[j]->Entity::getX(), NPC[j]->Entity::getY(), NPC[j]->getLeavingStatus());
						 NPC[j]->setNewDistAndDir(newDist, newDir);
						 NPC[j]->resetReachedDest();
					 }
					 if (CURRENT_TICK - NPC[j]->getMovementDelayTick() == REQ_TICK_REST_protester + 1) { // delay for 1 more tick
						 NPC[j]->setMovementDelayTick(CURRENT_TICK);
						 NPC[j]->increaseNonRestTick();
						 bool canEngagePlayer{ false };
						 GraphObject::Direction whatDir{ GraphObject::Direction::none }; // 0: up, 1: down, 2: left, 3: right
						 if (!NPC[j]->getLeavingStatus()) { // if the protester is not leaving, check if the protester encounters player
							 int playerX = mainChar->getX(), playerY = mainChar->getY();
							 int npcX = NPC[j]->Entity::getX(), npcY = NPC[j]->Entity::getY();
							 checkPlayerDetection(playerX, playerY, npcX, npcY, canEngagePlayer, whatDir);
							 NPC[j]->setEncounterStatus(std::move(canEngagePlayer), whatDir);
							 if (NPC[j]->getEncounterStatus()) { // see if the protester is engagable with player
								 NPC[j]->doSomething(mainChar->getX(), mainChar->getY()); // move toward with player
							 }
							 else { // move or leave
								 NPC[j]->doSomething(EARTH_X, EARTH_Y, CURRENT_TICK);
							 }
						 }
					 }
				 }
			 }break;
			 default: break;
			 }
		 }
		 else { // These Entities need key hit in order to update
			 switch (vID_cntr[i]) {
			 case TID_PLAYER: {
				 mainChar->doSomething(KEY_HIT, EARTH_X, EARTH_Y);
				 if (mainChar->dirLocked()) mainChar->freeDirLock();
			 } break;
			 case TID_BARREL: // refer to TID_GOLD
			 case TID_GOLD: {
				 idToVec(vID_cntr[i]);
				 std::shared_ptr<object> tempPtr;
				 for (int j{ 0 }; j < genericVec.size(); j++) {
					 tempPtr = std::static_pointer_cast<object>(genericVec[j]);
					 if (genericVec[j].get() && tempPtr->getActiveState()) { // not null and active
						 double distObject2Player = std::sqrt(std::pow(mainChar->getX() - tempPtr->getX(), 2) + std::pow(mainChar->getY() - tempPtr->getY(), 2));
						 tempPtr->doSomething(mainChar->getX(), mainChar->getY()); // visiblity
					 }
				 }
			 }break;
			 case TID_EARTH: break;
			 default: break;
			 }
		 }
	 }
	 genericVec.clear();
 }
 void StudentWorld::GenerateRandDynamicObj() {
	 int genNum = rand() % spwnChance_goodie; // 1 chance out of genNum
	 if (!genNum) {  // equal to 0
		 int genNumSonarOrWater = (rand() % ratioSonarOrWater) + 1;
		 int randIndex{ 0 }, randY{ 0 }, randX{ 0 };
		 do {
			 randIndex = rand() % (emptyField.size());
		 } while (!emptyField[randIndex]);
		 IndexToXY(randIndex, randX, randY);
		 if (genNumSonarOrWater == 1) {
			 sonarK.push_back(std::make_shared<sonarKit>(0, 60, CURRENT_TICK, tick_goodieAvail));
		 }
		 else {
			 waterPl.push_back(std::make_shared<waterPool>(randX, randY, CURRENT_TICK, tick_goodieAvail));
		 }
	 }
 }
 void StudentWorld::updateEmptySpace() {// make condition for vertical column
	 int playerX{ mainChar->getX() }, playerY{ mainChar->getY() };
	 if (!emptyField[XYtoIndex(playerX, playerY)]) { // if player just digged a new position
		 emptyField[XYtoIndex(playerX, playerY)] = true;
	 }
 }
 bool  StudentWorld::checkCollision(const int& ID_one, const int& ID_two) {
	 bool areCollided{ false };
	 switch (ID_one) { // entity that causes collision
	 case TID_PLAYER:
		 switch (ID_two) { // entity that is impacted by collision
		 case TID_BOULDER: {
			 int mainX{ mainChar->getX() }, mainY{ mainChar->getY() };
			 for (int i{ 0 }; i < boulder.size(); i++) {
				 int boulderX{ boulder[i]->getX() }, boulderY{ boulder[i]->getY() };
				 if (boulder[i]->getActiveState()) { // -1 preserve the current lock status of the direction
					 if (0 <= std::abs(mainX - boulderX) && std::abs(mainX - boulderX) < SPRITE_XY && boulderY - mainY == SPRITE_XY) {
						 mainChar->lockDir(1, -1, -1, -1); // prevent moving up if player is under boulder
					 }
					 else if (0 <= std::abs(mainX - boulderX) && std::abs(mainX - boulderX) < SPRITE_XY && mainY - boulderY == SPRITE_XY) {
						 mainChar->lockDir(-1, 1, -1, -1); // prevent moving down if player is above boulder
					 }
					 else if (0 <= std::abs(mainY - boulderY) && std::abs(mainY - boulderY) < SPRITE_XY && mainX - boulderX == SPRITE_XY) {
						 mainChar->lockDir(-1, -1, 1, -1); // prevent moving left if player is at the right of boulder
					 }
					 else if (0 <= std::abs(mainY - boulderY) && std::abs(mainY - boulderY) < SPRITE_XY && boulderX - mainX == SPRITE_XY) {
						 mainChar->lockDir(-1, -1, -1, 1); // prevent moving right if player is at the left of boulder
					 }
				 }
			 }
			 areCollided = true;
		 } break;
		 case TID_EARTH: {
			 int mainX{ mainChar->getX() }, mainY{ mainChar->getY() };
			 if (!mainChar->rotated()) {
				 switch (mainChar->getDirection()) {
				 case GraphObject::Direction::right: // refer to left
				 case GraphObject::Direction::left: {
					 bool dirIsRight = mainChar->getDirection() == GraphObject::Direction::right;
					 if ((dirIsRight ? 1 : 0) <= mainX && mainX <= EARTH_X - SPRITE_XY && mainY < EARTH_Y) {
						 int sprite_SideClipRange = (mainY + SPRITE_XY - 1 >= EARTH_Y ? EARTH_Y - mainY : SPRITE_XY);
						 for (int i{ 0 }; i < sprite_SideClipRange; i++) {
							 if (oilField[mainY + i][mainX + (dirIsRight ? SPRITE_XY - 1 : 0)]->getActiveState()) {
								 oilField[mainY + i][mainX + (dirIsRight ? SPRITE_XY - 1 : 0)]->disable();
								 areCollided = true;
							 }
						 }
					 }
				 } break;
				 case GraphObject::Direction::up: // refer to down
				 case GraphObject::Direction::down: {
					 bool dirIsUp = mainChar->getDirection() == GraphObject::Direction::up;
					 if ((dirIsUp ? 1 : 0) <= mainY && mainY < (dirIsUp ? EARTH_Y - SPRITE_XY + 1 : EARTH_Y)) {
						 for (int i{ 0 }; i < SPRITE_XY; i++) {
							 if (oilField[mainY + (dirIsUp ? SPRITE_XY - 1 : 0)][mainX + i]->getActiveState()) {
								 oilField[mainY + (dirIsUp ? SPRITE_XY - 1 : 0)][mainX + i]->disable();
								 areCollided = true;
							 }
						 }
					 }
				 } break;
				 }
			 }
			 mainChar->reset_isRotated();
		 } break;
		 case TID_BARREL: // refer to TID_WATER_POOL
		 case TID_GOLD:   // refer to TID_WATER_POOL
		 case TID_SONAR:  // refer to TID_WATER_POOL
		 case TID_PROTESTER: // refer to TID_WATER_POOL
		 case TID_HARD_CORE_PROTESTER:  // refer to TID_WATER_POOL
		 case TID_WATER_POOL: {
			 idToVec(ID_two);
			 double dist_obj2tnlMan;
			 bool isNotProtester = ID_two != TID_HARD_CORE_PROTESTER && ID_two != TID_PROTESTER;
			 std::shared_ptr<object> tempPtr;
			 for (int i{ 0 }; i < genericVec.size(); i++) {
				 tempPtr = std::static_pointer_cast<object>(genericVec[i]);
				 if (tempPtr && tempPtr->getActiveState()) { // not null and is active
					 dist_obj2tnlMan = std::sqrt(std::pow(tempPtr->getX() - mainChar->getX(), 2) + std::pow(tempPtr->getY() - mainChar->getY(), 2));
					 if (tempPtr->getActiveState() && dist_obj2tnlMan <= (isNotProtester ? REQ_CONSUME : REQ_COLLIDE_PROTESTER)) {
						 if (isNotProtester) {
							 tempPtr->disable();
							 areCollided = true; //playsound and modify stats
						 }
						 else { // either normal or hardcore protester
							 if (NPC[i]->getNonRestTick() >= 15) {
								 NPC[i]->resetNonRestTick();
								 playSound(SOUND_PROTESTER_YELL);
								 areCollided = true; // decrease player hit point
							 }
							 NPC[i]->setShoutingStatus(true); // prevent npc from moving when collided with player
						 }
					 }
					 else {
						 if (!isNotProtester) NPC[i]->setShoutingStatus(false);
					 }
				 }
			 }
		 } break;
		 } break;
	 case TID_BOULDER:
		 switch (ID_two) {
		 case TID_PROTESTER: // refere to hardcore
		 case TID_HARD_CORE_PROTESTER: {
			 for (int i{ 0 }; i < NPC.size(); i++) {
				 if (NPC[i] && NPC[i]->Entity::getActiveState()) {
					 for (int k{ 0 }; k < boulder.size(); k++) {
						 if (boulder[k] && boulder[k]->getActiveState()) {
							 int distBoulder2Protester = std::sqrt(std::pow(boulder[k]->getX() - NPC[i]->Entity::getX(), 2) + std::pow(boulder[k]->getY() - NPC[i]->Entity::getY(), 2));
							 if (!(static_cast<Boulder*>(boulder[k].get()))->getStability() && distBoulder2Protester == REQ_BOULDER_HIT) {// boulder must be in active state, is not stable and meet the hit distance required
								 boulder[k]->disable();
								 NPC[i]->modifiyHitPoints(-100);
								 NPC[i]->enableLeaving();
								 increaseScore(BOULDER_PTS);
								 areCollided = true;
							 }
						 }
					 }
				 }
			 }
		 }break;
		 case TID_PLAYER: {
			 for (int i{ 0 }; i < boulder.size(); i++) {
				 if (boulder[i]) { // must not be nullptr
					 int playerX{ mainChar->getX() }, playerY{ mainChar->getY() };
					 int boulderX{ boulder[i]->getX() }, boulderY{ boulder[i]->getY() };
					 int distBoulder2Player = std::sqrt(std::pow(playerX - boulderX, 2) + std::pow(playerY - boulderY, 2));
					 if (boulder[i]->getActiveState() && !(static_cast<Boulder*>(boulder[i].get()))->getStability() && distBoulder2Player == REQ_BOULDER_HIT) {// boulder must be in active state, is not stable and meet the hit distance required
						 boulder[i]->disable();
						 areCollided = true;
					 }
				 }
			 }
			 break;
		 case TID_EARTH: { // boulder is in instable state and droped on earth object
			 for (int i{ 0 }; i < boulder.size(); i++) {
				 if (boulder[i]) { // must not be nullptr
					 if (boulder[i]->getActiveState()) {
						 int earthSupport_cntr{ 0 };
						 bool boulderIsStable = static_cast<Boulder*>(boulder[i].get())->getStability();
						 for (int k{ 0 }; k < SPRITE_XY; k++) {
							 if (oilField[boulder[i]->getY() - boulderIsStable][boulder[i]->getX() + k]->getActiveState()) {
								 earthSupport_cntr++;
							 }
						 }
						 if (!earthSupport_cntr && boulderIsStable) { // if there is no earth underneath
							 static_cast<Boulder*>(boulder[i].get())->setStability(false);
							 static_cast<Boulder*>(boulder[i].get())->setInitUnstableTick(CURRENT_TICK);
						 }
						 if (earthSupport_cntr && !boulderIsStable) {// not stable and earth is underneath
							 boulder[i]->disable();
						 }
					 }
				 }
			 }
		 }break;
		 }break;
		 default: break;
		 } break;
	 case GOLD_DYNAMIC:
		 switch (ID_two) {
		 case TID_PROTESTER: // refer to hard core
		 case TID_HARD_CORE_PROTESTER: {
			 idToVec(ID_two);
			 double dist_protester2tnlMan;
			 std::shared_ptr<regularProtester> tempPtr;
			 for (int i{ 0 }; i < genericVec.size(); i++) {
				 tempPtr = std::static_pointer_cast<regularProtester>(genericVec[i]);
				 if (tempPtr && tempPtr->Entity::getActiveState()) { // not null and is active
					 for (int k{ 0 }; k < dymcGold.size(); k++) {
						 if (dymcGold[k] && dymcGold[k]->getActiveState()) { // not null and is active
							 dist_protester2tnlMan = std::sqrt(std::pow(tempPtr->Entity::getX() - dymcGold[k]->getX(), 2) + std::pow(tempPtr->Entity::getY() - dymcGold[k]->getY(), 2));
							 if (tempPtr->Entity::getActiveState() && dist_protester2tnlMan <= REQ_CONSUME) {
								 dymcGold[k]->disable();
								 NPC[i]->modifiyHitPoints(-2);
								 if (!NPC[i]->getHitPoints()) {
									 increaseScore(ID_two == TID_PROTESTER ? BRIB_NORM_PTS : BRIB_HARD_PTS);
									 NPC[i]->enableLeaving(); // NOTE: change this to stun for normal protester
								 }
								 areCollided = true;
							 }
						 }
					 }
				 }
			 }
		 } break;
		 } break;
	 case TID_WATER_SPURT:
		 switch (ID_two) {
		 case TID_BOULDER: // refer to hardcore
		 case TID_PROTESTER:  // refer to hardcore
		 case TID_HARD_CORE_PROTESTER: {
			 idToVec(ID_two);
			 std::shared_ptr<object> tempPtr;
			 bool isProtester = ID_two == TID_PROTESTER || ID_two == TID_HARD_CORE_PROTESTER;
			 double distWater2Entity;
			 for (int i{ 0 }; i < genericVec.size(); i++) {
				 tempPtr = std::static_pointer_cast<object>(genericVec[i]);
				 if (tempPtr && tempPtr->getActiveState()) { // not null and is active
					 for (int k{ 0 }; k < waterSpurt.size(); k++) {
						 if (waterSpurt[k] && waterSpurt[k]->getActiveState()) {
							 distWater2Entity = std::sqrt(std::pow(tempPtr->getX() - waterSpurt[k]->getX(), 2) + std::pow(tempPtr->getY() - waterSpurt[k]->getY(), 2));
							 if (distWater2Entity <= (isProtester ? REQ_COLLIDE_PROTESTER : REQ_CONSUME)) {
								 waterSpurt[k]->disable();
								 areCollided = true;
								 if (isProtester) {
									 NPC[i]->modifiyHitPoints(-2);
									 /*if (ID_two == TID_HARD_CORE_PROTESTER){
									 }*/
									 if (!NPC[i]->getHitPoints()) {
										 increaseScore(ID_two == TID_PROTESTER ? IRTED_NORM_PTS : IRTED_HARD_PTS);
										 NPC[i]->enableLeaving();
									 }
								 }
							 }
						 }
					 }
				 }
			 }
		 }break;
		 case TID_EARTH: {
			 for (int i{ 0 }; i < waterSpurt.size(); i++) {
				 if (waterSpurt[i] && waterSpurt[i]->getActiveState()) { // water squirt is not null and is active
					 bool wContacted{ false };
					 int waterX{ waterSpurt[i]->getX() }, waterY{ waterSpurt[i]->getY() };
					 if (0 <= waterY && waterY <= EARTH_Y && 0 <= waterX && waterX <= EARTH_X - SPRITE_XY) {
						 for (int j{ 0 }; j < (EARTH_Y - waterY < SPRITE_XY ? EARTH_Y - waterY : SPRITE_XY); j++) {
							 for (int k{ 0 }; k < SPRITE_XY; k++) {
								 if (oilField[waterY + j][waterX + k]->getActiveState()) {
									 wContacted = true; break;
								 }
							 }
							 if (wContacted) break;
						 }
					 }
					 else wContacted = true;
					 if (wContacted) {
						 waterSpurt[i]->disable();
						 areCollided = true;
					 }
				 }
			 }
		 } break;
		 } break;
	 default: break;
	 }
	 genericVec.clear();
	 return areCollided;
 }
 const int StudentWorld::interactSound(const int& targetID_one, const int& targetID_two) { // this is a general function, use it manually if any specific instance collided with certain requirement met
	 switch (targetID_one) { // entity that causes collision
	 case TID_PLAYER:
		 switch (targetID_two) { // entity that impacted by collision
		 case TID_WATER_SPURT: return SOUND_PLAYER_SQUIRT;
		 case TID_BOULDER: return SOUND_NONE;
		 case TID_BARREL: return SOUND_FOUND_OIL;
		 case TID_EARTH: return SOUND_DIG;
		 case TID_GOLD: return SOUND_GOT_GOODIE;
		 case TID_SONAR: return SOUND_SONAR;
		 case TID_WATER_POOL: return SOUND_GOT_GOODIE;
		 default: return SOUND_NONE;
		 } break;
	 case TID_WATER_SPURT:
		 switch (targetID_two) {
		 case TID_PROTESTER: return SOUND_PROTESTER_ANNOYED;
		 case TID_HARD_CORE_PROTESTER: return SOUND_PROTESTER_ANNOYED;
		 default: return SOUND_NONE;
		 }break;
	 case TID_BOULDER:
		 switch (targetID_two) {
		 case TID_PLAYER: return SOUND_PLAYER_GIVE_UP;
		 case TID_PROTESTER:// refer to hard core
		 case TID_HARD_CORE_PROTESTER: return SOUND_PROTESTER_GIVE_UP;
		 case FREE_FALL: return SOUND_FALLING_ROCK;
		 default: return SOUND_NONE;
		 }break;
	 case GOLD_DYNAMIC:
		 switch (targetID_two) {
		 case TID_PROTESTER: // refer to hard core
		 case TID_HARD_CORE_PROTESTER: return SOUND_PROTESTER_FOUND_GOLD;
		 default: return SOUND_NONE;
		 }break;
	 default: return SOUND_NONE;
	 }
 }
 void StudentWorld::modify_info(const int& targetID_one, const int& targetID_two, int manual_var) { // manual var can be used as modification value or specific instance index
	 switch (targetID_one) {
	 case TID_PLAYER:
		 switch (targetID_two) {
		 case TID_WATER_SPURT: mainChar->modify_infos(TID_WATER_SPURT, -1); break;
		 case TID_BOULDER: break;
		 case TID_BARREL: mainChar->modify_infos(TID_BARREL, 1); increaseScore(OIL_PTS);  break;
		 case TID_EARTH: break;
		 case TID_GOLD:  mainChar->modify_infos(TID_GOLD, (!manual_var ? 1 : manual_var)); increaseScore(GOLD_PTS); break;
		 case TID_SONAR: mainChar->modify_infos(TID_SONAR, (!manual_var ? 1 : manual_var)); increaseScore(SONAR_PTS); break;
		 case TID_WATER_POOL: mainChar->modify_infos(TID_WATER_SPURT, 5); increaseScore(WTR_PTS); break;
		 case TID_PROTESTER: // refer to hard core 	
		 case TID_HARD_CORE_PROTESTER: mainChar->modify_infos(TID_PLAYER, -20); break;
		 } break;
	 case TID_BOULDER:
		 switch (targetID_two) {
		 case TID_PLAYER: mainChar->modifiyHitPoints(-10); break;
		 default: break;
		 }
		 break;
	 default: break;
	 }
 }


 // clean up at the end of move()
 void StudentWorld::cleanUpObjectAndMisc() {
	 for (int i{ 0 }; i < vID_cntr.size(); i++) {
		 idToVec(vID_cntr[i]); // Earth objects are deleted when reseting level or new round
		 for (auto iterGvec{ genericVec.begin() }; iterGvec != genericVec.end(); iterGvec++) {
			 std::shared_ptr<Entity> tempPtr = std::static_pointer_cast<Entity>(*iterGvec);
			 if (!tempPtr->getActiveState()) { // true if entity is not active
				 int iterated_num{ iterGvec - genericVec.begin() }; // get the position wrt to the original index 0
				 switch (vID_cntr[i]) {
				 case TID_EARTH:
				 case TID_PLAYER: break;
				 case TID_PROTESTER:
				 case TID_HARD_CORE_PROTESTER: {
					 NPC.erase(std::vector<std::shared_ptr<regularProtester>>::iterator(NPC.begin() + (iterated_num)));
					 spawnProtester(true);
				 }  break;
				 case TID_WATER_SPURT: {
					 waterSpurt.erase(std::vector<std::shared_ptr<object>>::iterator(waterSpurt.begin() + (iterated_num)));
				 }  break;
				 case TID_WATER_POOL: {
					 waterPl.erase(std::vector<std::shared_ptr<object>>::iterator(waterPl.begin() + (iterated_num)));
				 }  break;
				 case TID_SONAR: {
					 sonarK.erase(std::vector<std::shared_ptr<object>>::iterator(sonarK.begin() + (iterated_num)));
				 }  break;
				 case GOLD_DYNAMIC: {
					 dymcGold.erase(std::vector<std::shared_ptr<object>>::iterator(dymcGold.begin() + (iterated_num)));
				 }  break;
				 case TID_BOULDER: {
					 boulder.erase(std::vector<std::shared_ptr<object>>::iterator(boulder.begin() + (iterated_num)));
				 }  break;
				 case TID_BARREL: {
					 barrelOil.erase(std::vector<std::shared_ptr<object>>::iterator(barrelOil.begin() + (iterated_num)));
				 }  break;
				 case TID_GOLD: {
					 goldNgt.erase(std::vector<std::shared_ptr<object>>::iterator(goldNgt.begin() + (iterated_num)));
				 }  break;
				 }
				 genericVec.clear();
				 idToVec(vID_cntr[i]); // reset genericVec due to the original object vector has gone to a new location
				 if (iterated_num == genericVec.size()) break;
				 else iterGvec = genericVec.begin() + iterated_num; // reset iter location
			 }
		 }
		 genericVec.clear();
	 }
	 if (static_consumable::distVisibleIsModified()) { // if REQ_DIST visiblity is modified by sonar
		 static_consumable::modifyRadVisibile(REQ_VISIBILE);
	 }
 }

