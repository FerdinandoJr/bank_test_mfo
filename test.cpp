#include <assert.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <iostream>
#include "bank.hpp"

using json = nlohmann::json;
using namespace std;

enum class Action {
  Init,
  Deposit,
  Withdraw,
  Transfer,
  BuyInvestment,
  SellInvestment,
  Unknown
};

Action stringToAction(const std::string &actionStr) {
  static const std::unordered_map<std::string, Action> actionMap = {
      {"init", Action::Init},
      {"deposit_action", Action::Deposit},
      {"withdraw_action", Action::Withdraw},
      {"transfer_action", Action::Transfer},
      {"buy_investment_action", Action::BuyInvestment},
      {"sell_investment_action", Action::SellInvestment}
  };

  auto it = actionMap.find(actionStr);
  if (it != actionMap.end()) {
    return it->second;
  } else {
    return Action::Unknown;
  }
}

int int_from_json(json j) {
  string s = j["#bigint"];
  return stoi(s);
}

map<string, int> balances_from_json(json j) {
  map<string, int> m;
  for (auto it : j["#map"]) {
    m[it[0]] = int_from_json(it[1]);
  }
  return m;
}

map<int, Investment> investments_from_json(json j) {
  map<int, Investment> m;
  for (auto it : j["#map"]) {
    m[int_from_json(it[0])] = {.owner = it[1]["owner"],
                               .amount = int_from_json(it[1]["amount"])};
  }
  return m;
}

BankState bank_state_from_json(json state) {
  map<string, int> balances = balances_from_json(state["balances"]);
  map<int, Investment> investments =
      investments_from_json(state["investments"]);
  int next_id = int_from_json(state["next_id"]);
  return {.balances = balances, .investments = investments, .next_id = next_id};
}

int main() {  
  for (int i = 0; i < 10000; i++) {
    cout << endl << "--------------- Trace # ---------------" << i << endl;
    std::ifstream f("traces/out" + to_string(i) + ".itf.json");
    json data = json::parse(f);

    // Estado inicial: começamos do mesmo estado incial do trace
    BankState bank_state = bank_state_from_json(data["states"][0]["bank_state"]);
  
    auto states = data["states"];
    for (auto state : states) {
      string action = state["action_taken"];
      json nondet_picks = state["nondet_picks"];

      string error = "";

      cout << endl;
      // Próxima transição
      switch (stringToAction(action)) {
        case Action::Init: {          
          cout << "initializing" << endl;
          break;
        }
        case Action::Deposit: {
          string depositor = nondet_picks["depositor"]["value"];
          int amount = int_from_json(nondet_picks["amount"]["value"]);

          error = deposit(bank_state, depositor, amount);              
          cout << "[DEPOSITO] para " << depositor << " no valor de " << amount << endl;            
          break;
        } 
        case Action::Withdraw: {
          string withdrawer = nondet_picks["withdrawer"]["value"];
          int amount = int_from_json(nondet_picks["amount"]["value"]);

          error = withdraw(bank_state, withdrawer, amount);      
          cout << "[RETIRADA] de " << withdrawer << " no valor de " << amount << endl;        
          break;
        } 
        case Action::Transfer: {
          string sender = nondet_picks["sender"]["value"];
          string receiver = nondet_picks["receiver"]["value"];
          int amount = int_from_json(nondet_picks["amount"]["value"]);

          error = transfer(bank_state, sender, receiver, amount);        
          cout << "[TRANSFERENCIA] de " << sender << " para "<< receiver <<" no valor de " << amount << endl;
          break;
        } 
        case Action::BuyInvestment: {        
          string buyer = nondet_picks["buyer"]["value"];
          int amount = int_from_json(nondet_picks["amount"]["value"]);

          error = buy_investment(bank_state, buyer, amount);
          cout << "[COMPRA DE INVESTIMENTO] feito por " << buyer << " no valor de " << amount << endl;
          break;
        } 
        case Action::SellInvestment: {        
          string seller = nondet_picks["seller"]["value"];
          int investment_id = int_from_json(nondet_picks["id"]["value"]);
          
          error = sell_investment(bank_state, seller, investment_id);
          cout << "[VENDA DE INVESTIMENTO] feito por " << seller << "(id "+ to_string(investment_id) +")"<< endl;
          break;
        }
        default: {
          cout << "TODO: fazer a conexao para as outras acoes. Acao: " << action << endl;
          error = "";
          break;
        }
      }

        BankState expected_bank_state = bank_state_from_json(state["bank_state"]);
        auto balances = expected_bank_state.balances;

        cout << "Saldo Esperado" << endl;
        for (const auto& balance : balances) {
            string key = balance.first;  // Chave (nome)
            int value = balance.second;  // Valor associado
            cout << key << ", Saldo: " << value << "\n";
        }

        auto balances_state = bank_state.balances;    

        cout << endl << "Saldo Atual" << endl;
        for (const auto& balance : balances_state) {
          string key = balance.first;  // Chave (nome)
          int value = balance.second;  // Valor associado
          cout << key << ", Saldo: " << value << "\n";
        }


        string expected_error = string(state["error"]["tag"]).compare("Some") == 0
          ? state["error"]["value"]
          : "";

      if ((error != expected_error) || (!error.empty() && expected_error.empty()) || (error.empty() && !expected_error.empty()) ) {
        cout << endl << "[ERROR]" << endl;
        cout << "Informado: " << error;
        cout << " != Esperado: " << expected_error << endl; 
        return 1;
      }
      
    }
  }
  return 0;
}
