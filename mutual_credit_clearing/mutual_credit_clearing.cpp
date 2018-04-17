#include <mutual_credit_clearing.hpp>

void mutual_credit_clearing::hi(account_name user) {
    require_auth(user);
    print("hi ", name{user});
}

void mutual_credit_clearing::bye(string msg) {
    print("bye bye bud!!! ", msg.c_str());
}

void mutual_credit_clearing::create(account_name issuer, asset maximum_supply) {
    require_auth(issuer);

    auto sym = maximum_supply.symbol;
    eosio_assert(sym.is_valid(), "invalid symbol name");

    _stats stats_table( _self, sym.name() );
    auto existing = stats_table.find( sym.name() );
    eosio_assert( existing == stats_table.end(), "token with symbol already exists" );

    stats_table.emplace( _self, [&]( auto& s ) {
        s.supply.symbol = maximum_supply.symbol;
        s.max_supply    = maximum_supply;
        s.issuer        = issuer;
    });

    print("created! ");
}


EOSIO_ABI(mutual_credit_clearing, (hi)(bye)(create))