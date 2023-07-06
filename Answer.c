#ifdef _TELL_ME_THE_ANSWER
#include "Question.h"

Account* Account_Create()
{
	Account* account = malloc(sizeof(Account));
	if (account == NULL)
		return NULL;

	account->balance = 0;
	account->mutex = CreateMutex(NULL, FALSE, NULL);
	return account;
}

void Account_Free(Account* account)
{
	CloseHandle(account->mutex);
	free(account);
}

double Account_GetBalance(Account* account) {
	return account ? account->balance : 0;
}

DWORD WINAPI transactionThread(LPVOID param) {

	SingleAccountTransaction* transaction = (SingleAccountTransaction*)param;

	WaitForSingleObject(transaction->account->mutex, INFINITE);

	transaction->account->balance += transaction->amount;
	printf("Person %d %s %.2f, balance: %.2f\n",
		transaction->personId, transaction->amount < 0 ? "withdrew" : "deposit", transaction->amount, transaction->account->balance);

	ReleaseMutex(transaction->account->mutex);

	return 0;
}

Bank* Bank_Create() {
	Bank* bank = malloc(sizeof(Bank));
	bank->accounts = NULL;
	bank->num_accounts = 0;
	return bank;
}

void Bank_Free(Bank* bank) {
	for (int i = 0; i < bank->num_accounts; i++) {
		CloseHandle(bank->accounts[i]->mutex);
		free(bank->accounts[i]);
	}
	free(bank->accounts);
	free(bank);
}


// Returns the id of the new account:
// Returns the id of the new account:
int Bank_AddAccount(Bank* bank) {
	bank->accounts = realloc(bank->accounts, sizeof(Account*) * (bank->num_accounts + 1));
	bank->accounts[bank->num_accounts] = Account_Create();
	bank->num_accounts++;

	return bank->num_accounts - 1;
}

double Bank_GetBalance(Bank* bank, int accountIdx) {
	if (0 <= accountIdx && accountIdx < bank->num_accounts)
		return bank->accounts[accountIdx]->balance;

	return 0;
}

// Move money from one account to another. Only positive amount can be transferred.
// returns true if transaction was conducted. false otherwise.
// Function must be thread safe!
bool Bank_DualTransaction(TwoAccountsTransaction* transaction) {
	if (transaction->amount <= 0) {
		printf("Invalid transaction amount %f.\n", transaction->amount);
		return false;
	}
	if (transaction->srcAccountIdx == transaction->dstAccountIdx)
	{
		printf("Invalid transaction from account %d to itself.\n", transaction->dstAccountIdx);
		return false;
	}

	Account* src = transaction->bank->accounts[transaction->srcAccountIdx];
	Account* dst = transaction->bank->accounts[transaction->dstAccountIdx];

	WaitForSingleObject(src->mutex, INFINITE);
	WaitForSingleObject(dst->mutex, INFINITE);

	src->balance -= transaction->amount;
	dst->balance += transaction->amount;

	printf("Person %d moved %.2f, from account %d to account %d. new balance: %.2f\n",
		transaction->personId, transaction->amount,
		transaction->srcAccountIdx, transaction->dstAccountIdx, dst->balance);

	ReleaseMutex(dst->mutex);
	ReleaseMutex(src->mutex);
	return true;
}
#endif