#ifndef COIN
#define COIN

#include"util.h"

/*-- Coin.h ---------------------------------------------------------------

  This header file defines the Coin class for The Blockchain.
  Basic operations are:
	 Constructor
  This class has will be responsible for the following
	 - Consensus: Delegated Proof of Stake

	 - Token supply:
		Handling inflation, deflation, or any mechanisms affecting the supply of the coin.
		Total Supply will be inflatory: We will introduce methods like burning || halving etcetera!
		we will host ICO not airdrops!
		Will Employ token Vesting (to which extent?)
		staking ofc (D-POS Structure!) miners must stake portion (how big?)
		inflationary supply (percentage? timeframe?)!

	 - Coin Data:
		name: The name of the coin (e.g., "Bitcoin", "MyCoin").
		symbol: The ticker symbol for the coin (e.g., BTC, MYC).
		total_supply: The maximum supply of coins allowed in circulation.
		current_supply: The current number of coins in circulation.
		reward: The reward given for mining a new block.
		block_time: The time interval between blocks (block generation time).
		block_size_limit: Maximum size of each block.
		transaction_fee: The fee required to send a transaction.
		liquidity Pools

	 - Governance D-POS:
		Delegates Must Be Servers
		Longer you stake Staking = Higher voting power
		interest Rates Stability Fees
		Must ensure stable government participation (Structure Requires Minimal (AI))
		Nothing can be done without voting power approval!
		voting power determines future protocol changes, funding decisions, and more.
		Certain Amount of outcry can lead to an automatic vote!

  Note:
  USD in CircSupply = 7,594,103,000,000 (Quadrillion)
  AI Will be a Major Future Focus in this class like all other classes
-------------------------------------------------------------------------*/

class Coin
{
private:
	const char* name = "0017";
	const char* symbol = "0017";
	double totalSupply;
	double circSupply;
	float reward;
	unsigned short block_time;
	double txFee;
	uint32_t blkSzLimit;

public:
	Coin();

	/* Getters and setters for Coin */
	const char* getName();
	const char* getSymbol();
	double getTotalSupply();
	void setTotalSupply(double ts);
	double getCircSupply();
	void setCircSupply(double cs);
	float getReward();
	void setReward(float rw);
	unsigned short getBlockTime();
	void setBlockTime(unsigned short bt);
	double getTxFee();
	void setTxFee(double txf);
	uint32_t getBlkSzLimit();
	void setBlkSzLimit(uint32_t bsl);

	//Voting methods


	// General Methods
	//void updateTotalSupply(); // Updates Circulating supply every time a block is added!
	//void updateCircSupply(); // Updates total supply every time a block is added!
	//transactions grantRewards(); // Grants Rewards to block miners!
};

#endif
