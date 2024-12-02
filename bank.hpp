#include <map>
#include <string>
using namespace std;

struct Investment {
  string owner;
  int amount;
};

struct BankState {
  map<string, int> balances;
  map<int, Investment> investments;
  int next_id;
};


bool client_exists(BankState &bank_state, const string &client) {
    return bank_state.balances.find(client) != bank_state.balances.end();
}

string deposit(BankState &bank_state, string depositor, int amount) {
  if (!client_exists(bank_state, depositor)) {
      return "DEPOSITOR_NOT_FOUND";
  }
  
  if (amount <= 0) {
    return "Quantidade precisa ser maior que zero ("+ to_string(amount) +")";
  }

  bank_state.balances[depositor] += amount;
  return "";
}

string withdraw(BankState &bank_state, string withdrawer, int amount) {
    if (!client_exists(bank_state, withdrawer)) {
      return "WITHDRAWER_NOT_FOUND";
    }

    if (amount <= 0) {
      return "Quantidade precisa ser maior que zero ("+ to_string(amount) +")";
    }

    if (bank_state.balances[withdrawer] < amount) {
      return "Saldo insuficiente para retirada de " + withdrawer;
    }
    
    bank_state.balances[withdrawer] -= amount;
    return "";
}

string transfer(BankState &bank_state, string sender, string receiver, int amount) {
  if (!client_exists(bank_state, sender)) {
    return "SENDER_NOT_FOUND";
  }

  if (!client_exists(bank_state, receiver)) {
    return "RECEIVER_NOT_FOUND";
  }
    
  if (amount <= 0) {
    return "Quantidade precisa ser maior que zero ("+ to_string(amount) +")";
  }  

  if (bank_state.balances[sender] < amount) {
    return "Saldo insuficiente para transferencia de " + sender + " (" + to_string(bank_state.balances[sender])+")";
  }

  if (sender == receiver) {
    return "";
  }  

  bank_state.balances[sender] -= amount;
  bank_state.balances[receiver] += amount;
  return "";
}

string buy_investment(BankState &bank_state, string buyer, int amount) {
    if (!client_exists(bank_state, buyer)) {
      return "BUYER_NOT_FOUND";
    }
    
    if (bank_state.balances[buyer] < amount) {
        return "Saldo insuficiente para compra";
    }

    if (amount <= 0) {
      return "Quantidade precisa ser maior que zero ("+ to_string(amount) +")";
    }

    bank_state.balances[buyer] -= amount;
    bank_state.investments[bank_state.next_id] = {buyer, amount};
    bank_state.next_id++;
    return "";
}


string sell_investment(BankState &bank_state, string seller, int investment_id) {
    if (!client_exists(bank_state, seller)) {
      return "SELLER_NOT_FOUND";
    }

    if (bank_state.investments.find(investment_id) != bank_state.investments.end()) {
        return "Investimento com id "+ to_string(investment_id) +" nao encontrado";
    }

    if (bank_state.investments[investment_id].owner != seller) {
        return "Investimento nao pertence ao vendedor";
    }

    bank_state.balances[seller] += bank_state.investments[investment_id].amount;
    // bank_state.investments.erase(investment_id);
    return "";
}