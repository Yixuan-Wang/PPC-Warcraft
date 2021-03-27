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
    static int default_hp;

    Dragon(int id, char army, int tick_born):
        Warrior(id, DRAGON, army, tick_born, Dragon::default_hp)
    {};
};
int Dragon::default_hp{0};

class Ninja: public Warrior {
public:
    static int default_hp;

    Ninja(int id, char army, int tick_born):
        Warrior(id, NINJA, army, tick_born, Ninja::default_hp)
    {};    
};
int Ninja::default_hp{0};

class Iceman: public Warrior {
public:
    static int default_hp;

    Iceman(int id, char army, int tick_born):
        Warrior(id, ICEMAN, army, tick_born, Iceman::default_hp)
    {}; 
};
int Iceman::default_hp{0};

class Lion: public Warrior {
public:
    static int default_hp;

    Lion(int id, char army, int tick_born):
        Warrior(id, LION, army, tick_born, Lion::default_hp)
    {};
};
int Lion::default_hp{0};

class Wolf: public Warrior {
public:
    static int default_hp;
    Wolf(int id, char army, int tick_born):
        Warrior(id, WOLF, army, tick_born, Wolf::default_hp)
    {};
};
int Wolf::default_hp{0};

using WarriorOrder = array<char, COUNT_WARRIOR_TYPE>;
using Units = vector<Warrior *>;
using LenUnits = map<char, int>;

enum HqStatus { ACTIVE, STOPPED };
class Headquarter {
public:
    char army;
    int hp;
    WarriorOrder warrior_order;
    Units units;
    LenUnits len_units;

    int tick;
    HqStatus status;

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
    bool is_hp_enough(int hp_required) {
        // cout << this->army << " hp: " << this->hp << " hp_required: " << hp_required << endl;
        return this->hp >= hp_required; }
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

    while(status >= TRY) {
        if(status == FALLTHROUGH && this->gen_current == first_try) {
            this->log_stop_gen();
            status = STOPPED;
            this->status = HqStatus::STOPPED;
            break;
        }
        // cout << "army " << this->army << " tries " << this->warrior_order[this->gen_current] << endl;
        switch(this->warrior_order[this->gen_current]) {
            case DRAGON:
                if(is_hp_enough(Dragon::default_hp)) {
                    this->hp -= Dragon::default_hp;
                    Dragon * dragon = new Dragon(id, this->army, this->tick);
                    this->units.push_back(dragon);
                    this->len_units[DRAGON]++;
                    this->log_gen(*dragon);
                    status = SUCCESS;
                }
                else status = FALLTHROUGH;
                this->gen_current = (this->gen_current + 1) % COUNT_WARRIOR_TYPE;
                break;
            case NINJA:
                if(is_hp_enough(Ninja::default_hp)) {
                    this->hp -= Ninja::default_hp;
                    Ninja * ninja = new Ninja(id, this->army, this->tick);
                    this->units.push_back(ninja);
                    this->len_units[NINJA]++;
                    this->log_gen(*ninja);
                    status = SUCCESS;
                }
                else status = FALLTHROUGH;
                this->gen_current = (this->gen_current + 1) % COUNT_WARRIOR_TYPE;
                break;
            case ICEMAN:
                if(is_hp_enough(Iceman::default_hp)) {
                    this->hp -= Iceman::default_hp;
                    Iceman * iceman = new Iceman(id, this->army, this->tick);
                    this->units.push_back(iceman);
                    this->len_units[ICEMAN]++;
                    this->log_gen(*iceman);
                    status = SUCCESS;
                }
                else status = FALLTHROUGH;
                this->gen_current = (this->gen_current + 1) % COUNT_WARRIOR_TYPE;
                break;
            case LION:
                if(is_hp_enough(Lion::default_hp)) {
                    this->hp -= Lion::default_hp;
                    Lion * lion = new Lion(id, this->army, this->tick);
                    this->units.push_back(lion);
                    this->len_units[LION]++;
                    this->log_gen(*lion);
                    status = SUCCESS;
                }
                else status = FALLTHROUGH;
                this->gen_current = (this->gen_current + 1) % COUNT_WARRIOR_TYPE;
                break;
            case WOLF:
                if(is_hp_enough(Wolf::default_hp)) {
                    this->hp -= Wolf::default_hp;
                    Wolf * wolf = new Wolf(id, this->army, this->tick);
                    this->units.push_back(wolf);
                    this->len_units[WOLF]++;
                    this->log_gen(*wolf);
                    status = SUCCESS;
                }
                else status = FALLTHROUGH;
                this->gen_current = (this->gen_current + 1) % COUNT_WARRIOR_TYPE;
                break;
        }
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
private:
    int tick;
public:
    Headquarter red, blue;

    Game(int hp_hq, int hp_dragon, int hp_ninja, int hp_iceman, int hp_lion, int hp_wolf):
        red('r', hp_hq, {ICEMAN, LION, WOLF, NINJA, DRAGON}),
        blue('b', hp_hq, {LION, DRAGON, NINJA, ICEMAN, WOLF}),
        tick(0)
    {
        Dragon::default_hp = hp_dragon;
        Ninja::default_hp = hp_ninja;
        Iceman::default_hp = hp_iceman;
        Lion::default_hp = hp_lion;
        Wolf::default_hp = hp_wolf;
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