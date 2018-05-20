/**
 * Pet-Tamagotchi-Alike Smart Contract
 *
 * The idea is to copy kind of the original tamagotchi to the chain :)
 *
 *
 * @author Leo Ribeiro
 */
#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/transaction.hpp>
#include <eosiolib/singleton.hpp>

using namespace eosio;

using std::string;
using std::hash;

/* ****************************************** */
/* ------------ Types Declarations ---------- */
/* ****************************************** */

typedef uint64_t uuid;

// There's five pet types
const uint8_t PET_TYPES = 5;
const uint32_t DAY = 86400;
const uint32_t TWENTY_HOURS = 72000;
const uint8_t  MAX_HEALTH = 100;
const uint32_t HUNGER_TO_ZERO = 120; // DAY
const uint8_t  MAX_HUNGER_POINTS = 100;
const uint8_t  HUNGER_HP_MODIFIER = 1;
const uint8_t  HUNGER_FEED_POINTS = 33;
const uint32_t HAPPINESS_TO_ZERO = 2 * DAY;
const uint8_t  MAX_HAPPINESS_POINTS = 100;
const uint8_t  HAPPINESS_HP_MODIFIER = 2;
const uint8_t  HAPPINESS_PLAY_POINTS = 33;
const uint32_t AWAKE_TO_ZERO = TWENTY_HOURS;
const uint8_t  MAX_AWAKE_POINTS = 100;
const uint8_t  AWAKE_BED_POINTS = 33;
const uint8_t  AWAKE_HP_MODIFIER = 2;
const uint32_t SHOWER_TO_ZERO = DAY;
const uint8_t  MAX_CLEAN_POINTS = 100;
const uint8_t  CLEAN_HP_MODIFIER = 3;
const uint8_t  CLEAN_SHOWER_POINTS = 33;

uint128_t combine_ids(uint64_t const& x, uint64_t const& y) {
    uint128_t times = 1;
    while (times <= y)
        times *= 10;

    return (x * times) + y;
}


/* ****************************************** */
/* ------------ Contract Definition --------- */
/* ****************************************** */

class pet : public eosio::contract {
public:
    pet(account_name self)
    :eosio::contract(self),
    pets(_self,_self),
    pet_config(_self,_self)
    {}

    /* ****************************************** */
    /* ------------ Contract Actions ------------ */
    /* ****************************************** */

    void createpet(name owner,
                   string pet_name) {

        require_auth(owner);

        st_pet_config pc = _get_pet_config();

        print("creating pet");

        // creates the pet
        pets.emplace(_self, [&](auto &r) {
            st_pets pet{};
            pet.id = _next_id();
            pet.name = pet_name;
            pet.owner = owner;
            pet.created_at = now();
            pet.last_fed_at = pet.created_at;
            pet.last_play_at = pet.created_at;
            pet.last_bed_at = pet.created_at;
            pet.last_shower_at = pet.created_at;

            pet.type = (_hash_str(pet_name) + pet.created_at + pet.id + owner) % PET_TYPES;

            r = pet;

//            // testing deferred
//            transaction out{};
//            out.actions.emplace_back(permission_level{_self, N(active)}, N(pet), N(updatepet), std::make_tuple(pet.id, 1));
//            out.delay_sec = 1;
//            out.send(pet.id, _self);
        });
    }

    void updatepet(uuid pet_id, uint32_t iteration) {
        require_auth(_self);
        print(pet_id, "|", iteration, ": updating pet ");

        auto itr_pet = pets.find(pet_id);
        eosio_assert(itr_pet != pets.end(), "E404|>Invalid pet");
        st_pets pet = *itr_pet;

        _update(pet);

        pets.modify(itr_pet, 0, [&](auto &r) {
            r.health = pet.health;
            r.death_at = pet.death_at;
            r.hunger = pet.hunger;
            r.awake = pet.awake;
            r.happiness = pet.happiness;
            r.clean = pet.clean;
        });

        // testing deferred recursive
        // does not look a good idea... better to have
        // a cronjob doing that for all pets maybe? :P
//        transaction out{};
//        out.actions.emplace_back(
//                permission_level{_self, N(active)},
//                N(pet), N(updatepet),
//                std::make_tuple(pet_id, iteration+1));
//        out.delay_sec = 60;
//        out.send(combine_ids(pet_id, uint64_t{iteration}), _self);
    }

    void feedpet(uuid pet_id, uint32_t iteration) {

        auto itr_pet = pets.find(pet_id);
        eosio_assert(itr_pet != pets.end(), "E404|>Invalid pet");
        st_pets pet = *itr_pet;

        _update(pet);

        pets.modify(itr_pet, 0, [&](auto &r) {
            r.health = pet.health;
            r.death_at = pet.death_at;
            r.hunger = pet.hunger;
            r.awake = pet.awake;
            r.happiness = pet.happiness;
            r.clean = pet.clean;

            if (r.health > 0) {
                r.hunger = (pet.hunger + HUNGER_FEED_POINTS) > MAX_HUNGER_POINTS ?
                        MAX_HUNGER_POINTS : (pet.hunger + HUNGER_FEED_POINTS);
                r.last_fed_at = now();
            }
        });
    }

    void bedpet(uuid pet_id) {
        print("bed lazy developer");
    }

    void playpet(uuid pet_id) {
        print("play lazy developer");
    }

    void washpet(uuid pet_id) {
        print("wash lazy developer");
    }


private:


    /* ****************************************** */
    /* ------------ Contract Tables ------------- */
    /* ****************************************** */

    // @abi table pets i64
    struct st_pets {
        uuid id;
        name owner;
        string name;
        uint8_t type;
        uint32_t created_at;
        uint32_t death_at = 0;
        uint8_t health = 100;
        uint8_t hunger = 70;
        uint32_t last_fed_at;
        uint8_t awake = 100;
        uint32_t last_bed_at;
        uint8_t happiness = 70;
        uint32_t last_play_at;
        uint8_t clean = 100;
        uint32_t last_shower_at;

        uint64_t primary_key() const { return id; }
    };

    typedef multi_index<N(pets), st_pets> _tb_pet;
    _tb_pet pets;



    /* ****************************************** */
    /* ------------ Contract Config Data -------- */
    /* ****************************************** */

    struct st_pet_config {
        uuid last_id = 1000000000;
        asset creation_fee = asset{10000,S(4,EOS)};
    };

    typedef singleton<N(pet_config), st_pet_config> pet_config_singleton;
    pet_config_singleton pet_config;

    /* ****************************************** */
    /* ------------ Private Functions ----------- */
    /* ****************************************** */

    st_pet_config _get_pet_config(){
        st_pet_config pc;

        if (pet_config.exists()) {
            pc = pet_config.get();
        }  else {
            pc = st_pet_config{};
            pet_config.set(pc, _self);
        }

        return pc;
    }

    uuid _next_id(){
        st_pet_config pc = _get_pet_config();
        pc.last_id++;
        pet_config.set(pc, _self);
        return pc.last_id;
    }

    uint64_t _hash_str(const string &str) {
        return hash<string>{}(str);
    }

    void _update(st_pets &pet) {

        eosio_assert(pet.health > 0 || pet.death_at > 0, "E099|>Pet is dead");

        uint32_t current_time = now();

        uint8_t effect_hp_hunger = _calc_hunger_hp(pet, current_time);

        int8_t hp = MAX_HEALTH - effect_hp_hunger;

        if (hp <= 0) {
            pet.health = 0;
            pet.death_at = current_time;
        } else {
            pet.health = hp;
        }
    }

    uint8_t _calc_hunger_hp(st_pets &pet, const uint32_t &current_time) {
        // how long it's hungry?
        uint32_t hungry_seconds = current_time - pet.last_fed_at;
        uint8_t hungry_points = (uint8_t) (hungry_seconds * MAX_HUNGER_POINTS / HUNGER_TO_ZERO);

        // calculates the effective hunger on hp, if pet hunger is 0
        uint8_t effect_hp_hunger = 0;
        if (hungry_points < MAX_HUNGER_POINTS) {
            pet.hunger = MAX_HUNGER_POINTS - hungry_points;
        } else {
            effect_hp_hunger = (uint8_t) ((hungry_points - MAX_HUNGER_POINTS) / HUNGER_HP_MODIFIER);
            pet.hunger = 0;
        }

        return effect_hp_hunger;
    }



};

EOSIO_ABI(pet, (createpet)(updatepet)(feedpet)(bedpet)(playpet)(washpet))
