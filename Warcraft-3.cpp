#include <iostream>
#include <iomanip>
#include <cstdio>
#include <string>
#include <array>
#include <vector>
#include <algorithm> 

#define COUNT_WARRIOR_TYPE 5
#define COUNT_WEAPON_TYPE 3
#define MAX_WEAPON 10

#define RED 0
#define BLUE 1
#define NEUTRAL -1

#define DRAGON 0
#define NINJA 1
#define ICEMAN 2
#define LION 3
#define WOLF 4

#define SWORD 0
#define BOMB 1
#define ARROW 2

using namespace std;

enum GameStatus { GAMING, GAME_OVER };

inline int to_enemy(int army) { return army == RED ? BLUE : RED; }

string to_full_army(int army) {
    return army == RED ? (string)"red" : (string)"blue" ;
}

string to_full_warrior(int type) {
    string full_warrior;
    switch(type) {
        case DRAGON: full_warrior = "dragon"; break;
        case NINJA: full_warrior = "ninja"; break;
        case ICEMAN: full_warrior = "iceman"; break;
        case LION: full_warrior = "lion"; break;
        case WOLF: full_warrior = "wolf"; break;
    }
    return full_warrior;
}

string to_full_weapon(int type) {
    string full_weapon;
    switch(type) {
        case SWORD: full_weapon = "sword"; break;
        case BOMB: full_weapon = "bomb"; break;
        case ARROW: full_weapon = "arrow"; break;
    }
    return full_weapon;
}

void log_tick();

class Game;
class City;
class Headquarter;
class Warrior;
class Weapon;

int game_tick;

using Hand = vector<Weapon*>;
class Weapon {
public:
    int type;
    int army;
    int durab;
    static vector<Weapon *> weapons;

    Weapon(int type, int army): type(type), army(army), durab(0) {};
    virtual ~Weapon() {};
    virtual int use(Warrior*, Warrior*) {return 0;};
};
vector<Weapon *> Weapon::weapons{};
Hand & sort_weapon(Hand &hand, bool prior_low_durab) {
    if(hand.empty()) return hand;
    sort(hand.begin(), hand.end(), [=](Weapon *a, Weapon *b) {
        return a->type < b->type || (a->type==ARROW && b->type==ARROW && (
            prior_low_durab ? a->durab < b->durab : a->durab > b->durab
        ));
    });
    return hand;
}


class Sword: public Weapon {
public:
    Sword(int army): Weapon(SWORD, army) { durab = 100; };
    int use(Warrior *, Warrior *);
};

class Bomb: public Weapon {
public:
    Bomb(int army): Weapon(BOMB, army) { durab = 1; };
    int use(Warrior *, Warrior *);
};
class Arrow: public Weapon {
public:
    Arrow(int army): Weapon(ARROW, army) { durab = 2; };
    int use(Warrior *, Warrior *);
};

class City {
public:
    int id;
    bool is_hq;
    int army;
    Warrior *post[2];

    City(int id, bool is_hq, int army): id(id), is_hq(is_hq), army(army), post{nullptr, nullptr} {}
    void fight();
};

class Warrior {
public:
    int id;
    int type;
    int army;
    int tick_born;
    int hp;
    int atk;
    Hand hand;

    Warrior(int id, int type, int army, int tick_born, int hp, int atk, Weapon * weapon):
        id(id), type(type), army(army), tick_born(tick_born), hp(hp), atk(atk)
    {
        if(weapon) hand.push_back(weapon);
    };
    virtual ~Warrior() {};
    
    virtual void log_gen() const {}
    void log_report();
    virtual bool check_flee(City *location) {return false;}
    virtual bool hook_move(City *location) {return true;}
    virtual bool hook_not_dead(City *) {return true;}
    virtual bool loot(Warrior*, City*, bool);
    virtual bool loot(City*, bool);
    virtual void attack(Warrior*, Weapon*);
    virtual void hurt(int, Warrior*, Weapon*);
};
bool Warrior::loot(City *location, bool is_wolf_loot = false) {
    return Warrior::loot(location->post[to_enemy(army)], location, is_wolf_loot);
}
bool Warrior::loot(Warrior * enemy, City *location, bool is_wolf_loot = false) {
    if(enemy == nullptr || enemy->hand.empty()) return false;
    if(is_wolf_loot && enemy->type == WOLF) return false;
    sort_weapon(enemy->hand, false);
    int loot_type = enemy->hand[0]->type; // hand is non-empty. only wolf loot restrict loot type
    int counter{0};
    while(!enemy->hand.empty() && hand.size()+1<MAX_WEAPON && (enemy->hand[0]->type == loot_type || !is_wolf_loot)) {
        hand.push_back(enemy->hand[0]);
        enemy->hand.erase(enemy->hand.begin());
        ++counter;
    }
    sort_weapon(hand, true); sort_weapon(enemy->hand, true);

    if(is_wolf_loot) {
        log_tick(); cout << to_full_army(army) << " wolf " << id << " took " << counter << " " << to_full_weapon(loot_type) << " from " << to_full_army(enemy->army) << " " << to_full_warrior(enemy->type) << " " << enemy->id << " in city " << location->id << endl;
    }
    return true;
}
void Warrior::log_report() {
    log_tick();
    int count_weapon[3]={0};
    if(!hand.empty()) for(Hand::iterator hand_iter = hand.begin(); hand_iter != hand.end(); ++hand_iter) { ++count_weapon[(*hand_iter)->type]; }
    cout << to_full_army(army) << ' ' << to_full_warrior(type) << ' ' << id
        << " has " << count_weapon[SWORD] << " sword " << count_weapon[BOMB] << " bomb " << count_weapon[ARROW] << " arrow and "
        << hp << " elements" << endl;
}
void Warrior::attack(Warrior *target, Weapon *weapon) {
    int hurt{weapon->use(this, target)};
    target->hurt(hurt, this, weapon);
    if(weapon->type == BOMB) {
        if(this->type != NINJA) this->hurt(hurt / 2, this, weapon);
    }
}
void Warrior::hurt(int hurt, Warrior *target, Weapon *weapon) {
    this->hp -= hurt;
}

int Sword::use(Warrior *user, Warrior *target) {
    return user->atk * 2 / 10;
}

int Bomb::use(Warrior *user, Warrior *target)  {
    --this->durab;
    return user->atk * 4 / 10;
}

int Arrow::use(Warrior *user, Warrior *usee) {
    --this->durab;
    return user->atk * 3 / 10;
}

void City::fight() {
    Warrior *first{id % 2 == 1 ? post[RED] : post[BLUE]}, *second{id % 2 == 1 ? post[BLUE] : post[RED]};
    Warrior *winner{nullptr}, *loser{nullptr};
    enum FightStatus { FIGHTING, BEAT, TIE, BOTH_DIE } status;
    int old_hp[2]{0};
    int w_f{0}, w_s{0}; // Weapon first/second, index
    Weapon *weapon{nullptr};

    status = FIGHTING;
    sort_weapon(first->hand, true); sort_weapon(second->hand, true);
    auto round = [&](Warrior *attacker, Warrior *target, int &wi) {
        if(wi == -1) return;
        weapon = attacker->hand[wi];
        attacker->attack(target, weapon);
        if(weapon->durab <= 0) {
            attacker->hand.erase(attacker->hand.begin() + wi);
            if(attacker->hand.empty()) { wi = -1; }
        } else ++wi;
        if(!attacker->hand.empty()) wi = wi % attacker->hand.size();
        if(target->hp <= 0) {
            if(attacker->hp <= 0) { status = BOTH_DIE; }
            else { winner = attacker; loser = target; status = BEAT; }
        } else if(attacker->hp <= 0) { winner = target; loser = attacker; status = BEAT; }
    };
    auto hand_not_changeable = [&](Hand &hand) {
        return hand.empty() || hand.back()->type == SWORD;
    };
    while(status == FIGHTING) {
        old_hp[0] = first->hp; old_hp[1] = second->hp;
        if(first->hand.empty()) w_f = -1; if(second->hand.empty()) w_s = -1;
        if(w_f == -1 && w_s == -1) { status = TIE; break; }
        round(first, second, w_f);
        if(status != FIGHTING) break;
        if(first->hand.empty()) w_f = -1; if(second->hand.empty()) w_s = -1;
        if(w_f == -1 && w_s == -1) { status = TIE; break; }
        round(second, first, w_s);
        if(status != FIGHTING) break;
        if(old_hp[0] == first->hp && old_hp[1] == second->hp && hand_not_changeable(first->hand) && hand_not_changeable(second->hand)) { status = TIE; break; } 
    }
    if(status == BEAT) {
        log_tick(); cout << to_full_army(winner->army) << " " << to_full_warrior(winner->type) << " " << winner->id << " killed " << to_full_army(loser->army) << " " << to_full_warrior(loser->type) << " " << loser->id << " in city " << this->id << " remaining " << winner->hp << " elements" << endl;
        winner->loot(loser, this, false);
        winner->hook_not_dead(this);
        post[loser->army] = nullptr;
    } else if (status == TIE || status == BOTH_DIE) {
        log_tick(); cout << "both red " << to_full_warrior(post[RED]->type) << " " << post[RED]->id << " and blue " << to_full_warrior(post[BLUE]->type) << " " << post[BLUE]->id << (status == TIE ? " were alive in city " : " died in city ") << id << endl;
        if(status == TIE) { post[RED]->hook_not_dead(this); post[BLUE]->hook_not_dead(this); }
        if(status == BOTH_DIE) { post[RED] = nullptr; post[BLUE] = nullptr; }
    }
}

class Dragon: public Warrior {
public:

    Dragon(int id, int army, int tick_born, int hp, int atk, Weapon * weapon):
        Warrior(id, DRAGON, army, tick_born, hp, atk, weapon)
    {};

    virtual void log_gen() const {}
    bool hook_not_dead(City *location) {
        log_tick(); cout << to_full_army(army) << " dragon " << id << " yelled in city " << location->id << endl;
        return true;
    }
};

class Ninja: public Warrior {
public:
    Ninja(int id, int army, int tick_born, int hp, int atk, Weapon * weapon, Weapon * weapon2):
        Warrior(id, NINJA, army, tick_born, hp, atk, weapon)
    {
        if(weapon2) hand.push_back(weapon2);
    };

    virtual void log_gen() const {
        // cout
        //     << "It has a " << to_full_weapon(this->weapon->type) << " and a " << to_full_weapon(this->weapon2->type)
        //     << endl;
    }
};

class Iceman: public Warrior {
public:
    Iceman(int id, int army, int tick_born, int hp, int atk, Weapon * weapon):
        Warrior(id, ICEMAN, army, tick_born, hp, atk, weapon)
    {};

    virtual void log_gen() const {
        // cout
        //     << "It has a " << to_full_weapon(this->weapon->type)
        //     << endl;
    }

    virtual bool hook_move(City *location) { 
        hp -= hp / 10;
        if(hp <= 0) { location->post[army] = nullptr; return false; }
        return true;
    }
};

class Lion: public Warrior {
public:
    int loyalty;
    static int step_loyalty;

    Lion(int id, int army, int tick_born, int hp, int atk, Weapon * weapon, int loyalty):
        Warrior(id, LION, army, tick_born, hp, atk, weapon),
        loyalty(loyalty)
    {};

    virtual void log_gen() const { cout << "Its loyalty is " << this->loyalty << endl; }

    virtual bool check_flee(City *location) {
        if(loyalty <= 0 && (!location->is_hq || location->army==army)) {
            hp = 0; location->post[army] = nullptr;
            log_tick(); cout << to_full_army(army) << " lion " << id <<" ran away" << endl;
            return true;
        }
        return false;
    }

    virtual bool hook_move(City *location) { loyalty -= step_loyalty; return true; }
};
int Lion::step_loyalty = 0;

class Wolf: public Warrior {
public:
    Wolf(int id, int army, int tick_born, int hp, int atk):
        Warrior(id, WOLF, army, tick_born, hp, atk, nullptr)
    {};
};

using WarriorOrder = array<int, COUNT_WARRIOR_TYPE>;
using Units = vector<Warrior *>;

enum HqStatus { ACTIVE, STOPPED };
class Headquarter {
public:
    int army;
    int hp;
    WarriorOrder warrior_order;
    Units units;
    int len_units[COUNT_WARRIOR_TYPE];

    HqStatus status;
    int *p_default_hp, *p_default_atk;
    City *hq;
    int gen_current;

    Headquarter(int army, int hp, WarriorOrder warrior_order):
        army(army),
        hp(hp),
        warrior_order(warrior_order),
        units(),
        len_units{0, 0, 0, 0, 0},
        status(ACTIVE),
        gen_current(0),
        hq(nullptr)
    {};

    ~Headquarter() {
        Units::iterator iter_units;
        for(iter_units = this->units.begin(); iter_units != this->units.end(); ++iter_units) { delete *iter_units; }
    }
    void log_gen(const Warrior &);
    void log_stop_gen();
    void log_hq_hp();

    void gen_warrior();
    Weapon * gen_weapon(int weapon_id);
    
};
void Headquarter::gen_warrior() {
    enum GenStatus { STOPPED, SUCCESS, TRY, FALLTHROUGH } status;
    status = TRY;

    int id = this->units.size() + 1;
    int first_try = this->gen_current;

    Warrior *p_warrior = nullptr;
    int warrior_type = 0;
    int warrior_hp_cost = 0;
    while(status >= TRY) {
        if(status == FALLTHROUGH && this->gen_current == first_try) {
            // this->log_stop_gen();
            status = STOPPED;
            this->status = HqStatus::STOPPED;
            break;
        }
        // cout << "army " << this->army << " tries " << this->warrior_order[this->gen_current] << endl;
        warrior_type = this->warrior_order[this->gen_current];
        warrior_hp_cost = this->p_default_hp[warrior_type];
        if(this->hp >= warrior_hp_cost) {
            this->hp -= warrior_hp_cost;
            switch(warrior_type) {
                case DRAGON:
                    p_warrior = new Dragon(id, this->army, game_tick,
                        warrior_hp_cost, this->p_default_atk[warrior_type],
                        gen_weapon(id % 3)); break;
                case NINJA:
                    p_warrior = new Ninja(id, this->army, game_tick,
                        warrior_hp_cost, this->p_default_atk[warrior_type],
                        gen_weapon(id % 3), gen_weapon((id + 1) % 3)
                    ); break;
                case ICEMAN:
                    p_warrior = new Iceman(id, this->army, game_tick,
                        warrior_hp_cost, this->p_default_atk[warrior_type],
                        gen_weapon(id % 3)); break;
                case LION:
                    p_warrior = new Lion(id, this->army, game_tick,
                        warrior_hp_cost, this->p_default_atk[warrior_type],
                        gen_weapon(id % 3), this->hp); break;
                case WOLF:
                    p_warrior = new Wolf(id, this->army, game_tick,
                    warrior_hp_cost, this->p_default_atk[warrior_type]); break;
            }
            status = SUCCESS;
            this->units.push_back(p_warrior);
            this->hq->post[this->army] = p_warrior;
            this->len_units[warrior_type]++;
            this->log_gen(*p_warrior);
        } else {
            // status = FALLTHROUGH;
            // this->log_stop_gen();
            status = STOPPED;
            this->status = HqStatus::STOPPED;
            break;
        } // NO FALLTHROUGH
        this->gen_current = (this->gen_current + 1) % COUNT_WARRIOR_TYPE;
    }
}
Weapon * Headquarter::gen_weapon(int weapon_id) {
    Weapon * weapon;
    switch(weapon_id) {
        case SWORD: weapon = new Sword(this->army); break;
        case BOMB: weapon = new Bomb(this->army); break;
        case ARROW: weapon = new Arrow(this->army); break;
    }
    Weapon::weapons.push_back(weapon);
    return weapon;
}
void Headquarter::log_gen(const Warrior & warrior) {
    const string full_army{to_full_army(this->army)}, full_warrior{to_full_warrior(warrior.type)};
    log_tick();
    cout << full_army << ' ' << full_warrior << ' ' << warrior.id << " born" << endl;
    warrior.log_gen();
}
void Headquarter::log_hq_hp() {
    log_tick();
    cout << hp << " elements in " << to_full_army(army) << " headquarter" << endl;
}

/* void Headquarter::log_stop_gen() {
    log_tick();
    cout
        << to_full_army(this->army) << " headquarter stops making warriors"
        << endl;
} */

class Game {
public:
    Headquarter red, blue;
    static GameStatus status;
    int default_hp[COUNT_WARRIOR_TYPE];
    int default_atk[COUNT_WARRIOR_TYPE];
    int count_city, endtime;
    static vector<Weapon*> weapons;
    vector<City*> cities;

    Game(int hp_hq, int count_city, int endtime,
        int hp_dragon, int hp_ninja, int hp_iceman, int hp_lion, int hp_wolf,
        int atk_dragon, int atk_ninja, int atk_iceman, int atk_lion, int atk_wolf 
    ):
        red(RED, hp_hq, {ICEMAN, LION, WOLF, NINJA, DRAGON}),
        blue(BLUE, hp_hq, {LION, DRAGON, NINJA, ICEMAN, WOLF}),
        count_city(count_city),
        endtime(endtime),
        default_hp{hp_dragon, hp_ninja, hp_iceman, hp_lion, hp_wolf},
        default_atk{atk_dragon, atk_ninja, atk_iceman, atk_lion, atk_wolf}
    {
        game_tick = 0;
        Game::status = GameStatus::GAMING;
        red.p_default_hp = default_hp; blue.p_default_hp = default_hp;
        red.p_default_atk = default_atk; blue.p_default_atk = default_atk;

        for(int i = 0; i < count_city+2; ++i) {
            cities.push_back(new City(i, i == 0 || i == count_city+1,
                i == 0 ? RED : (i == count_city+1 ? BLUE : NEUTRAL)
            ));
        }
        red.hq = cities.front(); blue.hq = cities.back();
    };

    ~Game() {
        vector<Weapon *>::iterator iter_weapons;
        for(iter_weapons = Weapon::weapons.begin(); iter_weapons != Weapon::weapons.end(); ++iter_weapons) { 
            delete *iter_weapons;
        }
        Weapon::weapons.clear();

        vector<City *>::iterator iter_cities;
        for(iter_cities = cities.begin(); iter_cities != cities.end(); ++iter_cities) { 
            delete *iter_cities;
        }
    }

    void log_move(Warrior &warrior, City &dest);

    void start();
    void gen();
    void flee();
    void move();
    void wolf_loot();
    void combat();
    void win(Warrior &warrior);
    void report();
};
void log_tick() {
    cout << setw(3) << setfill('0') << game_tick / 60 << ':' << setw(2) << setfill('0') << game_tick % 60 << ' ';
}
GameStatus Game::status = GameStatus::GAMING;
void Game::start() {
    while(game_tick <= endtime && status == GameStatus::GAMING/* red.status == HqStatus::ACTIVE || blue.status == HqStatus::ACTIVE */) {
        switch(game_tick%60) {
            case 0: gen(); break;
            case 5: flee(); break;
            case 10: move(); break;
            case 35: wolf_loot(); break;
            case 40: combat(); break;
            case 50: red.log_hq_hp(); blue.log_hq_hp(); break;
            case 55: report(); break;
        }
        game_tick+=5;
    }
}
void Game::gen() {
    if(red.status == HqStatus::ACTIVE) red.gen_warrior();
    if(blue.status == HqStatus::ACTIVE) blue.gen_warrior();
}
void Game::flee() {
    Warrior * warrior;
    for(int i = 0; i < count_city + 2; ++i) {
        if(cities[i]->post[RED] && cities[i]->post[RED]->type == LION) { cities[i]->post[RED]->check_flee(cities[i]); }
        if(cities[i]->post[BLUE] && cities[i]->post[BLUE]->type == LION) { cities[i]->post[BLUE]->check_flee(cities[i]); }
    }
}
void Game::move() {
    Warrior * warrior;
    for(int i = 0; i < count_city + 2; ++i) {
        if(i > 0 && cities[i-1]->post[RED]) {
            warrior = cities[i-1]->post[RED];
            if(warrior->hook_move(cities[i-1])) {
                if(i == count_city+1) win(*warrior);
                else log_move(*warrior, *cities[i]);
            }
            
        }
        if(i < count_city+1 && cities[i+1]->post[BLUE]) {
            warrior = cities[i+1]->post[BLUE];
            if(warrior->hook_move(cities[i-1])) {
                if(i == 0) win(*warrior);
                else log_move(*warrior, *cities[i]);
            }
        }
    }
    for(int i = count_city+1; i > 0; --i) {
        if(cities[i-1]->post[RED]) {
            cities[i]->post[RED] = cities[i-1]->post[RED]; cities[i-1]->post[RED] = nullptr;
        }
    }
    for(int i = 0; i < count_city+1; ++i) {
        if(cities[i+1]->post[BLUE]) {
            cities[i]->post[BLUE] = cities[i+1]->post[BLUE]; cities[i+1]->post[BLUE] = nullptr;
        }
    }
}
void Game::combat() {
    for(int i = 1; i < count_city+1; ++i) {
        if(cities[i]->post[RED] && cities[i]->post[BLUE]) {
            // cout << "DEBUG: COMBAT at city " << i << endl; //! DEBUG
            cities[i]->fight();
        }
    }
}
void Game::wolf_loot() {
    Warrior * warrior;
    for(int i = 0; i < count_city + 2; ++i) {
        if(cities[i]->post[RED] && cities[i]->post[RED]->type == WOLF) { cities[i]->post[RED]->loot(cities[i], true); }
        if(cities[i]->post[BLUE] && cities[i]->post[BLUE]->type == WOLF) { cities[i]->post[BLUE]->loot(cities[i], true); }
    }
}
void Game::log_move(Warrior &warrior, City &dest) {
    log_tick();
    cout << to_full_army(warrior.army) << " " << to_full_warrior(warrior.type) << " " << warrior.id << " marched to city " << dest.id << " with " << warrior.hp << " elements and force " << warrior.atk << endl;
}
void Game::win(Warrior &warrior) {
    Game::status = GameStatus::GAME_OVER;
    log_tick();
    cout << to_full_army(warrior.army) << " " << to_full_warrior(warrior.type) << " " << warrior.id
        << " reached " << to_full_army(to_enemy(warrior.army))
        << " headquarter with " << warrior.hp << " elements and force " << warrior.atk << endl;
    log_tick();
    cout << to_full_army(to_enemy(warrior.army)) << " headquarter was taken" << endl;
}
void Game::report() {
    Warrior * warrior;
    for(int i = 0; i < count_city + 2; ++i) {
        if(cities[i]->post[RED]) { cities[i]->post[RED]->log_report(); }
        if(cities[i]->post[BLUE]) { cities[i]->post[BLUE]->log_report(); }
    }
}

int main() {
    // freopen("in.txt", "r", stdin);
    // freopen("out.txt","w", stdout);

    int n;
    cin >> n;

    int hp_hq, count_city, step_loyalty, endtime;
    int hp_dragon, hp_ninja, hp_iceman, hp_lion, hp_wolf; 
    int atk_dragon, atk_ninja, atk_iceman, atk_lion, atk_wolf;

    for(int i = 1; i <= n; ++i) {
        cin
            >> hp_hq >> count_city >> Lion::step_loyalty >> endtime
            >> hp_dragon >> hp_ninja >> hp_iceman >> hp_lion >> hp_wolf
            >> atk_dragon >> atk_ninja >> atk_iceman >> atk_lion >> atk_wolf;
        cout << "Case " << i << ":" << endl;
        
        Game game(
            hp_hq, count_city, endtime,
            hp_dragon, hp_ninja, hp_iceman, hp_lion, hp_wolf,
            atk_dragon, atk_ninja, atk_iceman, atk_lion, atk_wolf
        );
        game.start();
    }
}