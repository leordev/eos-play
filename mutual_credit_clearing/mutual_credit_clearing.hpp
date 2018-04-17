#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <string>

using namespace eosio;

using std::string;

class mutual_credit_clearing : contract {
    using contract::contract;

    public:

        mutual_credit_clearing(account_name self) : contract(self) {}


        void hi(account_name user);
        void bye(string msg);
        void create(account_name issuer, asset maximum_supply);

    private:

        // @abi table
        struct stats {
            asset supply;
            asset max_supply;
            account_name issuer;

            uint64_t primary_key() const { return supply.symbol.name(); }
            EOSLIB_SERIALIZE(stats, (supply)(max_supply)(issuer))
        };

        typedef eosio::multi_index<N(stats), stats> _stats;

};