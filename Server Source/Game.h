#ifndef _GAME_H
#define _GAME_H

#include "stdafx.h"

// Number of cards a player holds
#define NUM_HOLD_CARDS 2

// Starting time of time bank
#define PLAYER_TIMEBANK_DEFAULT 5000

// Max duration a timebank can be
#define PLAYER_MAX_TIMEBANK 40000

// How much time you get to make a decision (in ms)
#define MAX_DECISION_TIME 15000

struct Player
{
	Player(std::string n, unsigned int a, PENNY stack) :
		name(n), accountId(a), penStack(stack), penBet(0), dTimeBank(PLAYER_TIMEBANK_DEFAULT), dDecisionTime(MAX_DECISION_TIME), bPostedBB(false), bNeedToAct(false), penStackBeforeHand(stack) {}
	
	bool bNeedToAct;
	bool bPostedBB;
	std::string name;
	unsigned int accountId; // Also GUID

	PENNY penStack;
	PENNY penBet;
	PENNY penStackBeforeHand;

	DWORD dTimeBank;
	DWORD dDecisionTime;

	Card cards[NUM_HOLD_CARDS];

	// In text format, which choices he can make.
	// "Bet", "Fold", "Check", etc.
	std::vector<std::string> vDecisionChoices;
};

// Number of seats
#define NUM_SEATS 6

struct Seat
{
	Seat() :
		pPlayer(0) {}

	Player* pPlayer;
};

// Number of cards dealt to the board
#define NUM_CARDS_BOARD 5

// Whether or not a player is connected (a) being accountId
#define PLAYER_CONNECTED(a) (g_gameFloor.GetSession(a) != 0)

// Stages of a hand
enum eHandStages
{
	NOSTAGE,
	NOSTAGE_DUMMY,
	DEALING,
	PREFLOP,
	FLOP,
	TURN,
	RIVER,
	SHOWDOWN,
	HANDISOVER
};

// Min & Max buy ins
#define MIN_BUYIN (m_penBigBlind * 40)
#define MAX_BUYIN (m_penBigBlind * 250)

// Penny to float
#define PENNY_F(a) (float(a) / 100.0f)

class Game
{
	public:
		Game(PENNY bigBlind = 2, PENNY smallBlind = 1);

		void Update(unsigned int uiDiff);
		void ProcessPlayerDecision(unsigned int accountId, std::string decision, float fAmount = 0);
		void BroadcastF(const char *fmt, ...);
		void SendF(unsigned int accountId, const char *fmt, ...);
		void AffectBankroll(Player* pPlayer, PENNY penNumPennies);

		bool AddPlayer(std::string name, unsigned int accountId, PENNY stack);
		bool PlayerAtTable(unsigned int accountId) const;

		bool handInProgress() const 
		{
			return m_uiCurrentStage >= PREFLOP;
		}
		
		std::vector<std::string> GetAvailableDecisions(unsigned int accountId) const;

		Player* getPlayer(unsigned int accountId) const
		{
			for (uint8 i = 0; i < NUM_SEATS; ++i)
				if (m_seats[i].pPlayer && m_seats[i].pPlayer->accountId == accountId)
					return m_seats[i].pPlayer;
			return 0;
		}

		// Networking
		//

		unsigned int PackOpcode(char* buffer, uint8 opcode);
		unsigned int PackAllTableInformation(char* buffer);

	private:
		void EndHand();
		void ResetGame();
		void DealCards();
		void NextSeatToAct();
		void PlayerFolds(Player* pPlayer);
		void Progress();
		void ProgressToFlop();
		void ProgressToTurn();
		void ProgressToRiver();
		void ProgressToShowdown();
		void ScoopUpBets();
		void RotateBlinds();
		void AwardWinners();
		void AwardPot(std::vector<Player*> vWinners);		
		void PopulateAvailableDecisions();
		void CheckHandOver();
		void RefreshNeedToAct(Player* pExcept = 0);
		void RemovePlayer(unsigned int accountId);
		void BroadcastPlayerHand(Player* pHand);

		std::vector<std::string> DeducePossibleDecisions(Player* pPlayer);
		
		PENNY LargestBetOutThere();
		PENNY ExtractBet(Player* pPlayer, PENNY penAmount) const;

		bool playerHasHolding(Player* pPlayer) const { return pPlayer->cards[0].uiSuit > 0; }
		bool EnoughPlayersForGame() const;
		
		bool someoneNeedsToAct() const
		{
			for (uint8 i = 0; i < NUM_SEATS; ++i)
				if (m_seats[i].pPlayer && m_seats[i].pPlayer->bNeedToAct)
					return true;
			return false;
		}

		void NormalizeSeatNumber(int& seat);
		
		uint8 NumNeedToAct() const;
		uint8 NumPlayers(bool bPostedPlayers = false) const;
		uint8 NumPlayersWithHolding() const;
		uint8 NumPlayersWithStack(bool bDealtPlayersOnly = false, bool bHoldingOnly = false) const;

		PENNY m_penPot;
		PENNY m_penBigBlind;
		PENNY m_penSmallBlind;

		bool m_bSkipToShowdown;
		bool m_bFirstBigBlind;
		
		uint8 m_uiCurrentStage;
		uint8 m_uiSeatToAct;
		uint8 m_uiDealerSeat;
		uint8 m_uiBigBlindSeat;
		uint8 m_uiSmallBlindSeat;

		unsigned int m_uiStageTimer;

		// Kind of pointless, just keeps track of how many hands have been played since last restart
		unsigned int m_uiUniqueHandCount;
		
		char m_gameBuffer[MAX_BUFFER_SIZE];

		Seat m_seats[NUM_SEATS];

		CardDeck m_cardDeck;
		
		std::vector<Card> m_board;
};

#endif