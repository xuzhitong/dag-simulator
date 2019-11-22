/**
 * This file contains the definitions of the functions declared in bitcoin.h
 */


#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/address.h"
#include "ns3/log.h"
#include "bitcoin.h"
#include <queue>
#include <assert.h>
#include <unordered_set>
#include "ns3/simulator.h"

namespace ns3 {
bool g_debug = true;

/**
 *
 * Class Block functions
 *
 */
/* BEGIN: Added by xuzt, 2019/5/14   DAG simulator support */
#define COUT_FOR_NODE if (-1 != m_nodeId && true == g_debug)\
	 std::cout
/* END:   Added by xuzt, 2019/5/14 */
Block::Block(int blockHeight, int minerId, int parentBlockMinerId, int blockSizeBytes, 
             double timeCreated, double timeReceived, Ipv4Address receivedFromIpv4)
{  
  m_blockHeight = blockHeight;
  m_minerId = minerId;
  m_parentBlockMinerId = parentBlockMinerId;
  m_blockSizeBytes = blockSizeBytes;
  m_timeCreated = timeCreated;
  m_timeReceived = timeReceived;
  m_receivedFromIpv4 = receivedFromIpv4;
  /* BEGIN: Added by xuzt, 2019/4/19   DAG simulator support */
  std::ostringstream stringStream;  
  std::string blockHash;
  stringStream << blockHeight << "/" << minerId << "/" << timeCreated;
  m_blockHash = stringStream.str();
  m_isOnMainChain = false;
  m_mci = -1;
  m_witnessLevel = 0;
  m_lastStableMci = 0;
  m_bpIncludedMci = 0;
  m_earliestBpIncludedMci = 0xFFFFFF;
  m_timeStable = 0.0;
  /* END:   Added by xuzt, 2019/4/19 */

}

Block::Block()
{  
  Block(0, 0, 0, 0, 0, 0, Ipv4Address("0.0.0.0"));

}

Block::Block (const Block &blockSource)
{  
  m_blockHeight = blockSource.m_blockHeight;
  m_minerId = blockSource.m_minerId;
  m_parentBlockMinerId = blockSource.m_parentBlockMinerId;
  m_blockSizeBytes = blockSource.m_blockSizeBytes;
  m_timeCreated = blockSource.m_timeCreated;
  m_timeReceived = blockSource.m_timeReceived;
  m_receivedFromIpv4 = blockSource.m_receivedFromIpv4;
  /* BEGIN: Added by xuzt, 2019/4/19   DAG simulator support */
  m_DagParents = blockSource.m_DagParents;
  m_DagChildrens = blockSource.m_DagChildrens;
  std::ostringstream stringStream;  
  std::string blockHash;
  stringStream << m_blockHeight << "/" << m_minerId << "/" << m_timeCreated;
  m_blockHash = stringStream.str();
  m_witnessLevel = blockSource.m_witnessLevel;
  m_isOnMainChain = blockSource.m_isOnMainChain;
  m_mci = blockSource.m_mci;
  m_lastStableMci = blockSource.m_lastStableMci;
  m_bpIncludedMci = blockSource.m_bpIncludedMci;
  m_bestParent = blockSource.m_bestParent;
  m_earliestBpIncludedMci = blockSource.m_earliestBpIncludedMci;
  m_timeStable = blockSource.m_timeStable;
  /* END:   Added by xuzt, 2019/4/19 */

}

Block::~Block (void)
{
}
/* BEGIN: Added by xuzt, 2019/4/19   DAG simulator support */
std::string
Block::GetBlockHash(void) const
{
    /*if (m_blockHash.empty())
    {
	  std::ostringstream stringStream;  
	  std::string blockHash;
	  stringStream << m_blockHeight << "/" << m_minerId << "/" << m_timeCreated;
	  m_blockHash = stringStream.str();
    }*/
	return m_blockHash;
}
/* END:   Added by xuzt, 2019/4/19 */
int 
Block::GetBlockHeight (void) const
{
  return m_blockHeight;
}

void
Block::SetBlockHeight (int blockHeight)
{
  m_blockHeight = blockHeight;
  std::ostringstream stringStream;  
  stringStream << m_blockHeight << "/" << m_minerId << "/" << m_timeCreated;
  m_blockHash = stringStream.str();
}

int 
Block::GetMinerId (void) const
{
  return m_minerId;
}

void 
Block::SetMinerId (int minerId)
{
  m_minerId = minerId;
}


int 
Block::GetParentBlockMinerId (void) const
{
  return m_parentBlockMinerId;
}

void 
Block::SetParentBlockMinerId (int parentBlockMinerId)
{
  m_parentBlockMinerId = parentBlockMinerId;
}

int 
Block::GetBlockSizeBytes (void) const
{
  return m_blockSizeBytes;
}

void 
Block::SetBlockSizeBytes (int blockSizeBytes)
{
  m_blockSizeBytes = blockSizeBytes;
}

double 
Block::GetTimeCreated (void) const
{
  return m_timeCreated;
}
  
double 
Block::GetTimeReceived (void) const
{
  return m_timeReceived;
}
  

Ipv4Address 
Block::GetReceivedFromIpv4 (void) const
{
  return m_receivedFromIpv4;
}
/* BEGIN: Added by xuzt, 2019/4/22   DAG simulator support */
void
Block::SetChildren(const Block& child)
{
    m_DagChildrens.push_back(child.GetBlockHash());
}
std::vector<std::string>
Block::GetDagParents(void) const
{
  return m_DagParents;
}

void Block::SetDagParents(std::vector<std::string> blocks)
{
   m_DagParents = blocks;
}

bool
Block::IsFree() const
{
    return (m_DagChildrens.size() == 0) ? true : false;
}

/* END:   Added by xuzt, 2019/4/22 */
  
void 
Block::SetReceivedFromIpv4 (Ipv4Address receivedFromIpv4)
{
  m_receivedFromIpv4 = receivedFromIpv4;
}

bool 
Block::IsParent(const Block &block) const
{
  if (GetBlockHeight() == block.GetBlockHeight() - 1 && GetMinerId() == block.GetParentBlockMinerId())
    return true;
  else
	return false;
}

bool 
Block::IsChild(const Block &pblock) const
{
  if (GetBlockHeight() == pblock.GetBlockHeight() + 1 && GetParentBlockMinerId() == pblock.GetMinerId())
    return true;
  else
    return false;
}


Block& 
Block::operator= (const Block &blockSource)
{  
  m_blockHeight = blockSource.m_blockHeight;
  m_minerId = blockSource.m_minerId;
  m_parentBlockMinerId = blockSource.m_parentBlockMinerId;
  m_blockSizeBytes = blockSource.m_blockSizeBytes;
  m_timeCreated = blockSource.m_timeCreated;
  m_timeReceived = blockSource.m_timeReceived;
  m_receivedFromIpv4 = blockSource.m_receivedFromIpv4;
  /* BEGIN: Added by xuzt, 2019/4/25   DAG simulator support */
  m_blockHash = blockSource.m_blockHash;
  m_DagParents = blockSource.m_DagParents;
  m_DagChildrens = blockSource.m_DagChildrens;
  m_bestParent = blockSource.m_bestParent;
  m_mci = blockSource.m_mci;
  m_isOnMainChain = blockSource.m_isOnMainChain;
  m_witnessLevel = blockSource.m_witnessLevel;
  m_lastStableMci = blockSource.m_lastStableMci;
  m_bpIncludedMci = blockSource.m_bpIncludedMci;
  m_earliestBpIncludedMci = blockSource.m_earliestBpIncludedMci;
  m_timeStable = blockSource.m_timeStable;
  /* END:   Added by xuzt, 2019/4/25 */
  return *this;
}
/* BEGIN: Added by xuzt, 2019/4/25   DAG simulator support */
Block
Block::GetBestParent(std::vector<Block> pblocks)
{
  Block bestParent(-1,-1);
  for (Block &pblock : pblocks)
  {
      if (pblock.GetBlockHeight() > bestParent.GetBlockHeight() || 
	  	    (pblock.GetBlockHeight()  == bestParent.GetBlockHeight() && pblock.GetTimeCreated() > bestParent.GetTimeCreated() ))
	      bestParent = pblock;
  }
  return bestParent;
}
void
Block::SetBestParent(std::string bpHash)
{
    m_bestParent = bpHash;
}

/* END:   Added by xuzt, 2019/4/25 */

/**
 *
 * Class BitcoinChunk functions
 *
 */
 
BitcoinChunk::BitcoinChunk(int blockHeight, int minerId, int chunkId, int parentBlockMinerId, int blockSizeBytes, 
             double timeCreated, double timeReceived, Ipv4Address receivedFromIpv4) :  
             Block (blockHeight, minerId, parentBlockMinerId, blockSizeBytes, 
                    timeCreated, timeReceived, receivedFromIpv4)
{  
  m_chunkId = chunkId;
}

BitcoinChunk::BitcoinChunk()
{  
  BitcoinChunk(0, 0, 0, 0, 0, 0, 0, Ipv4Address("0.0.0.0"));
}

BitcoinChunk::BitcoinChunk (const BitcoinChunk &chunkSource)
{  
  m_blockHeight = chunkSource.m_blockHeight;
  m_minerId = chunkSource.m_minerId;
  m_chunkId = chunkSource.m_chunkId;
  m_parentBlockMinerId = chunkSource.m_parentBlockMinerId;
  m_blockSizeBytes = chunkSource.m_blockSizeBytes;
  m_timeCreated = chunkSource.m_timeCreated;
  m_timeReceived = chunkSource.m_timeReceived;
  m_receivedFromIpv4 = chunkSource.m_receivedFromIpv4;

}

BitcoinChunk::~BitcoinChunk (void)
{
}

int 
BitcoinChunk::GetChunkId (void) const
{
  return m_chunkId;
}

void
BitcoinChunk::SetChunkId (int chunkId)
{
  m_chunkId = chunkId;
}

BitcoinChunk& 
BitcoinChunk::operator= (const BitcoinChunk &chunkSource)
{  
  m_blockHeight = chunkSource.m_blockHeight;
  m_minerId = chunkSource.m_minerId;
  m_chunkId = chunkSource.m_chunkId;
  m_parentBlockMinerId = chunkSource.m_parentBlockMinerId;
  m_blockSizeBytes = chunkSource.m_blockSizeBytes;
  m_timeCreated = chunkSource.m_timeCreated;
  m_timeReceived = chunkSource.m_timeReceived;
  m_receivedFromIpv4 = chunkSource.m_receivedFromIpv4;

  return *this;
}


/**
 *
 * Class Blockchain functions
 *
 */
 
Blockchain::Blockchain(void)
{
  m_noStaleBlocks = 0;
  m_totalBlocks = 0;
  Block genesisBlock(0, -1, -2, 0, 0, 0, Ipv4Address("0.0.0.0"));
  AddBlock(genesisBlock); 
  /* BEGIN: Added by xuzt, 2019/4/18   DAG simulator support */
  m_maxHeight = 0;
  m_lastMci = 0;
  m_lastStableMci = 0;
  //m_debug = false;
  /* END:   Added by xuzt, 2019/4/18 */
}

Blockchain::~Blockchain (void)
{
}

int 
Blockchain::GetNoStaleBlocks (void) const
{
  return m_noStaleBlocks;
}


int 
Blockchain::GetNoOrphans (void) const
{
  return m_orphans.size();
}

int 
Blockchain::GetTotalBlocks (void) const
{
  return m_totalBlocks;
}


int 
Blockchain::GetBlockchainHeight (void) const 
{
  return GetCurrentTopBlock()->GetBlockHeight();
}

bool 
Blockchain::HasBlock (const Block &newBlock) const
{
  
  if (newBlock.GetBlockHeight() > GetCurrentTopBlock()->GetBlockHeight())
  {
    /* The new block has a new blockHeight, so we haven't received it previously. */
	
	return false;
  }
  else														
  {															
    /*  The new block doesn't have a new blockHeight, so we have to check it is new or if we have already received it. */
	
    for (auto const &block: m_blocks[newBlock.GetBlockHeight()]) 
    {
      if (block == newBlock)
	  {
	    return true;
	  }
    }
  }
  return false;
}

bool 
Blockchain::HasBlock (int height, int minerId) const
{
  
  if (height > GetCurrentTopBlock()->GetBlockHeight())		
  {
    /* The new block has a new blockHeight, so we haven't received it previously. */
	
	return false;
  }
  else														
  {															
    /*  The new block doesn't have a new blockHeight, so we have to check it is new or if we have already received it. */
	
    for (auto const &block: m_blocks[height]) 
    {
      if (block.GetBlockHeight() == height && block.GetMinerId() == minerId)
	  {
	    return true;
	  }
    }
  }
  return false;
}


Block 
Blockchain::ReturnBlock(int height, int minerId)
{
  std::vector<Block>::iterator  block_it;

  if (height <= GetBlockchainHeight() && height >= 0)
  {
    for (block_it = m_blocks[height].begin();  block_it < m_blocks[height].end(); block_it++)
    {
      if (block_it->GetBlockHeight() == height && block_it->GetMinerId() == minerId)
        return *block_it;
    }
  }
  
  for (block_it = m_orphans.begin();  block_it < m_orphans.end(); block_it++)
  {
    if (block_it->GetBlockHeight() == height && block_it->GetMinerId() == minerId)
      return *block_it;
  }
  
  return Block(-1, -1, -1, -1, -1, -1, Ipv4Address("0.0.0.0"));
}
/* BEGIN: Added by xuzt, 2019/4/26   DAG simulator support */
Block 
Blockchain::ReturnBlock(std::string blockHash)
{
  std::vector<Block>::iterator  block_it;
  int height = std::stoi(blockHash.substr(0,blockHash.find("/")));

  if (height <= GetBlockchainHeight() && height >= 0)
  {
    for (block_it = m_blocks[height].begin();  block_it < m_blocks[height].end(); block_it++)
    {
      if (block_it->GetBlockHash() == blockHash)
        return *block_it;
    }
  }

  
  /*for (block_it = m_orphans.begin();  block_it < m_orphans.end(); block_it++)
  {
    if (block_it->GetBlockHash() == blockHash)
      return *block_it;
  }*/
  
  return Block(-1, -1, -1, -1, -1, -1, Ipv4Address("0.0.0.0"));
}
bool
Blockchain::UpdateBlock(Block& newBlock)
{
    int height = newBlock.GetBlockHeight();
	if (height < 0 || height > m_blocks.size())
		return false;
	if (height == 0)
		COUT_FOR_NODE<<"UpdateBlock is genesis block ,isOnMainchain = "<<newBlock.m_isOnMainChain<<"\n";
    for (int i = 0; i < m_blocks[height].size();i++)
    {
        Block block = m_blocks[height][i];
        if (block.GetBlockHash() == newBlock.GetBlockHash())
        {
            //std::cout<<"Blockchain::UpdateBlock find block "<<block.GetBlockHash()<<"\n";
            m_blocks[height][i] = newBlock;
			break;
        }
    }
	return true;
}
/* END:   Added by xuzt, 2019/4/26 */
bool 
Blockchain::IsOrphan (const Block &newBlock) const
{													
  for (auto const &block: m_orphans) 
  {
    if (block == newBlock)
    {
      return true;
    }
  }
  return false;
}


bool 
Blockchain::IsOrphan (int height, int minerId) const
{													
  for (auto const &block: m_orphans) 
  {
    if (block.GetBlockHeight() == height && block.GetMinerId() == minerId)
    {
      return true;
    }
  }
  return false;
}
/* BEGIN: Added by xuzt, 2019/4/19   DAG simulator support */
bool 
Blockchain::SetMutiParents(Block& newBlock)
{
     
    /*std::vector<std::string> dagParentBlocks = pickParents(newBlock);
	newBlock.SetDagParents(dagParentBlocks);
	newBlock.SetBlockHeight(bestFreeBlock.GetBlockHeight() + 1); */
	return true;
	
}
bool
Blockchain::CheckDagParents (const Block &newBlock) 
{
  std::vector<Block>::iterator  block_it;
  std::vector<std::string>::iterator parent_it;

  bool missParent = false;
  std::vector<std::string> dagParents = newBlock.GetDagParents();
  for (auto& parentHash : dagParents)
  {
    COUT_FOR_NODE<<"Blockchain::CheckDagParents parent is "<<parentHash<<"\n";
    Block dagParent = ReturnBlock(parentHash);
	if (dagParent.GetBlockHeight() < 0)
	{
	   //std::string blockHash = parentHash;
	   //std::map<std::string,std::vector<Block>>::iterator it =  m_missingParents.find(blockHash);
	   //if (it == m_missingParents.end())
	     //m_missingParents.push_back(blockHash);
	   COUT_FOR_NODE<<"Blockchain::CheckDagParents parent  "<<parentHash<<" is missing\n";
	   m_missingParents[newBlock.GetBlockHash()].push_back(parentHash);
	   missParent = true ;
	}
  }

  return !missParent;
  //return true;
}
std::string 
Blockchain::GetBestFreeBlock()
{
    //Block bestFree(-1, -1, -1, -1, -1, -1, Ipv4Address("0.0.0.0"));
	/*std::vector<Block> freeBlocks = m_blocks[m_blocks.size()-1];
    for (Block& block : freeBlocks)
    {
		if (block.GetTimeCreated() > bestFree.GetTimeCreated())
			bestFree = block;
    }*/
	/*std::vector<Block>::reverse_iterator freeIt = m_freeBlocks.rbegin();
	if (freeIt != m_freeBlocks.rend())
	{
	    bestFree = *freeIt;
		m_freeBlocks.erase(std::next(freeIt).base());
	}
	else
		bestFree = m_blocks[m_blocks.size()-1][0];
	*/
	Block bestFree = GetBestParent(m_freeBlocks);
	if (bestFree.GetBlockHeight() == -1)
		return (m_blocks[m_blocks.size()-1][0]).GetBlockHash();
	return bestFree.GetBlockHash();
}

Block
Blockchain::GetBestParent(std::vector<Block> pblocks)
{
   int maxParentLevel = -1;
   Block bestParent(-1,-1);
   int maxWitnessLevel = -1;
   int maxBlockHash = 0;
   for (Block &pblock : pblocks)
   {
       int blockHeight = pblock.GetBlockHeight();
	   int wl = pblock.m_witnessLevel;
       if ( wl > maxWitnessLevel 
	   	    || ( wl == maxWitnessLevel && blockHeight > maxParentLevel ) 
	   	    || ( wl == maxWitnessLevel && blockHeight == maxParentLevel 
	   	          && pblock.GetTimeCreated() < bestParent.GetTimeCreated() ))
       {
           maxWitnessLevel = wl;
	       maxParentLevel = blockHeight;
		   bestParent = pblock;
       }	   	    
   }
   return bestParent;
}

Block
Blockchain::GetBestParent(Block block)
{
  Block bestParent(-1,-1);
  std::vector<Block> pblocks;
  for (std::string& pBlockHash : block.GetDagParents())
  {
	  //printf("GetBestParent Current block is %s,pBlockHash = %s\n",block.GetBlockHash().c_str(),pBlockHash.c_str());
	  Block pblock = ReturnBlock(pBlockHash);
	  if (pblock.GetBlockHeight() != -1)
		pblocks.push_back(pblock);
  }

  //std::cout<<"GetBestParent parent blocks size is "<<pblocks.size()<<"\n";
  return GetBestParent(pblocks);
}
Block
Blockchain::GetWitnessedLevelBlock(int minerId,std::string bestParent)
{

	std::unordered_set<int> collectedWitnesses;
	Block wlBlock(-1,-1);
	collectedWitnesses.insert(minerId);
	std::string bpHash = bestParent;
	//std::cout<<"Blockchain::GetWitnessedLevelBlock m_majorityCount is "<<m_majorityCount<<",minerId is "<<minerId<<"\n";
	while (true)
	{
	    //std::cout<<"Blockchain::GetWitnessedLevelBlock best parent is "<<bpHash<<"\n";
		if (bpHash == "0/-1/0")
		{
		    return m_blocks[0][0];
		}

		Block bpBlock = ReturnBlock(bpHash);
		assert(bpBlock.GetBlockHeight() > 0);
		if (collectedWitnesses.count(bpBlock.GetMinerId()))
			break;
		else
		{
			collectedWitnesses.insert(bpBlock.GetMinerId());
			if (collectedWitnesses.size() == m_majorityCount)
			{ 
				return bpBlock;

			}
		}

		bpHash = bpBlock.m_bestParent;
		
	}

	return wlBlock;
}


int
Blockchain::GetWitnessLevel(Block block)
{
    std::vector<int> witnessList;
	Block bpBlock = block;
	int minerId = -1;
	while(true)
	{
	    if (bpBlock.GetBlockHeight() <= 0) //genesis block
	        break;
		minerId = bpBlock.GetMinerId();
		if (minerId != -1 && std::find(witnessList.begin(),witnessList.end(),minerId) == witnessList.end())
		{
		    //std::cout<<"Blockchain::GetWitnessLevel miner id is "<<minerId<<",witness list count is "<<witnessList.size()<<"\n";
			witnessList.push_back(minerId);
		}
		if (witnessList.size() >= MAJORITY_WITNESSES_COUNT)
	    {
	        COUT_FOR_NODE<<"Blockchain::GetWitnessLevel get witness level "<<bpBlock.GetBlockHeight()<<"\n";
	        return bpBlock.GetBlockHeight();
		}
		//std::cout<<"Blockchain::GetWitnessLevel block "<<bpBlock.GetBlockHash()<<",best parent hash is "<<bpBlock.m_bestParent<<"\n";
	    bpBlock = ReturnBlock(bpBlock.m_bestParent);
		//COUT_FOR_NODE<<"Blockchain::GetWitnessLevel best parent "<<bpBlock.GetBlockHash()<<"\n";
	}
	return 0;
}
int
Blockchain::GetMinWitnessLevel(Block block,std::vector<int>& witnessList)
{
	Block bpBlock = block;
	int minerId = -1;
	while(true)
	{
	    if (bpBlock.GetBlockHeight() <= 0) //genesis block
	        break;
		minerId = bpBlock.GetMinerId();
		if (minerId != -1 && std::find(witnessList.begin(),witnessList.end(),minerId) == witnessList.end())
			witnessList.push_back(minerId);
		if (witnessList.size() >= MAJORITY_WITNESSES_COUNT)
	    {
	        return GetWitnessLevel(bpBlock);
		}
		
	    bpBlock = ReturnBlock(bpBlock.m_bestParent);
		//COUT_FOR_NODE<<"Blockchain::GetMinWitnessLevel best parent "<<bpBlock.GetBlockHash()<<"\n";
	}
	return 0;
}
bool
Blockchain::CheckStable(int checkedBlockMci,int      advancedBlockMci,Block block)
{
	COUT_FOR_NODE<<"Blockchain::CheckStable "<<checkedBlockMci<<","<<advancedBlockMci<<","<<block.GetBlockHash()<<"\n";

    if (checkedBlockMci >= advancedBlockMci)
		return true;
	if (checkedBlockMci < 0 || checkedBlockMci >= m_mainChain.size()
		|| advancedBlockMci < 0 || advancedBlockMci >= m_mainChain.size())
		return false;

    std::string checkedBlockHash = m_mainChain[checkedBlockMci];
	std::string advancedBlockHash = m_mainChain[advancedBlockMci];
	Block checkedBlock = ReturnBlock(checkedBlockHash);
	Block advancedBlock = ReturnBlock(advancedBlockHash);
    Block bpBlock = GetBestParent(block);
	std::vector<std::string> parents = block.GetDagParents();
	std::unordered_set<std::string> mcBlocks;
    //1.get min witness level and list
    std::vector<int> witnessList;
	int minWl = GetMinWitnessLevel(bpBlock,witnessList);
    COUT_FOR_NODE<<"Blockchain::CheckStable bpBlock is "<<bpBlock.GetBlockHash()<<",minWl is "<<minWl<<",witness list is : ";
	for (auto& witness : witnessList)
	{
	    COUT_FOR_NODE<<witness<<",";
	}
	COUT_FOR_NODE<<"\n";
	if (minWl < advancedBlock.GetBlockHeight())
		return false;
	
	//2.get main chain blocks
	//COUT_FOR_NODE<<"Blockchain::CheckStable insert mcBlocks :\n";
	while(true)
	{ 
	    if (bpBlock.GetBlockHeight() == 0)
			break;
		if (bpBlock.GetBlockHash() == advancedBlock.GetBlockHash())
			break;
		mcBlocks.insert(bpBlock.GetBlockHash());
	    //COUT_FOR_NODE<<bpBlock.GetBlockHash()<<",";
	    bpBlock = GetBestParent(bpBlock);

	}
	//COUT_FOR_NODE<<"\n";

	//3.handle branch blocks
	std::queue<std::string> searchBlocks;
	std::unordered_set<std::string> searchedHashes;
	std::unordered_set<std::string> handledBlocks;
	Block bBlock; 
	//3K-2N-1
	//K=6,N=7,3
	//K=5,N=6,2
	//K=8,N=10,3
	//int checkCount = 3 * MAJORITY_WITNESSES_COUNT - 2 * WITNESS_COUNT - 1;

	//2K-N-M
	//int checkCount = 2 * MAJORITY_WITNESSES_COUNT - WITNESS_COUNT - 1;
	int checkCount = 2 * m_majorityCount - m_witnessCount - 1;
	COUT_FOR_NODE<<"Blockchain::CheckStable check count is "<<checkCount<<"\n";
	assert(checkCount > 0);
	for (auto &pHash : parents)
	{
	    //Block pblock = ReturnBlock(pHash);
	    searchedHashes.insert(pHash);
		searchBlocks.push(pHash);
		COUT_FOR_NODE<<"Blockchain::CheckStable begin 1 get parent "<<pHash<<"\n";
	}
    while (!searchBlocks.empty())
    {
       std::string bHash = std::move(searchBlocks.front());
       bBlock = ReturnBlock(bHash);
	   assert(bBlock.GetBlockHeight() >= 0);
	   //COUT_FOR_NODE<<"Blockchain::CheckStable process searchBlock "<<bBlock.GetBlockHash()<<","\
	   	 //           <<bBlock.m_bpIncludedMci<<","<<bBlock.GetMinerId()<<","<<bBlock.m_earliestBpIncludedMci<<"\n";
 	   searchBlocks.pop();
	   if (bBlock.GetBlockHeight() < minWl)
	       continue;
       
 	   if (!mcBlocks.count(bBlock.GetBlockHash()))
 	   {
 	       COUT_FOR_NODE<<"Blockchain::CheckStable search branch block "<<bBlock.GetBlockHash()\
		   	            <<",bpIncludedMci = "<<bBlock.m_bpIncludedMci<<",minerId="<<bBlock.GetMinerId()<<"\n";
		   if (bBlock.GetBlockHeight() < checkedBlockMci || bBlock.m_earliestBpIncludedMci >= advancedBlockMci)
	   	       continue;
 		   if (std::find(witnessList.begin(),witnessList.end(),bBlock.GetMinerId()) != witnessList.end()
 		   	    && bBlock.m_bpIncludedMci >= checkedBlockMci
 		   	    && bBlock.m_bpIncludedMci < advancedBlockMci)
 		   {
 		        if (!handledBlocks.count(bBlock.GetBlockHash()))
 		        {
     		        COUT_FOR_NODE<<"Blockchain::CheckStable get branch block "<<bBlock.GetBlockHash()<<"\n";
     		        Block branchBpBlock = bBlock;
     				std::vector<Block> branchWitnesses;
     		        while(true)
     		        {
     		            handledBlocks.insert(branchBpBlock.GetBlockHash());
     		            COUT_FOR_NODE<<"Blockchain::CheckStable check branch best parent "<<branchBpBlock.GetBlockHash()<<"\n";
         		        if (branchBpBlock.GetBlockHeight() < minWl)
         		        {
         		            COUT_FOR_NODE<<"Blockchain::CheckStable branch best parent "<<branchBpBlock.GetBlockHash()<<" is advanceble\n";
     						break;
         		        }
     					if (std::find(witnessList.begin(),witnessList.end(),branchBpBlock.GetMinerId()) != witnessList.end()
     						&& std::find(branchWitnesses.begin(),branchWitnesses.end(),branchBpBlock) == branchWitnesses.end())
                             branchWitnesses.push_back(branchBpBlock);
     					if (branchWitnesses.size() == checkCount)
     					{
     					    COUT_FOR_NODE<<"Blockchain::CheckStable branch best parent "<<branchBpBlock.GetBlockHash()<<" is not advanceble\n";
     					    return false;
     					}
    					if (branchBpBlock.m_isOnMainChain)
    					{
    					    COUT_FOR_NODE<<"Blockchain::CheckStable branch best parent "<<branchBpBlock.GetBlockHash()<<" is on main chain\n";
    						break;
    					}
     					branchBpBlock = GetBestParent(branchBpBlock);
     		        }
 		        }
 		        
 		   }
 	   }
	   for (auto &pHash : bBlock.GetDagParents())
	   {
	       //Block pblock = ReturnBlock(pHash); 
		   //if (pblock.GetBlockHeight() > 0)
		   {   
		       auto r = searchedHashes.insert(pHash);
			   if (r.second)
			   {
	               searchBlocks.push(pHash);
		           COUT_FOR_NODE<<"Blockchain::CheckStable end 2 get parent "<<pHash<<"\n";
			   }
		   }
	   }
    }
    
	return true;
	
}
bool
Blockchain::pickParents(Block& newBlock)
{
    //genesis block
    if (newBlock.GetBlockHeight() == 0)
    {
        return true;
    }

	static int pickFailedCount = 0;
    //1.pick parents,one is the tip block and other two are randly picked from free blocks set
    std::vector<std::pair<uint32_t,uint32_t>> dagParents;
    //Block bestFreeBlock = GetBestParent(m_freeBlocks);
	int i = 0;
	std::vector<std::string> dagParentBlocks;
	std::string bestFreeHash = GetBestFreeBlock();
	Block bestFreeBlock = ReturnBlock(bestFreeHash);
	
	/*Block bestFreeBlock;
	std::vector<Block>::reverse_iterator freeIt = m_freeBlocks.rbegin();
	if (freeIt != m_freeBlocks.rend())
	{
	    bestFreeBlock = *freeIt;
		if (bestFreeBlock.GetBlockHeight() != 0 && bestFreeBlock.GetMinerId() == m_nodeId)
		{
		    COUT_FOR_NODE<<"Pick parents get the best free block is mine\n";
		    return false;
		}
	    m_freeBlocks.erase(std::next(freeIt).base());
	    
	}
	else
		bestFreeBlock = m_blocks[m_blocks.size()-1][0];*/
    int maxPickFailedCount = 8;
    /*if (bestFreeBlock.GetBlockHeight() >0 && bestFreeBlock.GetMinerId() == m_nodeId)
    {
        COUT_FOR_NODE<<"Pick parents get the best free block is mine\n";
        pickFailedCount++;
        if (pickFailedCount < maxPickFailedCount)
        {
            //AddFree(bestFreeBlock);
            return false;
        }
        else
        {
             pickFailedCount = 0;
        }
    }*/
          
	
	COUT_FOR_NODE<<"Pick the tip block is "<<bestFreeBlock.GetBlockHash()<<"\n";

	//first pick the tip block of chain
    dagParentBlocks.push_back(bestFreeBlock.GetBlockHash());
    //bestFreeBlock.SetChildren(newBlock);
	//std::cout<<"Miner "<<m_nodeId<<" get best free block "<<bestFreeBlock.GetBlockHash()<<" set children "<<bestFreeBlock.m_DagChildrens[0]<<"\n";
	//UpdateBlock(bestFreeBlock);
	RemoveFree(bestFreeBlock);

	//second pick at most 8 blocks at random 
	while (i < 8 && m_freeBlocks.size() > 0)
	{
	    int randIndex = rand() % m_freeBlocks.size();
		std::string pickHash = m_freeBlocks[randIndex].GetBlockHash();
		Block pickParent = ReturnBlock(pickHash);
		COUT_FOR_NODE<<"Pick the free parent = "<<pickParent.GetBlockHash()<<"\n";
		dagParentBlocks.push_back(pickParent.GetBlockHash());
		//pickParent.SetChildren(newBlock);
		//UpdateBlock(pickParent);
		m_freeBlocks.erase(m_freeBlocks.begin() + randIndex);
		i++;
		
	}   
	//should not impossible
	//AdjustParents(newBlock);

    //2.get links
    //To Do

	//3.get best parent
  	newBlock.SetDagParents(dagParentBlocks);
	Block bpParent = GetBestParent(newBlock);
	COUT_FOR_NODE<<"Blockchain::pickParents get best parent is "<<bpParent.GetBlockHash()<<"\n";
	Block wlBlock = GetWitnessedLevelBlock(newBlock.GetMinerId(),bpParent.GetBlockHash());
    if (wlBlock.GetBlockHeight() == -1)
    {
        COUT_FOR_NODE<<"Miner : "<<m_nodeId<<" Pick parents can't get the witnessed level block.\n";
        return false;
	}
    //4.get last stable block
    int maxParentLsbMci = -1;
    int maxBlockHeight = -1;
    for (auto& pblockHash : dagParentBlocks)
    {
        Block pblock = ReturnBlock(pblockHash);
		COUT_FOR_NODE<<"Blockchain::pickParents parent last stable mci is "<< pblock.m_lastStableMci<<", "\
			<<pblock.GetBlockHeight()<<","<<pblockHash<<"\n";
		
		if (-1 == maxParentLsbMci || pblock.m_lastStableMci > maxParentLsbMci)
			maxParentLsbMci = pblock.m_lastStableMci;
		if (-1 == maxBlockHeight || pblock.GetBlockHeight() > maxBlockHeight)
			maxBlockHeight = pblock.GetBlockHeight();
    }

    newBlock.SetBlockHeight(maxBlockHeight + 1); 
	
    COUT_FOR_NODE<<"Blockchain::pickParents maxParentLsbMci is "<<maxParentLsbMci<<"\n";
	int nextLsbMci = maxParentLsbMci + 1;
	int currentLsbMci = nextLsbMci;
	if (CheckStable(maxParentLsbMci,nextLsbMci, newBlock))
	{
	    /*int skip = 1024;
	    while(skip > 0)
	    {

    		nextLsbMci += skip;
    		if (CheckStable(currentLsbMci, nextLsbMci, newBlock))
    		{
    		    currentLsbMci = nextLsbMci;
				nextLsbMci += skip;
    		}
			else
			{
			    skip /= 2;
				nextLsbMci -= skip;
			}

	    }*/
	    newBlock.m_lastStableMci = nextLsbMci;
		Block lastStableBlock = ReturnBlock(m_mainChain[nextLsbMci]);
		lastStableBlock.m_timeStable = Simulator::Now ().GetSeconds ();
		UpdateBlock(lastStableBlock);
		COUT_FOR_NODE<<"Block "<<lastStableBlock.GetBlockHash()<<" advanced to stable state at "<<lastStableBlock.m_timeStable<<"\n";
	}
	else
		newBlock.m_lastStableMci = maxParentLsbMci;
		

	COUT_FOR_NODE<<"Blockchain::pickParents finished\n";
	//set the block height(dag level)
	
	
	return true;
}
bool
Blockchain::AdjustParents(Block& newBlock)
{
    //std::vector<Block> excludeParents;
	std::vector<std::string> parents = newBlock.GetDagParents();
    while(true)
    {
	   Block bpBlock = GetBestParent(newBlock);
	   int bpWl = GetWitnessLevel(bpBlock);
	   int newWl = GetWitnessLevel(newBlock);
		if (newWl > bpWl)
		{
		   //best parent is right
		   break;
		}
		else
		{
		   std::vector<std::string> bpBlockParents = bpBlock.GetDagParents();
		   parents.erase(std::find(parents.begin(),parents.end(),bpBlock.GetBlockHash()));
		   for (auto& bpBlockParentHash : bpBlockParents)
		   {
		       Block bpBlockParent = ReturnBlock(bpBlockParentHash);
			   auto it = std::find(bpBlockParent.m_DagChildrens.begin(),bpBlockParent.m_DagChildrens.end(),bpBlock.GetBlockHash());
		       if (it != bpBlockParent.m_DagChildrens.end())
			   	  bpBlockParent.m_DagChildrens.erase(it);
			   //has no any other childen
			   if (bpBlockParent.m_DagChildrens.size() == 0)
			   {
			       parents.push_back(bpBlockParentHash);
			   }
			   
		   }
		   newBlock.SetDagParents(parents);
		   //excludeParents.push_back(bpBlock);
		   
		}
    }
}
bool
Blockchain::UpdateChildren(Block newBlock)
{
    for (auto& pHash : newBlock.GetDagParents())
    {
        Block pBlock = ReturnBlock(pHash);
		assert(pBlock.GetBlockHeight() >= 0);
		if (std::find(pBlock.m_DagChildrens.begin(),pBlock.m_DagChildrens.end(),newBlock.GetBlockHash())
			    == pBlock.m_DagChildrens.end())
		{
		    pBlock.SetChildren(newBlock);
		    UpdateBlock(pBlock);
		}
    }
}
bool
Blockchain::UpdateMainChain(Block newBlock)
{
    std::list<Block> newMcBlocks;
	int retreatMci = 0;

	if (newBlock.GetBlockHeight() == 0)
	{
	    COUT_FOR_NODE<<"Update main chain add genesis block\n";
	    //Block gensis = m_blocks[0][0];
	    m_mainChain.push_back("0/-1/0");
		m_blocks[0][0].m_mci = 0;
		m_blocks[0][0].m_isOnMainChain = true;
		m_lastMci = 0;
		m_lastStableMci = 0;
		return true;
	}
	std::string bfHash = GetBestFreeBlock();
	Block bpBlock = ReturnBlock(bfHash);
	COUT_FOR_NODE<<"UpdateMainChain best free block is "<<bpBlock.GetBlockHash()<<"\n";
	if (bpBlock.GetBlockHeight() < 0)
		return false;
	int i = 0;
	Block lastBlock;
	while(true)
	{
	    COUT_FOR_NODE<<"find main chain block "<<bpBlock.GetBlockHash()<<" ,on main chain flag is "<<bpBlock.m_isOnMainChain<<"\n";
	    if (bpBlock.m_isOnMainChain)
	    {
			retreatMci = bpBlock.m_mci;
			break;
	    }
	    newMcBlocks.push_front(bpBlock);	    
		//UpdateBlock(bpBlock);

		//lastBlock = bpBlock;
		COUT_FOR_NODE<<"block "<<bpBlock.GetBlockHash()<<" best parent is "<<bpBlock.m_bestParent<<"\n";
	    bpBlock = ReturnBlock(bpBlock.m_bestParent);
		if (bpBlock.GetBlockHeight() == 0)
			break;
		
		//i++;
		if (bpBlock.GetBlockHeight() == -1)
		{
			std::cout<<"------------------------------Blockchain::UpdateMainChain fall into dead line,block is "<<lastBlock.GetBlockHash()<<"\n";
		    break;

		}
	}

	COUT_FOR_NODE<<"UpdateMainChain new mci is "<<retreatMci<<",old mci is "<<m_lastMci<<",main chain size is "<<m_mainChain.size()<<"\n";
	/*COUT_FOR_NODE<<"Now the main chain block set is :\n";
	for (auto& blockHash : m_mainChain)
	    COUT_FOR_NODE<<blockHash<<",";
	COUT_FOR_NODE<<"\n";*/

    int mci = m_lastMci;
	Block oldMciBlock;
	assert(mci < m_mainChain.size());
	while ( mci > retreatMci)
	{
	    oldMciBlock = ReturnBlock(m_mainChain[mci]);
		if (oldMciBlock.GetBlockHeight() == -1)
			std::cout<<"Update main chain 1 block["<<mci<<"] is not found\n";
		//COUT_FOR_NODE<<"UpdateMainChain old main chain blocks["<<mci<<"] is "<<oldMciBlock.GetBlockHash()<<"\n";
	    oldMciBlock.m_isOnMainChain = false;
		UpdateBlock(oldMciBlock);
		m_mainChain.pop_back();
	    mci--;
	}

    int newMci = retreatMci;
	//std::cout<<"UpdateMainChain retreat mci is "<<mci<<"\n";
	for ( auto it = newMcBlocks.begin();it != newMcBlocks.end();it++ )
	{
		newMci++;
	    Block block(*it);
		block.m_mci = newMci;
	    block.m_isOnMainChain = true;
	    m_mainChain.push_back(it->GetBlockHash());
		if (block.GetBlockHeight() == -1)
			std::cout<<"Update main chain 2 block["<<newMci<<"] is not found\n";
        UpdateBlock(block);
	    COUT_FOR_NODE<<"UpdateMainChain new main chain blocks["<<newMci<<"] is "<<block.GetBlockHash()<<"\n";

	}	

	//update lastest included mci
	/*(COUT_FOR_NODE<<"Now the main chain block set is :\n";
	for (auto& blockHash : m_mainChain)
	    COUT_FOR_NODE<<blockHash<<",";
	COUT_FOR_NODE<<"\n";*/
	UpdateLastestIncludedMci(retreatMci,m_lastMci,newBlock);

	m_lastMci = newMci;
	return true;
}
bool
Blockchain::UpdateLastestIncludedMci(int retreatMci,int oldMci,Block& newBlock)
{
   COUT_FOR_NODE<<"Blockchain::UpdateLastestIncludedMci retreatMci="<<retreatMci<<",oldMci="<<oldMci<<",block="<<newBlock.GetBlockHash()<<"\n";
   std::queue<std::string> searchHashes;
   std::map<int,std::unordered_set<std::string>> updateHashes;
   //genesis block
   if (newBlock.GetBlockHeight() == 0)
   {
       /*newBlock.m_bpIncludedMci = 0;
	   UpdateBlock(newBlock);*/
       return true;
   }

   if (retreatMci < 0 || retreatMci >= m_mainChain.size())
       return false;

   //main chain fall back
   assert(retreatMci < m_mainChain.size());
   if (retreatMci < oldMci)
   {
       Block retreatBlock = ReturnBlock(m_mainChain[retreatMci]);
	   assert(retreatBlock.GetBlockHeight() >= 0);
	   searchHashes.push(m_mainChain[retreatMci]);
	   while(!searchHashes.empty())
	   {
	       std::string searchHash = searchHashes.front();
		   searchHashes.pop();
		   Block searchBlock = ReturnBlock(searchHash);
		   //if (searchBlock.GetBlockHeight() == -1)
		       //continue;
		   assert(searchBlock.GetBlockHeight() >= 0);
		   std::vector<std::string> childrens = searchBlock.m_DagChildrens;
		   //COUT_FOR_NODE<<"Blockchain::UpdateLastestIncludedMci retreat block "<<m_mainChain[retreatMci]<<",childrens:";
		   for (auto& children : childrens)
		   {
		       //COUT_FOR_NODE<<children<<",";
		       Block childBlock = ReturnBlock(children);
			   if (childBlock.GetBlockHeight() == -1)
			       COUT_FOR_NODE<<"child block "<<children<<" is not found\n";
			   assert(childBlock.GetBlockHeight() != -1);
               auto r = updateHashes[childBlock.GetBlockHeight()].insert(children);
			   if (r.second)
		           searchHashes.push(children);
		   }
		   //COUT_FOR_NODE<<"\n";
		   
	   }
   }

   if (updateHashes.empty())
   {
       updateHashes[newBlock.GetBlockHeight()].insert(newBlock.GetBlockHash());
   }
   
   //COUT_FOR_NODE<<"Blockchain::UpdateLastestIncludedMci updateHashes size is "<<updateHashes.size()<<"\n";
   for (auto uit(updateHashes.begin());uit != updateHashes.end();uit++)
   {
       std::unordered_set<std::string> uHashes(uit->second);
	   for (auto it(uHashes.begin());it != uHashes.end();it++)
	   {
	       std::string updateHash(*it);
           Block updateBlock = ReturnBlock(updateHash);
	       //COUT_FOR_NODE<<"Blockchain::UpdateLastestIncludedMci update block is "<<updateHash\
	   	     //      <<",isOnMainChain="<<updateBlock.m_isOnMainChain<<",mci="<<updateBlock.m_mci<<"\n";
	       if (!updateBlock.m_isOnMainChain || updateBlock.m_mci > retreatMci)
    	   {
    	       std::string bpHash = updateBlock.m_bestParent;
    		   //COUT_FOR_NODE<<"Blockchain::UpdateLastestIncludedMci best parent hash is "<<bpHash<<"\n";
    		   int minBpLimci = 0xFFFFFF;
    		   for (auto &pHash : updateBlock.m_DagParents)
    		   {
    		       Block pBlock = ReturnBlock(pHash);
    			   //best parent
    			   if (pHash == bpHash)
    			   {
    			       updateBlock.m_bpIncludedMci = pBlock.m_isOnMainChain ? pBlock.m_mci : pBlock.m_bpIncludedMci;
    				   break;
    				   //COUT_FOR_NODE<<"Update "<<updateHash<<" best parent lastest included main chain index is "<<updateBlock.m_bpIncludedMci<<"\n";
    				   //updateBlock.m_mci = pBlock.m_mci;				   
    			   }
    			   /*else
    			   {
    			      updateBlock.m_mci = pBlock.m_isOnMainChain ? pBlock.m_mci : pBlock.m_bpIncludedMci;
    			   }*/
    
    		   }
    
    		   minBpLimci = updateBlock.m_bpIncludedMci;
    		   for (auto& pHash : updateBlock.m_DagParents)
    		   {
    		       Block pBlock = ReturnBlock(pHash);
    			   if (!pBlock.m_isOnMainChain)
    			   {
    			   	    //COUT_FOR_NODE<<"Blockchain::UpdateLastestIncludedMci parent  "<<pHash\
    	   	                //<<",m_earliestBpIncludedMci ="<<pBlock.m_earliestBpIncludedMci<<"\n";
        		        if (minBpLimci > pBlock.m_earliestBpIncludedMci)
        			   	    minBpLimci = pBlock.m_earliestBpIncludedMci;
    			   }
    		   }
    		   updateBlock.m_earliestBpIncludedMci = minBpLimci;
    		   //COUT_FOR_NODE<<"Blockchain::UpdateLastestIncludedMci update block is "<<updateHash\
    	   	           //<<",m_earliestBpIncludedMci ="<<updateBlock.m_earliestBpIncludedMci<<",m_bpIncludedMci = "<<updateBlock.m_bpIncludedMci<<",mci="<<updateBlock.m_mci<<"\n";
    		   UpdateBlock(updateBlock);
    	   }
	   	}
   }
   return true;
}

bool
Blockchain::SaveBlock(Block newBlock)
{
	Block updateBlock = newBlock;

	//update parent block's children
    UpdateChildren(updateBlock);
	
    //save best parent
    if (updateBlock.GetBlockHeight() > 0)
    {
        updateBlock.m_bestParent = GetBestParent(updateBlock).GetBlockHash();
		//std::cout<<"Blockchain::SaveBlock block "<<newBlock.GetBlockHash()<<",best parent "<<updateBlock.m_bestParent<<"\n";
		//updateBlock.m_isOnMainChain = false;
        UpdateBlock(updateBlock);
    }

	//save witness level
	updateBlock.m_witnessLevel = GetWitnessLevel(updateBlock);
    COUT_FOR_NODE<<"Update block witness level "<<updateBlock.GetBlockHash()<<"\n";
	UpdateBlock(updateBlock);
	//Block updateBlock = ReturnBlock(newBlock.GetBlockHash());
    //COUT_FOR_NODE<<"Blockchain::SaveBlock change block witness level to "<<updateBlock.m_witnessLevel<<",get witness level is "<<newBlock.m_witnessLevel<<"\n";
    COUT_FOR_NODE<<"Blockchain::SaveBlock change block witness level to "<<updateBlock.m_witnessLevel<<"\n";

    if (updateBlock.IsFree())
  	    AddFree(updateBlock);
	//find main chain changes
    UpdateMainChain(updateBlock);

    COUT_FOR_NODE<<"BitcoinNode::SaveBlock   Now the main chain is :\n";
	for (auto &mc : m_mainChain)
		COUT_FOR_NODE<<mc<<",";
	COUT_FOR_NODE<<"\n";
	updateBlock = ReturnBlock(updateBlock.GetBlockHash());
	assert(updateBlock.GetBlockHeight() >= 0);



	//advance stable block
	bool rst = AdvanceStableBlock(updateBlock);
	return rst;
	//return true;
}
bool
Blockchain::AdvanceStableBlock(Block& newBlock)
{
    //genesis block
    if (newBlock.GetBlockHeight() == 0)
    {
		m_stableChain.push_back(newBlock.GetBlockHash());
		m_lastStableMci = 0;
		return true;
    }

	COUT_FOR_NODE<<"Blockchain::AdvanceStableBlock newBlock.m_lastStableMci is "<<newBlock.m_lastStableMci\
	        <<", m_lastStableMci is "<<m_lastStableMci<<"\n";
	
    if ( newBlock.m_lastStableMci > m_lastStableMci )
    {
        int currentStableMci = m_lastStableMci;
		Block currentStableBlock;
		while(currentStableMci < newBlock.m_lastStableMci)
		{
		    currentStableMci++;
		    currentStableBlock = ReturnBlock(m_mainChain[currentStableMci]);
			if (currentStableBlock.GetBlockHeight() != -1)
			    m_stableChain.push_back(currentStableBlock.GetBlockHash());
		}
		m_lastStableMci = currentStableMci;

		double stableTime = Simulator::Now ().GetSeconds ();
		SetStable(currentStableBlock.GetBlockHash(),stableTime);
		
    }

	
	/*COUT_FOR_NODE<<"Advance stable block new block is "<<newBlock.GetBlockHash()<<",stable main chain is :\n";
	for (auto& blockHash : m_stableChain)
	{
	    COUT_FOR_NODE<<blockHash<<",";
	}
	COUT_FOR_NODE<<"\n";*/
	
	return true;
}
bool
Blockchain::SetStable(std::string lsbHash,double stableTime)
{
    std::string stableHash = lsbHash;
    std::queue<std::string> stables;
    std::unordered_set<std::string> handledHashes;
    stables.push(stableHash);
    while(!stables.empty())
    {
      stableHash = stables.front();
      stables.pop();
      if (stableHash == "0/-1/0")
      	  break;
	  if (handledHashes.count(stableHash) > 0)
          continue;
      handledHashes.insert(stableHash);
	  Block stableBlock = ReturnBlock(stableHash);
	  //std::cout<<"Blockchain::SetStable block "<<stableHash<<",block stable time is "<<stableBlock.m_timeStable<<"\n";
	  if (stableBlock.m_timeStable > -0.000001 && stableBlock.m_timeStable < 0.000001)
	  {
	      stableBlock.m_timeStable = stableTime;
		  UpdateBlock(stableBlock);
	  }


      for (auto& parent : stableBlock.m_DagParents)
      {
          stables.push(parent);
      }
    }
}
std::vector<Block>
Blockchain::CheckMissingParent(std::string blockHash)
{
    std::vector<std::string> missingBlocks;
	std::vector<Block> findParentsOrphans;
    for (auto it = m_missingParents.begin();it != m_missingParents.end();it++)
    {
        missingBlocks = it->second;
		auto itBlock = std::find(missingBlocks.begin(),missingBlocks.end(),blockHash);
		if (itBlock != missingBlocks.end())
		{
		    COUT_FOR_NODE<<"CheckMissingParent find "<<it->first<<"'s missing parent "<<blockHash<<"\n";
		    std::string bHash = it->first;
			int index = itBlock - missingBlocks.begin();
			m_missingParents[bHash].erase(m_missingParents[bHash].begin() + index);
			COUT_FOR_NODE<<"CheckMissingParent erase parent "<<blockHash<<"\n";
			if (m_missingParents[bHash].size() == 0)
			{
			    COUT_FOR_NODE<<"CheckMissingParent found orphan "<<bHash<<"\n";
			    for (Block &orphan:m_orphans)
			    {
			        if (orphan.GetBlockHash() == bHash)
			        {
			            findParentsOrphans.push_back(orphan);
						break;
			        }
			    }
			}
			//break;				

		}
    }

    COUT_FOR_NODE<<"CheckMissingParent  findParentsOrphans size is "<<findParentsOrphans.size()<<"\n";
	return findParentsOrphans;
}

/*void 
Blockchain::RemoveMissingParent (const Block& newBlock)
{
  std::vector<Block>::iterator  block_it;

  for (block_it = m_missingParents.begin();  block_it < m_missingParents.end(); block_it++)
  {
    if (newBlock == *block_it)
      break;
  }
  
  if (block_it == m_orphans.end())
  {
    // name not in vector
    return;
  } 
  else
  {
    m_missingParents.erase(block_it);
  }
}*/


/* END:   Added by xuzt, 2019/4/19 */

const Block* 
Blockchain::GetBlockPointer (const Block &newBlock) const
{
  
  for (auto const &block: m_blocks[newBlock.GetBlockHeight()]) 
  {
    if (block == newBlock)
    {
      return &block;
    }
  }
  return nullptr;
}
 
const std::vector<const Block *> 
Blockchain::GetChildrenPointers (const Block &block)
{
  std::vector<const Block *> children;
  std::vector<Block>::iterator  block_it;
  int childrenHeight = block.GetBlockHeight() + 1;
  
  if (childrenHeight > GetBlockchainHeight())
    return children;

  for (block_it = m_blocks[childrenHeight].begin();  block_it < m_blocks[childrenHeight].end(); block_it++)
  {
    if (block.IsParent(*block_it))
    {
      children.push_back(&(*block_it));
    }
  }
  return children;
}


const std::vector<const Block *> 
Blockchain::GetOrphanChildrenPointers (const Block &newBlock)
{
  std::vector<const Block *> children;
  std::vector<Block>::iterator  block_it;

  for (block_it = m_orphans.begin();  block_it < m_orphans.end(); block_it++)
  {
    if (newBlock.IsParent(*block_it))
    {
      children.push_back(&(*block_it));
    }
  }
  return children;
}


const Block* 
Blockchain::GetParent (const Block &block) 
{
  std::vector<Block>::iterator  block_it;
  int parentHeight = block.GetBlockHeight() - 1;

  if (parentHeight > GetBlockchainHeight() || parentHeight < 0)
    return nullptr;
  
  for (block_it = m_blocks[parentHeight].begin();  block_it < m_blocks[parentHeight].end(); block_it++)  {
    if (block.IsChild(*block_it))
    {
      return &(*block_it);
    }
  }

  return nullptr;
}

/* BEGIN: Modified by xuzt, 2019/4/22   DAG simulator support */
const Block* 
Blockchain::GetCurrentTopBlock (void) const
{
  return &m_blocks[m_blocks.size() - 1][0];
  //return &m_blocks[m_maxHeight][0];
}
/* END:   Modified by xuzt, 2019/4/22 */

void 
Blockchain::AddBlock (const Block& newBlock)
{
  //std::cout<<"Add Block "<<newBlock.GetBlockHash()<<"\n";
  if (m_blocks.size() == 0)
  {
    std::vector<Block> newHeight(1, newBlock);
	m_blocks.push_back(newHeight);
  }	
  else if (newBlock.GetBlockHeight() > GetCurrentTopBlock()->GetBlockHeight()) 
  {
    /**
     * The new block has a new blockHeight, so have to create a new vector (row)
     * If we receive an orphan block we have to create the dummy rows for the missing blocks as well
     */
    int dummyRows = newBlock.GetBlockHeight() - GetCurrentTopBlock()->GetBlockHeight() - 1;
	//printf("Add a newer height [%d] block and old height is [%d]\n",newBlock.GetBlockHeight(),GetCurrentTopBlock()->GetBlockHeight());
    for(int i = 0; i < dummyRows; i++)
    {  
      std::vector<Block> newHeight; 
      m_blocks.push_back(newHeight);
    }

	if ( m_blocks.size() < newBlock.GetBlockHeight() + 1 )	
	{
       std::vector<Block> newHeight(1, newBlock);
       m_blocks.push_back(newHeight);
	}
	else
	{
	    m_blocks[newBlock.GetBlockHeight()].push_back(newBlock); 
	}
  }
  else
  {
    /* The new block doesn't have a new blockHeight, so we have to add it in an existing row */
	
    if (m_blocks[newBlock.GetBlockHeight()].size() > 0)
      m_noStaleBlocks++;									

    //printf("Add a block at height[%d]\n",newBlock.GetBlockHeight());
    m_blocks[newBlock.GetBlockHeight()].push_back(newBlock);   
  }

  
  m_totalBlocks++;
  //Block checkBlock = ReturnBlock(newBlock.GetBlockHash());
  //if (checkBlock.GetBlockHeight() == -1)
  	//std::cout<<"Add block "<<newBlock.GetBlockHash()<<" failed\n";
  /* BEGIN: Added by xuzt, 2019/4/19   DAG simulator support */
  /*if ( m_blocks[newBlock.GetBlockHeight()].size() >= 2 )
      m_maxHeight++;*/
  m_maxHeight = newBlock.GetBlockHeight();
  SaveBlock(newBlock);
  /* END:   Added by xuzt, 2019/4/19 */
}


void 
Blockchain::AddOrphan (const Block& newBlock)
{
  if (std::find(m_orphans.begin(),m_orphans.end(),newBlock) == m_orphans.end())
      m_orphans.push_back(newBlock);
}


void 
Blockchain::RemoveOrphan (const Block& newBlock)
{
  std::vector<Block>::iterator  block_it;

  for (block_it = m_orphans.begin();  block_it < m_orphans.end(); block_it++)
  {
    if (newBlock == *block_it)
      break;
  }
  
  if (block_it == m_orphans.end())
  {
    // name not in vector
    return;
  } 
  else
  {
    m_orphans.erase(block_it);
  }
}
/* BEGIN: Added by xuzt, 2019/4/24   DAG simulator support */
bool
Blockchain::BlockCompare(Block a,Block b)
{
    if (a.m_witnessLevel < b.m_witnessLevel 
		 || (a.m_witnessLevel == b.m_witnessLevel && a.GetBlockHeight() < b.GetBlockHeight()) 
		 || (a.m_witnessLevel == b.m_witnessLevel && a.GetBlockHeight() == b.GetBlockHeight() 
		         && a.GetTimeCreated() < b.GetTimeCreated()))
		return true;
	else
		return false;
}

void 
Blockchain::AddFree(const Block& block)
{
    //std::cout<<"Blockchain::AddFree block "<<block.GetBlockHash()<<"\n";
    m_freeBlocks.push_back(block);
	//std::sort(m_freeBlocks.begin(),m_freeBlocks.end(),BlockCompare);
}
void 
Blockchain::RemoveFree(const Block& block)
{
  std::vector<Block>::iterator  block_it;

  for (block_it = m_freeBlocks.begin();  block_it < m_freeBlocks.end(); block_it++)
  {
    if (block == *block_it)
      break;
  }
  
  if (block_it == m_freeBlocks.end())
  {
    // name not in vector
    return;
  } 
  else
  {
    m_freeBlocks.erase(block_it);
  }
}

/* END:   Added by xuzt, 2019/4/24 */

void
Blockchain::PrintOrphans (void)
{
  std::vector<Block>::iterator  block_it;
  
  std::cout << "The orphans are:\n";
  
  for (block_it = m_orphans.begin();  block_it < m_orphans.end(); block_it++)
  {
    std::cout << *block_it << "\n";
  }
  
  std::cout << "\n";
}


int 
Blockchain::GetBlocksInForks (void)
{
  std::vector< std::vector<Block>>::iterator blockHeight_it;
  int count = 0;
  
  for (blockHeight_it = m_blocks.begin(); blockHeight_it < m_blocks.end(); blockHeight_it++) 
  {
    if (blockHeight_it->size() > 1)
      count += blockHeight_it->size();
  }
  
  return count;
}


int 
Blockchain::GetLongestForkSize (void)
{
  std::vector< std::vector<Block>>::iterator   blockHeight_it;
  std::vector<Block>::iterator                 block_it;
  std::map<int, int>                           forkedBlocksParentId;
  std::vector<int>                             newForks; 
  int maxSize = 0;
  int blockHeight = 0;
  for (blockHeight_it = m_blocks.begin(); blockHeight_it < m_blocks.end(); blockHeight_it++,blockHeight++) 
  {
	//printf("GetLongestForkSize block height is %d,block size is %d\r\n",blockHeight,blockHeight_it->size());
    if (blockHeight_it->size() > 1 && forkedBlocksParentId.size() == 0)
    {
      for (block_it = blockHeight_it->begin();  block_it < blockHeight_it->end(); block_it++)
      {
        forkedBlocksParentId[block_it->GetMinerId()] = 1;
		//printf("GetLongestForkSize 1 minerid is %d\r\n",block_it->GetMinerId());
      }
    }
    else if (blockHeight_it->size() > 1)
    {
      for (block_it = blockHeight_it->begin();  block_it < blockHeight_it->end(); block_it++)
      {
        std::map<int, int>::iterator mapIndex = forkedBlocksParentId.find(block_it->GetParentBlockMinerId());
        
        if(mapIndex != forkedBlocksParentId.end())
        {
          forkedBlocksParentId[block_it->GetMinerId()] = mapIndex->second + 1;
		  //printf("GetLongestForkSize 2 minerid is %d,forkedBlocksParentId size is %d,parent miner id is %d\r\n",block_it->GetMinerId(),forkedBlocksParentId[block_it->GetMinerId()],block_it->GetParentBlockMinerId());
          if(block_it->GetMinerId() != mapIndex->first)
            forkedBlocksParentId.erase(mapIndex);	
          newForks.push_back(block_it->GetMinerId());		  
        }
        else
        {
          //printf("GetLongestForkSize 3 minerid is %d\r\n",block_it->GetMinerId());
          forkedBlocksParentId[block_it->GetMinerId()] = 1;
        }		  
      }
	  
      for (auto &block : forkedBlocksParentId)
      {
       if (std::find(newForks.begin(), newForks.end(), block.first) == newForks.end() )
       {
         if(block.second > maxSize)
           maxSize = block.second;
         forkedBlocksParentId.erase(block.first);
       }
	  }
    }
    else if (blockHeight_it->size() == 1 && forkedBlocksParentId.size() > 0)
    {

      for (auto &block : forkedBlocksParentId)
      {
        if(block.second > maxSize)
          maxSize = block.second;
      }
	
      forkedBlocksParentId.clear();
    }
  }
  
  for (auto &block : forkedBlocksParentId)
  {
    if(block.second > maxSize)
      maxSize = block.second;
  }
   
  return maxSize;
}


/* BEGIN: Modified by xuzt, 2019/4/19   DAG simulator support */
bool operator== (const Block &block1, const Block &block2)
{
  //if (block1.GetBlockHeight() == block2.GetBlockHeight() && block1.GetMinerId() == block2.GetMinerId())
  if (block1.GetBlockHash() == block2.GetBlockHash())
    return true;
  else
    return false;
}
/* END:   Modified by xuzt, 2019/4/19 */

bool operator== (const BitcoinChunk &chunk1, const BitcoinChunk &chunk2)
{
  if (chunk1.GetBlockHeight() == chunk2.GetBlockHeight() && chunk1.GetMinerId() == chunk2.GetMinerId() && chunk1.GetChunkId() == chunk2.GetChunkId())
    return true;
  else
    return false;
}

bool operator< (const BitcoinChunk &chunk1, const BitcoinChunk &chunk2)
{
  if (chunk1.GetBlockHeight() < chunk2.GetBlockHeight())
    return true;
  else if (chunk1.GetBlockHeight() == chunk2.GetBlockHeight() && chunk1.GetMinerId() < chunk2.GetMinerId())
    return true;
  else if (chunk1.GetBlockHeight() == chunk2.GetBlockHeight() && chunk1.GetMinerId() == chunk2.GetMinerId() && chunk1.GetChunkId() < chunk2.GetChunkId())
    return true;
  else
    return false;
}

std::ostream& operator<< (std::ostream &out, const Block &block)
{
/* BEGIN: Modified by xuzt, 2019/4/23   DAG simulator support */
    out << "(m_blockHeight: " << block.GetBlockHeight() << ", " <<
        "m_minerId: " << block.GetMinerId() << ", " <<
        "m_parentBlockMinerId: " << block.GetParentBlockMinerId() << ", " <<
        "m_blockSizeBytes: " << block.GetBlockSizeBytes() << ", " <<
        "m_timeCreated: " << block.GetTimeCreated() << ", " <<
        "m_timeReceived: " << block.GetTimeReceived() << ", " <<
        "m_receivedFromIpv4: " << block.GetReceivedFromIpv4() << ", " << 
        "m_dagParents: ";
   for (auto &pblock : block.GetDagParents())
   {
       out << pblock << "\n";
   }
   out<<"m_dagChildrens: ";
   for (auto &child : block.m_DagChildrens)
   {
       out << child << "\n";
   }
   out << ")";
/* END:   Modified by xuzt, 2019/4/23 */
    return out;
}

std::ostream& operator<< (std::ostream &out, const BitcoinChunk &chunk)
{

    out << "(m_blockHeight: " << chunk.GetBlockHeight() << ", " <<
        "m_minerId: " << chunk.GetMinerId() << ", " <<
        "chunkId: " << chunk.GetChunkId() << ", " <<
        "m_parentBlockMinerId: " << chunk.GetParentBlockMinerId() << ", " <<
        "m_blockSizeBytes: " << chunk.GetBlockSizeBytes() << ", " <<
        "m_timeCreated: " << chunk.GetTimeCreated() << ", " <<
        "m_timeReceived: " << chunk.GetTimeReceived() << ", " <<
        "m_receivedFromIpv4: " << chunk.GetReceivedFromIpv4() <<
        ")";
    return out;
}

std::ostream& operator<< (std::ostream &out, Blockchain &blockchain)
{
  
  std::vector< std::vector<Block>>::iterator blockHeight_it;
  std::vector<Block>::iterator  block_it;
  int i;
  
  for (blockHeight_it = blockchain.m_blocks.begin(), i = 0; blockHeight_it < blockchain.m_blocks.end(); blockHeight_it++, i++) 
  {
    out << "  BLOCK HEIGHT " << i << ":\n";
    for (block_it = blockHeight_it->begin();  block_it < blockHeight_it->end(); block_it++)
    {
      //out << *block_it << "\n";
      out << block_it->GetBlockHash() <<",";
    }
	for (auto orphanIt = blockchain.m_orphans.begin();  orphanIt < blockchain.m_orphans.end();orphanIt++)
    {
      //out << *block_it << "\n";
      if (orphanIt->GetBlockHeight() == i)
      out << orphanIt->GetBlockHash() <<"[orphan],";
    }
  }
  
  return out;
}

const char* getMessageName(enum Messages m) 
{
  switch (m) 
  {
    case INV: return "INV";
    case GET_HEADERS: return "GET_HEADERS";
    case HEADERS: return "HEADERS";
    case GET_BLOCKS: return "GET_BLOCKS";
    case BLOCK: return "BLOCK";
    case GET_DATA: return "GET_DATA";
    case NO_MESSAGE: return "NO_MESSAGE";
    case EXT_INV: return "EXT_INV";
    case EXT_GET_HEADERS: return "EXT_GET_HEADERS";
    case EXT_HEADERS: return "EXT_HEADERS";
    case EXT_GET_BLOCKS: return "EXT_GET_BLOCKS";
    case CHUNK: return "CHUNK";
    case EXT_GET_DATA: return "EXT_GET_DATA";
  }
}

const char* getMinerType(enum MinerType m)
{
  switch (m) 
  {
    case NORMAL_MINER: return "NORMAL_MINER";
    case SIMPLE_ATTACKER: return "SIMPLE_ATTACKER";
    case SELFISH_MINER: return "SELFISH_MINER";
    case SELFISH_MINER_TRIALS: return "SELFISH_MINER_TRIALS";
  }
}

const char* getBlockBroadcastType(enum BlockBroadcastType m)
{
  switch (m) 
  {
    case STANDARD: return "STANDARD";
    case UNSOLICITED: return "UNSOLICITED";
    case RELAY_NETWORK: return "RELAY_NETWORK";
    case UNSOLICITED_RELAY_NETWORK: return "UNSOLICITED_RELAY_NETWORK";
  }
}

const char* getProtocolType(enum ProtocolType m)
{
  switch (m) 
  {
    case STANDARD_PROTOCOL: return "STANDARD_PROTOCOL";
    case SENDHEADERS: return "SENDHEADERS";
  }
}

const char* getCryptocurrency(enum Cryptocurrency m)
{
  switch (m) 
  {
    case BITCOIN: return "BITCOIN";
    case LITECOIN: return "LITECOIN";
    case DOGECOIN: return "DOGECOIN";
  }
}

const char* getBitcoinRegion(enum BitcoinRegion m)
{
  switch (m) 
  {
    case ASIA_PACIFIC: return "ASIA_PACIFIC";
    case AUSTRALIA: return "AUSTRALIA";
    case EUROPE: return "EUROPE";
    case JAPAN: return "JAPAN";
    case NORTH_AMERICA: return "NORTH_AMERICA";
    case SOUTH_AMERICA: return "SOUTH_AMERICA";
    case OTHER: return "OTHER";
  }
}


enum BitcoinRegion getBitcoinEnum(uint32_t n)
{
  switch (n) 
  {
    case 0: return NORTH_AMERICA;
    case 1: return EUROPE;
    case 2: return SOUTH_AMERICA;
    case 3: return ASIA_PACIFIC;
    case 4: return JAPAN;
    case 5: return AUSTRALIA;
    case 6: return OTHER;
  }
}
}// Namespace ns3
