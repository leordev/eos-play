#include <mutual_credit_clearing.hpp>

void mutual_credit_clearing::hi(account_name user) {
    require_auth(user);
    print("hi ", name{user});
}

void mutual_credit_clearing::bye(string msg) {
    print("bye bye bud ", msg.c_str());
}

void mutual_credit_clearing::create(account_name issuer, asset maximum_supply) {
    require_auth(_self); // only application can create a token

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

void mutual_credit_clearing::issue( account_name to, asset quantity, string memo ) {

    auto sym = quantity.symbol;

    _stats stats_table( _self, sym.name() );
    const auto& st = stats_table.get( sym.name() );

    require_auth( st.issuer ); // only issuer can issue

    print("issuer >>", name{st.issuer});

    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must issue positive quantity" );

    stats_table.modify( st, 0, [&](auto& s) {
       s.supply.amount += quantity.amount;
    });

    add_balance(st.issuer, quantity, st, st.issuer);

    if(to != st.issuer) {
        dispatch_inline( permission_level{st.issuer,N(active)}, _self, N(transfer),
                         &mutual_credit_clearing::transfer,
                         {st.issuer, to, quantity, memo} );
    }

}

void mutual_credit_clearing::transfer( account_name from,
                                       account_name to,
                                       asset quantity,
                                       string memo ) {

    print( "transfer from ", name{from}, " to ", name{to}, " - MEMO: ", memo.c_str() );
    require_auth( from );

    require_recipient( from );
    require_recipient( to );

    auto sym = quantity.symbol;
    _stats stats_table( _self, sym.name() );
    const auto& st = stats_table.get( sym.name() );

    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must transfer positive quantity" );

    sub_balance( from, quantity, st );
    add_balance( to, quantity, st, from );

}

void mutual_credit_clearing::add_balance(account_name owner,
                                         asset value,
                                         const stats& st,
                                         account_name payer) {
    accounts to_acc(_self, owner);

    auto to = to_acc.find(value.symbol);

    if (to == to_acc.end()) {
        to_acc.emplace(payer, [&](auto& a) {
            a.balance = value;
        });
    } else {
        to_acc.modify( to, 0, [&](auto& a) {
           a.balance.amount += value.amount;
        });
    }
}

void mutual_credit_clearing::sub_balance(account_name owner,
                                         asset value,
                                         const stats& st) {
    accounts from_acc(_self, owner);

    auto from = from_acc.find(value.symbol);

    if (from == from_acc.end()) {
        from_acc.emplace(owner, [&](auto& a) {
            a.balance = value * -1;
        });
    } else {
        from_acc.modify( from, 0, [&](auto& a) {
            a.balance.amount -= value.amount;
        });
    }
}


EOSIO_ABI(mutual_credit_clearing, (hi)(bye)(create)(issue)(transfer))