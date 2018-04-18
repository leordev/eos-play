#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <string>

using namespace eosio;

using std::string;

class mutual_credit_clearing : contract {
    using contract::contract;

    public:

        mutual_credit_clearing(account_name self) : contract(self) {}

        void create(account_name issuer, asset maximum_supply);

        void issue(account_name to, asset quantity, string memo);

        void transfer(account_name from, account_name to, asset quantity, string memo);

        // tests
        void hi(account_name user);
        void bye(string msg);

    private:

        // tables

        struct stats {
            asset supply;
            asset max_supply;
            account_name issuer;

            uint64_t primary_key() const { return supply.symbol.name(); }
            EOSLIB_SERIALIZE(stats, (supply)(max_supply)(issuer))
        };

        typedef eosio::multi_index<N(stats), stats> _stats;

        struct account {
            asset balance;

            uint64_t primary_key() const { return balance.symbol; }
        };

        typedef multi_index<N(accounts), account> accounts;

        // methods

        void add_balance( account_name owner, asset value, const stats& st, account_name payer );
        void sub_balance( account_name owner, asset value, const stats& st);

};