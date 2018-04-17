#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/singleton.hpp>

using namespace eosio;

class deep : contract {
    using contract::contract;

public:
    deep( account_name self ) : contract(self){}

    void setapp( account_name application ) {
        require_auth(_self);
        require_auth(application);

        eosio_assert(!configs::exists(), "Configuration already exists");

        configs::set(config{application});
    }

    void setacc ( account_name user ) {
        require_auth(user);
        require_auth(configs::get().application);
        print("Hello acc");
    }

private:

    typedef uint64_t id;

    struct config {
        account_name application;

        EOSLIB_SERIALIZE( config, (application) )
    };

    typedef singleton<N(deep), N(config), N(deep), config> configs;

};

EOSIO_ABI(deep, (setapp) (setacc) )