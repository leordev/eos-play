/**
 * Pet Smart Contract
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

        // TODO: Utilize receipt eosio::transfer to manage account balances
//        // Pay for Pet Creation Fee
//        if (pc.creation_fee > asset{}) {
//            action(
//                permission_level{ owner, N(active) },
//                N(eosio.token), N(transfer),
//                std::make_tuple(owner, _self, pc.creation_fee, std::string("PET - Creation Fee"))
//            ).send();
//        }

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

            // testing deferred
            transaction out{};
            out.actions.emplace_back(permission_level{_self, N(active)}, N(pet), N(feedpet), std::make_tuple(pet.id));
            out.delay_sec = 10;
            out.send(pet.id, _self);
        });
    }

    void feedpet(uuid pet_id) {
        print("feed lazy developer");
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
        uint8_t health = 100;
        uint8_t hunger = 70;
        uint8_t awake = 100;
        uint8_t happiness = 70;
        uint8_t clean = 100;
        uint32_t created_at;
        uint32_t last_fed_at;
        uint32_t last_play_at;
        uint32_t last_bed_at;
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



};

EOSIO_ABI(pet, (createpet)(feedpet)(bedpet)(playpet)(washpet))
