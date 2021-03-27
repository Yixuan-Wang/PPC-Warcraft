#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <array>
#include <vector>

#define COUNT_WARRIOR_TYPE 5

#define DRAGON 'd'
#define NINJA 'n'
#define ICEMAN 'i'
#define LION 'l'
#define WOLF 'w'

using namespace std;

string to_full_army(char army) {
    return army == 'r' ? (string)"red" : (string)"blue" ;
}

string to_full_type(char type) {
    string full_type;
    switch(type) {
        case DRAGON: full_type = "dragon"; break;
        case NINJA: full_type = "ninja"; break;
        case ICEMAN: full_type = "iceman"; break;
        case LION: full_type = "lion"; break;
        case WOLF: full_type = "wolf"; break;
    }
    return full_type;
}

class Warrior {
public:
    int id;
    char type;
    char army;
    int tick_born;
    int hp;

    Warrior(int id, char type, char army, int tick_born, int hp):
        id(id),
        type(type),
        army(army),
        tick_born(tick_born),
        hp(hp)
    {};
};

class Dragon: public Warrior {
public:
    Dragon(int id, char army, int tick_born, int hp):
        Warrior(id, DRAGON, army, tick_born, hp)
    {};
};

class Ninja: public Warrior {
public:
    Ninja(int id, char army, int tick_born, int hp):
        Warrior(id, NINJA, army, tick_born, hp)
    {};    
};

class Iceman: public Warrior {
public:
    Iceman(int id, char army, int tick_born, int hp):
        Warrior(id, ICEMAN, army, tick_born, hp)
    {}; 
};

class Lion: public Warrior {
public:
    Lion(int id, char army, int tick_born, int hp):
        Warrior(id, LION, army, tick_born, hp)
    {};
};

class Wolf: public Warrior {
public:
    Wolf(int id, char army, int tick_born, int hp):
        Warrior(id, WOLF, army, tick_born, hp)
    {};
};

using WarriorOrder = array<char, COUNT_WARRIOR_TYPE>;
using Units = vector<Warrior *>;
using map_c_i = map<char, int>;

enum HqStatus { ACTIVE, STOPPED };
class Headquarter {
public:
    char army;
    int hp;
    WarriorOrder warrior_order;
    Units units;
    map_c_i len_units;

    int tick;
    HqStatus status;
    map_c_i *p_default_hp;

private:
    int gen_current;

public:

    Headquarter(char army, int hp, WarriorOrder warrior_order):
        army(army),
        hp(hp),
        warrior_order(warrior_order),
        units(),
        len_units({{DRAGON, 0}, {NINJA, 0}, {ICEMAN, 0}, {LION, 0}, {WOLF, 0}}),
        status(ACTIVE),
        gen_current(0)
    {};

    ~Headquarter() {
        Units::iterator iter_units;
        for(iter_units = this->units.begin(); iter_units != this->units.end(); ++iter_units) { delete *iter_units; }
    }

private:
    void log_gen(const Warrior &);
    void log_stop_gen();

public:
    void gen();
};
void Headquarter::gen() {
    enum GenStatus { STOPPED, SUCCESS, TRY, FALLTHROUGH } status;
    status = TRY;

    int id = this->units.size() + 1;
    int first_try = this->gen_current;

    Warrior *p_warrior = nullptr;
    char warrior_type = ' ';
    int warrior_hp_cost = 0;
    while(status >= TRY) {
        if(status == FALLTHROUGH && this->gen_current == first_try) {
            this->log_stop_gen();
            status = STOPPED;
            this->status = HqStatus::STOPPED;
            break;
        }
        // cout << "army " << this->army << " tries " << this->warrior_order[this->gen_current] << endl;
        warrior_type = this->warrior_order[this->gen_current];
        warrior_hp_cost = (*(this->p_default_hp))[warrior_type];
        if(this->hp >= warrior_hp_cost) {
            this->hp -= warrior_hp_cost;
            switch(warrior_type) {
                case DRAGON: p_warrior = new Dragon(id, this->army, this->tick, warrior_hp_cost); break;
                case NINJA: p_warrior = new Ninja(id, this->army, this->tick, warrior_hp_cost); break;
                case ICEMAN: p_warrior = new Iceman(id, this->army, this->tick, warrior_hp_cost); break;
                case LION: p_warrior = new Lion(id, this->army, this->tick, warrior_hp_cost); break;
                case WOLF: p_warrior = new Wolf(id, this->army, this->tick, warrior_hp_cost); break;
            }
            status = SUCCESS;
            this->units.push_back(p_warrior);
            this->len_units[warrior_type]++;
            this->log_gen(*p_warrior);
        } else status = FALLTHROUGH;
        this->gen_current = (this->gen_current + 1) % COUNT_WARRIOR_TYPE;
    }
    
}
void Headquarter::log_gen(const Warrior & warrior) {
    const string full_army{to_full_army(this->army)}, full_type{to_full_type(warrior.type)};
    cout
        << setw(3) << setfill('0') << this->tick << ' '
        << full_army << ' ' << full_type << ' ' << warrior.id << ' '
        << "born with strength " << warrior.hp << ','
        << this->len_units[warrior.type] << ' ' << full_type << " in " << full_army << " headquarter"
        << endl;
}
void Headquarter::log_stop_gen() {
    cout
        << setw(3) << setfill('0') << this->tick << ' '
        << to_full_army(this->army) << " headquarter stops making warriors"
        << endl;
}

class Game {
public:
    Headquarter red, blue;
    int tick;
    map_c_i default_hp;

    Game(int hp_hq, int hp_dragon, int hp_ninja, int hp_iceman, int hp_lion, int hp_wolf):
        red('r', hp_hq, {ICEMAN, LION, WOLF, NINJA, DRAGON}),
        blue('b', hp_hq, {LION, DRAGON, NINJA, ICEMAN, WOLF}),
        tick(0),
        default_hp{{DRAGON, hp_dragon},{NINJA, hp_ninja},{ICEMAN, hp_iceman},{LION, hp_lion},{WOLF, hp_wolf}}
    {
        red.p_default_hp = &default_hp; blue.p_default_hp = &default_hp;
    };

    void start();
    void gen();
};
void Game::start() {
    while(red.status == HqStatus::ACTIVE || blue.status == HqStatus::ACTIVE) {
        red.tick = tick; blue.tick = tick;
        gen();
        // cout << endl; 

        tick++;
    }
}
void Game::gen() {
    if(red.status == HqStatus::ACTIVE) red.gen();
    if(blue.status == HqStatus::ACTIVE) blue.gen();
}

int main() {
    // freopen("in.txt", "r", stdin);
    // freopen("out.txt","w", stdout);

    int n;
    cin >> n;

    int hp_hq, hp_dragon, hp_ninja, hp_iceman, hp_lion, hp_wolf; 

    for(int i = 1; i <= n; ++i) {
        cin
            >> hp_hq
            >> hp_dragon >> hp_ninja >> hp_iceman >> hp_lion >> hp_wolf;
        cout << "Case:" << i << endl;
        
        Game game(
            hp_hq,
            hp_dragon, hp_ninja, hp_iceman, hp_lion, hp_wolf
        );
        game.start();
    }
}