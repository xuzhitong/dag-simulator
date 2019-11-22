/**
 * This file contains all the necessary enumerations and structs used throughout the simulation.
 * It also defines 3 very important classed; the Block, Chunk and Blockchain.
 */


#ifndef BITCOIN_H
#define BITCOIN_H

#include <vector>
#include <map>
#include "ns3/address.h"
#include <algorithm>
#include <array>
namespace ns3 {
	
/**
 * The bitcoin message types that have been implemented.
 */
enum Messages
{
  INV,              //0
  GET_HEADERS,      //1
  HEADERS,          //2
  GET_BLOCKS,       //3
  BLOCK,            //4
  GET_DATA,         //5
  NO_MESSAGE,       //6
  EXT_INV,          //7
  EXT_GET_HEADERS,  //8
  EXT_HEADERS,      //9
  EXT_GET_BLOCKS,   //10
  CHUNK,            //11
  EXT_GET_DATA,     //12
};


/**
 * The bitcoin miner types that have been implemented. The first one is the normal miner (default), the last 3 are used to simulate different attacks.
 */
enum MinerType
{
  NORMAL_MINER,                //DEFAULT
  SIMPLE_ATTACKER,
  SELFISH_MINER,
  SELFISH_MINER_TRIALS
};


/**
 * The different block broadcast types that the miner uses to adventize newly mined blocks.
 */
enum BlockBroadcastType
{
  STANDARD,                    //DEFAULT
  UNSOLICITED,
  RELAY_NETWORK,
  UNSOLICITED_RELAY_NETWORK
};


/**
 * The protocol that the nodes use to advertise new blocks. The STANDARD_PROTOCOL (default) uses the standard INV messages for advertising,
 * whereas the SENDHEADERS uses HEADERS messages to advertise new blocks.
 */
enum ProtocolType
{
  STANDARD_PROTOCOL,           //DEFAULT
  SENDHEADERS
};


/** 
 * The different cryptocurrency networks that the simulation supports.
 */
enum Cryptocurrency
{
  BITCOIN,                     //DEFAULT
  LITECOIN,
  DOGECOIN
};


/** 
 * The geographical regions used in the simulation. OTHER was only used for debugging reasons.
 */
enum BitcoinRegion
{
  NORTH_AMERICA,    //0
  EUROPE,           //1
  SOUTH_AMERICA,    //2
  ASIA_PACIFIC,     //3
  JAPAN,            //4
  AUSTRALIA,        //5
  OTHER             //6
};


/**
 * The struct used for collecting node statistics.
 */
typedef struct {
  int      nodeId;
  double   meanBlockReceiveTime;
  double   meanBlockPropagationTime;
  double   meanBlockSize;
  int      totalBlocks;
  int      staleBlocks;
  int      miner;	                         //0->node, 1->miner
  int      minerGeneratedBlocks;
  double   minerAverageBlockGenInterval;
  double   minerAverageBlockSize;
  double   hashRate;
  int      attackSuccess;                    //0->fail, 1->success
  long     invReceivedBytes;
  long     invSentBytes;
  long     getHeadersReceivedBytes;
  long     getHeadersSentBytes;
  long     headersReceivedBytes;
  long     headersSentBytes;
  long     getDataReceivedBytes;
  long     getDataSentBytes;
  long     blockReceivedBytes;
  long     blockSentBytes;
  long     extInvReceivedBytes;
  long     extInvSentBytes;
  long     extGetHeadersReceivedBytes;
  long     extGetHeadersSentBytes;
  long     extHeadersReceivedBytes;
  long     extHeadersSentBytes;
  long     extGetDataReceivedBytes;
  long     extGetDataSentBytes;
  long     chunkReceivedBytes;
  long     chunkSentBytes;
  int      longestFork;
  int      blocksInForks;
  int      connections;
  long     blockTimeouts;
  long     chunkTimeouts;
  int      minedBlocksInMainChain;
} nodeStatistics;
typedef struct {
  double downloadSpeed;
  double uploadSpeed;
} nodeInternetSpeeds;


/**
 * Fuctions used to convert enumeration values to the corresponding strings.
 */
const char* getMessageName(enum Messages m);
const char* getMinerType(enum MinerType m);
const char* getBlockBroadcastType(enum BlockBroadcastType m);
const char* getProtocolType(enum ProtocolType m);
const char* getBitcoinRegion(enum BitcoinRegion m);
const char* getCryptocurrency(enum Cryptocurrency m);
enum BitcoinRegion getBitcoinEnum(uint32_t n);
/* BEGIN: Added by xuzt, 2019/5/7   DAG simulator support */
#define DAG_MAX_PARENTS_COUNT 2
#define MAJORITY_WITNESSES_COUNT 5
#define WITNESS_COUNT 6

/* END:   Added by xuzt, 2019/5/7 */

class Block
{
public:
  Block (int blockHeight, int minerId, int parentBlockMinerId = 0, int blockSizeBytes = 0, 
         double timeCreated = 0, double timeReceived = 0, Ipv4Address receivedFromIpv4 = Ipv4Address("0.0.0.0"));
  Block ();
  Block (const Block &blockSource);  // Copy constructor
  virtual ~Block (void);
 
  int GetBlockHeight (void) const;
  void SetBlockHeight (int blockHeight);
  
  int GetMinerId (void) const;
  void SetMinerId (int minerId);
  
  int GetParentBlockMinerId (void) const;
  void SetParentBlockMinerId (int parentBlockMinerId);
  
  int GetBlockSizeBytes (void) const;
  void SetBlockSizeBytes (int blockSizeBytes);
  
  double GetTimeCreated (void) const;
  double GetTimeReceived (void) const;

  Ipv4Address GetReceivedFromIpv4 (void) const;
  void SetReceivedFromIpv4 (Ipv4Address receivedFromIpv4);

/* BEGIN: Added by xuzt, 2019/4/19   DAG simulator support */
  std::string GetBlockHash(void) const;
  std::vector<std::string> GetDagParents(void) const;
  void SetDagParents(std::vector<std::string> blocks);
  Block GetBestParent(std::vector<Block> pblocks);
  void SetBestParent(std::string bpHash);
  /* END:   Added by xuzt, 2019/4/19 */
    
  /**
   * Checks if the block provided as the argument is the parent of this block object
   */
  bool IsParent (const Block &block) const; 

  /**
   * Checks if the block provided as the argument is a child of this block object
   */
  bool IsChild (const Block &block) const; 
  
  Block& operator= (const Block &blockSource); //Assignment Constructor
  
  friend bool operator== (const Block &block1, const Block &block2);
  friend std::ostream& operator<< (std::ostream &out, const Block &block);

  /* BEGIN: Added by xuzt, 2019/4/24   DAG simulator support */
  void SetChildren(const Block& children);
  bool IsFree() const;


  /* END:   Added by xuzt, 2019/4/24 */
  
protected:	
  std::string   m_blockHash;
  int           m_blockHeight;                // The height of the block
  int           m_minerId;                    // The id of the miner which mined this block
  int           m_parentBlockMinerId;         // The id of the miner which mined the parent of this block
  int           m_blockSizeBytes;             // The size of the block in bytes
  double        m_timeCreated;                // The time the block was created
  double        m_timeReceived;               // The time the block was received from the node
  Ipv4Address   m_receivedFromIpv4;           // The Ipv4 of the node which sent the block to the receiving node
  /* BEGIN: Added by xuzt, 2019/4/24   DAG simulator support */
public:
  int           m_mci;
  int           m_bpIncludedMci;
  int           m_earliestBpIncludedMci;
  bool          m_isOnMainChain;
  int           m_lastStableMci;
  int           m_witnessLevel;
  std::vector<std::string> m_DagParents;
  std::vector<std::string> m_DagChildrens;
  std::string   m_bestParent;
  double        m_timeStable;
  //bool          m_isStable;
  /* END:   Added by xuzt, 2019/4/24 */
};

class BitcoinChunk : public Block
{
public:
  BitcoinChunk (int blockHeight, int minerId, int chunkId, int parentBlockMinerId = 0, int blockSizeBytes = 0, 
                double timeCreated = 0, double timeReceived = 0, Ipv4Address receivedFromIpv4 = Ipv4Address("0.0.0.0"));
  BitcoinChunk ();
  BitcoinChunk (const BitcoinChunk &chunkSource);  // Copy constructor
  virtual ~BitcoinChunk (void);
 
  int GetChunkId (void) const;
  void SetChunkId (int minerId);
  
  BitcoinChunk& operator= (const BitcoinChunk &chunkSource); //Assignment Constructor
  
  friend bool operator== (const BitcoinChunk &chunk, const BitcoinChunk &chunk2);
  friend bool operator< (const BitcoinChunk &chunk, const BitcoinChunk &chunk2);
  friend std::ostream& operator<< (std::ostream &out, const BitcoinChunk &chunk);
  
protected:	
  int           m_chunkId;

};

class Blockchain
{
public:
  Blockchain(void);
  virtual ~Blockchain (void);

  int GetNoStaleBlocks (void) const;
  
  int GetNoOrphans (void) const;
  
  int GetTotalBlocks (void) const;
  
  int GetBlockchainHeight (void) const;

  /**
   * Check if the block has been included in the blockchain.
   */
  bool HasBlock (const Block &newBlock) const;
  bool HasBlock (int height, int minerId) const;
  
  /**
   * Return the block with the specified height and minerId.
   * Should be called after HasBlock() to make sure that the block exists.
   * Returns the orphan blocks too.
   */
  Block ReturnBlock(int height, int minerId);  
/* BEGIN: Added by xuzt, 2019/4/26   DAG simulator support */
  Block ReturnBlock(std::string blockHash);
  /* END:   Added by xuzt, 2019/4/26 */
  /**
   * Check if the block is an orphan.
   */
  bool IsOrphan (const Block &newBlock) const;
  bool IsOrphan (int height, int minerId) const;

  /**
   * Gets a pointer to the block.
   */
  const Block* GetBlockPointer (const Block &newBlock) const;

  /**
   * Gets the children of a block that are not orphans.
   */
  const std::vector<const Block *> GetChildrenPointers (const Block &block);  
  
  /**
   * Gets the children of a newBlock that used to be orphans before receiving the newBlock.
   */
  const std::vector<const Block *> GetOrphanChildrenPointers (const Block &newBlock);  

  /**
   * Gets the parent of a block
   */
  const Block* GetParent (const Block &block);  //Get the parent of newBlock

  /**
   * Gets the current top block. If there are two block with the same height (siblings), returns the one received first.
   */
  const Block* GetCurrentTopBlock (void) const;

  /**
   * Adds a new block in the blockchain.
   */
  void AddBlock (const Block& newBlock);

  /**
   * Adds a new orphan block in the blockchain.
   */
  void AddOrphan (const Block& newBlock);
  
  /**
   * Removes a new orphan block in the blockchain.
   */
  void RemoveOrphan (const Block& newBlock);
  
  /**
   * Prints all the currently orphan blocks.
   */
  void PrintOrphans (void);

  /**
   * Gets the total number of blocks in forks.
   */
  int GetBlocksInForks (void);

  /**
   * Gets the longest fork size
   */
  int GetLongestForkSize (void);

  /* BEGIN: Added by xuzt, 2019/4/22   DAG simulator support */
  bool CheckDagParents (const Block &block);
  std::vector<Block> CheckMissingParent(std::string blockHash);
  bool SetMutiParents(Block& block);
  void AddFree(const Block& block);
  void RemoveFree(const Block& block);
  Block GetBestParent(std::vector<Block> pblocks);
  Block GetBestParent(Block block);
  static bool BlockCompare(Block a,Block b);
  std::string GetBestFreeBlock();
  int GetWitnessLevel(Block block);
  int GetMinWitnessLevel(Block block,std::vector<int>& witnessList);
  bool CheckStable(int checkedBlockMci,int      advancedBlockMci,Block block);
  bool pickParents(Block& newBlock);
  bool AdjustParents(Block& newBlock); 
  bool UpdateLastestIncludedMci(int retreatMci,int oldMci,Block& newBlock);
  bool UpdateMainChain(Block newBlock);  
  bool SaveBlock(Block newBlock);  
  bool AdvanceStableBlock(Block& newBlock);
  bool UpdateBlock(Block& newBlock);
  bool UpdateChildren(Block newBlock);
  Block GetWitnessedLevelBlock(int minerId,std::string bestParent);
  bool SetStable(std::string lsbHash,double stableTime);
  /* END:   Added by xuzt, 2019/4/22 */

  friend std::ostream& operator<< (std::ostream &out, Blockchain &blockchain);

private:
  int                                m_noStaleBlocks;     //total number of stale blocks
  int                                m_totalBlocks;       //total number of blocks including the genesis block
  //std::vector<std::vector<Block>>    m_blocks;            //2d vector containing all the blocks of the blockchain. (row->blockHeight, col->sibling blocks)
  std::vector<Block>                 m_orphans;           //vector containing the orphans
  /* BEGIN: Added by xuzt, 2019/4/18   DAG simulator support */
  int                                m_maxHeight;
public:
  std::vector<std::vector<Block>>    m_blocks; 
  std::map<std::string,std::vector<std::string>>   m_missingParents;
  std::vector<Block>                 m_freeBlocks;
  std::vector<std::string>           m_mainChain;
  std::vector<std::string>           m_stableChain;
  int                                m_lastMci;
  int                                m_lastStableMci;
  int                                m_nodeId;
  int                                m_majorityCount;
  int                                m_witnessCount;
  bool                               m_debug;
  /* END:   Added by xuzt, 2019/4/24 */

};


}// Namespace ns3

#endif /* BITCOIN_H */
