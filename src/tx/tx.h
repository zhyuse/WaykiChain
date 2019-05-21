// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2017-2019 The WaykiChain Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php

#ifndef COIN_BASETX_H
#define COIN_BASETX_H

#include "commons/serialize.h"
#include "commons/uint256.h"
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
#include "accounts/account.h"
#include "accounts/id.h"
#include "persistence/contractdb.h"

#include <boost/variant.hpp>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

class CTxUndo;
class CValidationState;
class CAccountViewCache;
class CScriptDB;
class CTransactionDBCache;
class CScriptDBViewCache;
class CScriptDBOperLog;

typedef vector<unsigned char> vector_unsigned_char;

static const int nTxVersion1 = 1;
static const int nTxVersion2 = 2;

#define SCRIPT_ID_SIZE (6)

enum TxType : unsigned char {
    BLOCK_REWARD_TX     = 1,  //!< Miner Block Reward Tx
    ACCOUNT_REGISTER_TX = 2,  //!< Account Registeration Tx
    BCOIN_TRANSFER_TX   = 3,  //!< BaseCoin Transfer Tx
    CONTRACT_INVOKE_TX  = 4,  //!< Contract Invocation Tx
    CONTRACT_DEPLOY_TX  = 5,  //!< Contract Deployment Tx
    DELEGATE_VOTE_TX    = 6,  //!< Vote Delegate Tx
    COMMON_MTX          = 7,  //!< Multisig Tx

    /******** Begin of Stable Coin TX Type Enums ********/
    CDP_OPEN_TX         = 11,  //!< CDP Collateralize Tx
    CDP_REFUEL_TX       = 12,  //!< CDP Refuel Tx
    CDP_REDEMP_TX       = 13,  //!< CDP Redemption Tx (partial or full)
    CDP_LIQUIDATE_TX    = 14,  //!< CDP Liquidation Tx (partial or full)

    PRICE_FEED_TX       = 22,  //!< Price Feed Tx: WICC/USD | MICC/WUSD | WUSD/USD

    SFC_PARAM_MTX       = 31,  //!< StableCoin Fund Committee invokes Param Set/Update MulSigTx
    SFC_GLOBAL_HALT_MTX = 32,  //!< StableCoin Fund Committee invokes Global Halt CDP Operations MulSigTx
    SFC_GLOBAL_SETTLE_MTX =33,  //!< StableCoin Fund Committee invokes Global Settle Operation MulSigTx

    SCOIN_TRANSFER_TX   = 41,  //!< StableCoin Transfer Tx
    FCOIN_TRANSFER_TX   = 42,  //!< FundCoin Transfer Tx
    FCOIN_STAKE_TX      = 43,  //!< Stake Fund Coin in order to become a price feeder

    DEX_WICC_FOR_MICC_TX = 51,  //!< DEX: owner sells WICC for MICC Tx
    DEX_MICC_FOR_WICC_TX = 52,  //!< DEX: owner sells MICC for WICC Tx
    DEX_WICC_FOR_WUSD_TX = 53,  //!< DEX: owner sells WICC for WUSD Tx
    DEX_WUSD_FOR_WICC_TX = 54,  //!< DEX: owner sells WUSD for WICC Tx
    DEX_MICC_FOR_WUSD_TX = 55,  //!< DEX: owner sells MICC for WUSD Tx
    DEX_WUSD_FOR_MICC_TX = 56,  //!< DEX: owner sells WUSD for MICC Tx
    /******** End of Stable Coin Enums ********/

    NULL_TX = 0 //!< NULL_TX
};

static const unordered_map<unsigned char, string> kTxTypeMap = {
    { BLOCK_REWARD_TX,      "BLOCK_REWARD_TX" },
    { ACCOUNT_REGISTER_TX,  "ACCOUNT_REGISTER_TX" },
    { BCOIN_TRANSFER_TX,    "BCOIN_TRANSFER_TX" },
    { CONTRACT_INVOKE_TX,   "CONTRACT_INVOKE_TX" },
    { CONTRACT_DEPLOY_TX,   "CONTRACT_DEPLOY_TX" },
    { DELEGATE_VOTE_TX,     "DELEGATE_VOTE_TX" },
    { COMMON_MTX,           "COMMON_MTX"},
    { CDP_OPEN_TX,          "CDP_OPEN_TX" },
    { CDP_REFUEL_TX,        "CDP_REFUEL_TX" },
    { CDP_REDEMP_TX,        "CDP_REDEMP_TX" },
    { CDP_LIQUIDATE_TX,     "CDP_LIQUIDATE_TX" },
    { PRICE_FEED_TX,        "PRICE_FEED_TX" },
    { SFC_PARAM_MTX,        "SFC_PARAM_MTX" },
    { SFC_GLOBAL_HALT_MTX,  "SFC_GLOBAL_HALT_MTX" },
    { SFC_GLOBAL_SETTLE_MTX,"SFC_GLOBAL_SETTLE_MTX" },
    { DEX_WICC_FOR_MICC_TX, "DEX_WICC_FOR_MICC_TX" },
    { DEX_MICC_FOR_WICC_TX, "DEX_MICC_FOR_WICC_TX" },
    { DEX_WICC_FOR_WUSD_TX, "DEX_WICC_FOR_WUSD_TX" },
    { DEX_WUSD_FOR_WICC_TX, "DEX_WUSD_FOR_WICC_TX" },
    { DEX_MICC_FOR_WUSD_TX, "DEX_MICC_FOR_WUSD_TX" },
    { DEX_WUSD_FOR_MICC_TX, "DEX_WUSD_FOR_MICC_TX" },
    { NULL_TX,              "NULL_TX" },
};

string GetTxType(unsigned char txType);

class CBaseTx {
public:
    static uint64_t nMinTxFee;
    static uint64_t nMinRelayTxFee;
    static uint64_t nDustAmountThreshold;
    static const int CURRENT_VERSION = nTxVersion1;

    int nVersion;
    unsigned char nTxType;
    mutable CUserID txUid;
    int nValidHeight;
    uint64_t llFees;
    vector_unsigned_char signature;

    uint64_t nRunStep;        //!< only in memory
    int nFuelRate;            //!< only in memory
    mutable uint256 sigHash;  //!< only in memory

public:
    CBaseTx(const CBaseTx &other) { *this = other; }

    CBaseTx(int nVersionIn, TxType nTxTypeIn, CUserID txUidIn, int nValidHeightIn, uint64_t llFeesIn) :
        nVersion(nVersionIn), nTxType(nTxTypeIn), txUid(txUidIn), nValidHeight(nValidHeightIn), llFees(llFeesIn),
        nRunStep(0), nFuelRate(0) {}

    CBaseTx(TxType nTxTypeIn, CUserID txUidIn, int nValidHeightIn, uint64_t llFeesIn) :
        nVersion(CURRENT_VERSION), nTxType(nTxTypeIn), txUid(txUidIn), nValidHeight(nValidHeightIn), llFees(llFeesIn),
        nRunStep(0), nFuelRate(0) {}

    CBaseTx(int nVersionIn, TxType nTxTypeIn) :
        nVersion(nVersionIn), nTxType(nTxTypeIn),
        nValidHeight(0), llFees(0), nRunStep(0), nFuelRate(0) {}

    CBaseTx(TxType nTxTypeIn) :
        nVersion(CURRENT_VERSION), nTxType(nTxTypeIn),
        nValidHeight(0), llFees(0), nRunStep(0), nFuelRate(0) {}

    virtual ~CBaseTx() {}

    virtual uint64_t GetFee() const { return llFees; }
    virtual uint256 GetHash() const { return ComputeSignatureHash(); };
    virtual unsigned int GetSerializeSize(int nType, int nVersion) const { return 0; };

    virtual uint64_t GetFuel(int nfuelRate);
    virtual double GetPriority() const { return llFees / GetSerializeSize(SER_NETWORK, PROTOCOL_VERSION); };
    virtual uint64_t GetValue() const { return 0; };


    virtual bool GetAddress(std::set<CKeyID> &vAddr,
                        CAccountViewCache &view,
                        CScriptDBViewCache &scriptDB)                       = 0;
    virtual uint256 ComputeSignatureHash(bool recalculate = false) const    = 0;
    virtual std::shared_ptr<CBaseTx> GetNewInstance()                       = 0;
    virtual string ToString(CAccountViewCache &view) const                  = 0;
    virtual Object ToJson(const CAccountViewCache &AccountView) const       = 0;
    virtual bool CheckTx(CValidationState &state,
                        CAccountViewCache &view,
                        CScriptDBViewCache &scriptDB)                       = 0;
    virtual bool ExecuteTx(int nIndex, CAccountViewCache &view,
                        CValidationState &state,
                        CTxUndo &txundo, int nHeight,
                        CTransactionDBCache &txCache,
                        CScriptDBViewCache &scriptDB)                       = 0;
    virtual bool UndoExecuteTx(int nIndex, CAccountViewCache &view,
                        CValidationState &state,
                        CTxUndo &txundo, int nHeight,
                        CTransactionDBCache &txCache,
                        CScriptDBViewCache &scriptDB)                       = 0;

    int GetFuelRate(CScriptDBViewCache &scriptDB);
    bool IsValidHeight(int nCurHeight, int nTxCacheHeight) const;
    bool IsCoinBase() { return (nTxType == BLOCK_REWARD_TX); }

protected:
    bool CheckMinTxFee(const uint64_t llFees) const;
    bool CheckSignatureSize(const vector<unsigned char> &signature) const ;
};

class CTxUndo {
public:
    uint256 txHash;
    vector<CAccountLog> vAccountLog;
    vector<CScriptDBOperLog> vScriptOperLog;
    IMPLEMENT_SERIALIZE(
        READWRITE(txHash);
        READWRITE(vAccountLog);
        READWRITE(vScriptOperLog);)

public:
    bool GetAccountOperLog(const CKeyID &keyId, CAccountLog &accountLog);
    void Clear() {
        txHash = uint256();
        vAccountLog.clear();
        vScriptOperLog.clear();
    }
    string ToString() const;
};

#endif //COIN_BASETX_H