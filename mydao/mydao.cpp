#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/singleton.hpp>

using namespace eosio;

using std::string;
using std::vector;

using boost::container::flat_map;

typedef uint64_t uuid;

/**
 * We are playing with contracts from ethereum
 * to compare how is a solidity implementation
 * vs awesome eos :)
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

    void createprop(uuid dao_id, account_name author, account_name recipient, asset amount,
                    string description, uint64_t min_execution_date) {
        require_auth(author);

        auto dao = daos.find(dao_id);

        // validate member
        const auto itr_member = find(begin(dao->members), end(dao->members), author);
        eosio_assert(itr_member != end(dao->members), "author not in dao");

        daos.modify(dao, author, [&](auto &r) {
            proposal p{recipient, amount, description, min_execution_date, false, false, 0, "xxxx"};
            r.proposals.emplace(next_id(), p);
        });
    }

private:

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

EOSIO_ABI(mydao, (hi)(createdao)(createprop))
