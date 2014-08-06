/* Copyright (C) 2013-2014 Michal Brzozowski (rusolis@poczta.fm)

   This file is part of KeeperRL.

   KeeperRL is free software; you can redistribute it and/or modify it under the terms of the
   GNU General Public License as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   KeeperRL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
   even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with this program.
   If not, see http://www.gnu.org/licenses/ . */

#ifndef _PLAYER_CONTROL_H
#define _PLAYER_CONTROL_H

#include "map_memory.h"
#include "creature_view.h"
#include "task.h"
#include "entity_set.h"
#include "sectors.h"
#include "event.h"
#include "spell_info.h"
#include "view.h"
#include "collective_control.h"
#include "collective.h"

class Model;
class Technology;
class View;

class PlayerControl : public CreatureView, public CollectiveControl, public EventListener {
  public:
  PlayerControl(Collective*, Model*, Level*);
  virtual const MapMemory& getMemory() const override;
  MapMemory& getMemory(Level* l);
  virtual ViewIndex getViewIndex(Vec2 pos) const override;
  virtual void refreshGameInfo(GameInfo&) const  override;
  virtual Vec2 getPosition() const  override;
  virtual bool canSee(const Creature*) const  override;
  virtual bool canSee(Vec2 position) const  override;
  virtual vector<const Creature*> getUnknownAttacker() const override;
  virtual const Tribe* getTribe() const override;
  Tribe* getTribe();
  virtual bool isEnemy(const Creature*) const override;

  virtual bool staticPosition() const override;
  virtual int getMaxSightRange() const override;

  virtual void onSquareReplacedEvent(const Level*, Vec2 pos) override;
  virtual void onChangeLevelEvent(const Creature*, const Level* from, Vec2 pos, const Level* to, Vec2 toPos) override;
  virtual void onTechBookEvent(Technology*) override;
  virtual void onWorshipEvent(Creature* who, const Deity* to, WorshipType) override;

  void onConqueredLand(const string& name);
  virtual void onCreatureKilled(const Creature* victim, const Creature* killer) override;

  virtual void onConstructedSquare(Vec2 pos, SquareType) override;

  void processInput(View* view, UserInput);
  void tick(double);
  MoveInfo getMove(Creature* c);

  virtual const Level* getViewLevel() const;

  bool isRetired() const;
  const Creature* getKeeper() const;
  Creature* getKeeper();
  virtual double getWarLevel() const override;
  void addImp(Creature*);
  void addKeeper(Creature*);

  void render(View*);

  bool isTurnBased();
  void retire();

  struct RoomInfo {
    string name;
    string description;
    Optional<TechId> techId;
  };
  static vector<RoomInfo> getRoomInfo();
  static vector<RoomInfo> getWorkshopInfo();
  static vector<CreatureId> getSpawnInfo(const Technology*);

  struct SpawnInfo {
    CreatureId id;
    int manaCost;
    Optional<TechId> techId;
    EnumSet<MinionTrait> traits;
  };

  enum class MinionOption;

  SERIALIZATION_DECL(PlayerControl);

  template <class Archive>
  static void registerTypes(Archive& ar);

  private:
  friend class KeeperControlOverride;

  void considerDeityFight();
  void checkKeeperDanger();
  void addDeityServant(Deity*, Vec2 deityPos, Vec2 victimPos);
  static string getWarningText(Collective::Warning);

  void onWorshipEpithet(EpithetId);
  void addCreature(Creature* c, EnumSet<MinionTrait>);
  Creature* addCreature(PCreature c, Vec2 v, EnumSet<MinionTrait>);
  void importCreature(Creature* c, EnumSet<MinionTrait> traits);
  Creature* getCreature(UniqueId id);
  void handleCreatureButton(Creature* c, View* view);
  void unpossess();
  void possess(const Creature*, View*);
  struct BuildInfo {
    struct SquareInfo {
      SquareType type;
      Collective::CostInfo cost;
      string name;
      bool buildImmediatly;
      bool noCredit;
    } squareInfo;

    struct TrapInfo {
      TrapType type;
      string name;
      ViewId viewId;
    } trapInfo;

    enum BuildType { DIG, SQUARE, IMP, TRAP, GUARD_POST, DESTROY, FETCH, DISPATCH } buildType;

    Optional<TechId> techId;
    string help;
    char hotkey;
    string groupName;

    BuildInfo(SquareInfo info, Optional<TechId> techId = Nothing(), const string& h = "", char hotkey = 0,
        string group = "");
    BuildInfo(TrapInfo info, Optional<TechId> techId = Nothing(), const string& h = "", char hotkey = 0,
        string group = "");
    BuildInfo(DeityHabitat, Collective::CostInfo, const string& groupName, const string& h = "", char hotkey = 0);
    BuildInfo(const Creature*, Collective::CostInfo, const string& groupName, const string& h = "", char hotkey = 0);
    BuildInfo(BuildType type, const string& h = "", char hotkey = 0, string group = "");
  };
  void handleSelection(Vec2 pos, const BuildInfo&, bool rectangle);
  vector<GameInfo::BandInfo::Button> fillButtons(const vector<BuildInfo>& buildInfo) const;
  vector<BuildInfo> getBuildInfo() const;
  static vector<BuildInfo> getBuildInfo(const Level*);
  static vector<BuildInfo> workshopInfo;
  static vector<BuildInfo> libraryInfo;
  static vector<BuildInfo> minionsInfo;

  ViewObject getResourceViewObject(Collective::ResourceId id) const;
  Optional<pair<ViewObject, int>> getCostObj(Collective::CostInfo) const;

  typedef GameInfo::BandInfo::TechButton TechButton;

  int getMinLibrarySize() const;

  struct TechInfo {
    TechButton button;
    function<void(PlayerControl*, View*)> butFun;
  };
  vector<TechInfo> getTechInfo() const;

  MoveInfo getBeastMove(Creature* c);
  MoveInfo getPossessedMove(Creature* c);
  MoveInfo getBacktrackMove(Creature* c);

  int getImpCost() const;
  bool canBuildDoor(Vec2 pos) const;
  bool canPlacePost(Vec2 pos) const;
  void handleMarket(View*, int prevItem = 0);
  void getEquipmentItem(View* view, ItemPredicate predicate);
  Item* chooseEquipmentItem(View* view, vector<Item*> currentItems, ItemPredicate predicate,
      int* index = nullptr, double* scrollPos = nullptr) const;

  void getMinionOptions(Creature*, vector<MinionOption>&, vector<View::ListElem>&);

  int getNumMinions() const;
  void minionView(View* view, Creature* creature, int prevItem = 0);
  void handleEquipment(View* view, Creature* creature, int prevItem = 0);
  void handleNecromancy(View*);
  void handleMatterAnimation(View*);
  void handleBeastTaming(View*);
  void handleHumanoidBreeding(View*);
  void handleSpawning(View* view, SquareType spawnSquare, const string& info1, 
      const string& info2, const string& title, MinionTrait spawnTrait, vector<SpawnInfo> spawnInfo,
      double multiplier, Optional<vector<pair<Vec2, Item*>>> genItems = Nothing(), string genItemsInfo = "",
      string info3 = "");
  void handlePersonalSpells(View*);
  void handleLibrary(View*);
  static ViewObject getTrapObject(TrapType type);
  bool underAttack() const;
  void addToMemory(Vec2 pos);
  void updateMemory();
  bool tryLockingDoor(Vec2 pos);
  void uncoverRandomLocation();

  mutable unique_ptr<map<UniqueId, MapMemory>> SERIAL(memory);
  bool SERIAL2(gatheringTeam, false);
  vector<Creature*> SERIAL(team);
  map<const Level*, Vec2> SERIAL(teamLevelChanges);
  map<const Level*, Vec2> SERIAL(levelChangeHistory);
  Creature* SERIAL2(possessed, nullptr);
  Model* SERIAL(model);
  bool SERIAL2(showWelcomeMsg, true);
  Optional<Vec2> rectSelectCorner;
  Optional<Vec2> rectSelectCorner2;
  double SERIAL2(lastControlKeeperQuestion, -100);
  int SERIAL2(startImpNum, -1);
  bool SERIAL2(retired, false);
  int SERIAL2(executions, 0);
  unique_ptr<Sectors> SERIAL(sectors);
  unique_ptr<Sectors> SERIAL(flyingSectors);
  unordered_set<Vec2> SERIAL(surprises);
};

#endif
