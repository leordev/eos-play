/**
 * Pet Smart Contract
 * @author Leo Ribeiro
 */
#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/singleton.hpp>

using namespace eosio;

using std::string;
using std::hash;

/* ****************************************** */
/* ------------ Types Declarations ---------- */
/* ****************************************** */

typedef uint64_t uuid;




/* ****************************************** */
/* ------------ Helper Functions ------------ */
/* ****************************************** */


uuid gen_uuid(const uint64_t &id) {
    return hash<uint64_t>{}(id);
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

    void createpet(account_name owner,
                   string name) {

        require_auth(owner);

        st_pet_config pc = _get_pet_config();

        // Pay for Pet Creation Fee
        if (pc.creation_fee > asset{}) {

            print(pc.creation_fee);

            action(permission_level{ owner, N(active) },
                   N(eosio.token), N(transfer),
                   std::make_tuple(owner, _self, pc.creation_fee, "PET - Creation Fee")
            ).send();
        }

        // creates the pet
        pets.emplace(_self, [&](auto &r) {
            r.id = gen_uuid(_next_id());
            r.name = name;
            r.owner = owner;
            r.health = pc.initial_health;
            r.hunger = pc.initial_hunger;
            r.created_at = now();
            r.last_fed_at = r.created_at;
        });

    }


private:


    /* ****************************************** */
    /* ------------ Contract Tables ------------- */
    /* ****************************************** */

    // @abi table pets i64
    struct st_pets {
        uuid id;
        account_name owner;
        string name;
        uint8_t health;
        uint8_t hunger;
        uint32_t created_at;
        uint32_t last_fed_at;

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
        uint8_t initial_health = 100;
        uint8_t initial_hunger = 100;
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



};

EOSIO_ABI(pet, (createpet))
