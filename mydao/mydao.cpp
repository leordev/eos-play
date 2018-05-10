#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/singleton.hpp>

using namespace eosio;

using std::string;
using std::vector;

using boost::container::flat_map;

/**
 * We are playing with contracts from ethereum
 * this example has horrible performance compared to
 * default eosio.token because we are utilizing vector
 * just to simulate what eth has in their contracts
 * The correct structure would be create another table
 * for balance and persist the token on the account
 * owner scope
 * @author Leo Ribeiro
 */
class mydao : public eosio::contract {
public:
    mydao( account_name self)
            :eosio::contract(self),
             daos(_self,_self),
             last_id(_self, _self)
            {}

    void hi(account_name user) {
        require_recipient(user);
        print("Hello, ", name{user});
    }

    void createdao(account_name owner, uint16_t min_quorum, uint16_t debating_period_minutes,
                   uint16_t majority_margin) {

        require_auth(owner);

        daos.emplace(owner, [&](auto& r) {
            r.id = next_id();
            r.owner = owner;
            r.min_quorum = min_quorum;
            r.debating_period_minutes = debating_period_minutes;
            r.majority_margin = majority_margin;
            r.members = {{owner}};
        });

    }

private:

    typedef uint64_t uuid;

    struct vote {
        bool in_support;
        string justification;
    };

    struct proposal {
        account_name recipient;
        asset amount;
        string description;
        uint64_t min_execution_date;
        bool executed;
        bool proposal_passed;
        uint8_t number_of_votes;
        uint8_t current_result;
        string proposal_hash;

        flat_map<account_name, vote> votes;
    };

    // @abi table dao i64
    struct dao {
        uuid id;
        account_name owner;
        uint16_t min_quorum;
        uint16_t debating_period_minutes;
        uint16_t majority_margin;

        vector<account_name> members;

        flat_map<uuid, proposal> proposals;

        uuid primary_key() const { return id; }
    };

    typedef eosio::multi_index<N(dao), dao> tb_dao;

    tb_dao daos;

    typedef singleton<N(last_id), uuid> last_id_singleton;
    last_id_singleton last_id;

    uuid next_id(){
        uuid lid = last_id.exists() ? last_id.get()+1 : 1000000000;
        last_id.set(lid, _self);
        return lid;
    }

};

EOSIO_ABI(mydao, (hi)(createdao))
