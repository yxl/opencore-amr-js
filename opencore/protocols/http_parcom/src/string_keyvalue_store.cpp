/* ------------------------------------------------------------------
 * Copyright (C) 1998-2009 PacketVideo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */
// class for encapsulating string based key-value pair handlings
#include "string_keyvalue_store.h"
#include "oscl_variablesize_mem_pool.h"

#define STRINGKEYVALUESTOREOVERHEAD 100

uint32 StringKeyValueStore::getCurrentMemoryUsage()
{
    return iVariableSizeMemPool->getCurrMemoryUsage();
}

uint32 StringKeyValueStore::getStoreSize()
{
    return iVariableSizeMemPool->getPoolSize();
}

uint32 StringKeyValueStore::getAvailableSize()
{
    return (iVariableSizeMemPool->getTotalAvailableSize() - STRINGKEYVALUESTOREOVERHEAD);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool StringKeyValueStore::storeNewKeyValueItem(const char *aItem, const int32 aItemLength, char *&aNewLocation)
{
    int32 err = 0;
    OSCL_TRY(err, aNewLocation = (char*)iVariableSizeMemPool->allocate(aItemLength + 1););
    if (err || aNewLocation == NULL) return false;
    oscl_memcpy(aNewLocation, aItem, aItemLength);
    aNewLocation[aItemLength] = 0;
    return true;
}

void StringKeyValueStore::releaseOldKeyValueItem(const char *aItem, const int32 aItemLength)
{
    OSCL_UNUSED_ARG(aItemLength);

    int32 err = 0;
    OSCL_TRY(err, iVariableSizeMemPool->deallocate((OsclAny*)aItem););
}

int32 StringKeyValueStore::addKeyToStore(const StrCSumPtrLen &aNewKey, int32 tableIndex)
{
    if (!iFieldKeys[tableIndex].empty()) return StringKeyValueStore_Success;

    // new field
    // save this checksum
    int32 err = 0;
    OSCL_TRY(err, iFieldKeyTableIndexVector.push_back(tableIndex));
    if (err) return StringKeyValueStore_NoMemory;

    // add new field key
    char *newLocation = NULL;
    int32 aKeyLength = aNewKey.length();
    if (!storeNewKeyValueItem(aNewKey.c_str(), aKeyLength, newLocation)) return StringKeyValueStore_NoMemory;
    iFieldKeys[tableIndex].setPtrLen(newLocation, aKeyLength);
    iTotalKeyValueLength += aKeyLength;
    return StringKeyValueStore_Success;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int32 StringKeyValueStore::addKeyValuePair(const StrCSumPtrLen &aNewKey, const StrPtrLen &aNewValue, const bool aNeedReplaceOldValue)
{
    int32 tableIndex;
    if ((tableIndex = getHashTableIndex(aNewKey, false)) < 0) return false; // false in getHashTableIndex means for adding an element

    // add the field key if it is new
    if (addKeyToStore(aNewKey, tableIndex)) return StringKeyValueStore_NoMemory;

    // add or append new field value
    char *newLocation = NULL;
    int32 aValueLength = aNewValue.length();
    if (!storeNewKeyValueItem(aNewValue.c_str(), aValueLength, newLocation)) return StringKeyValueStore_NoMemory;

    if (iFieldVals[tableIndex].length() == 0)
    {
        // empty value field

        iFieldVals[tableIndex].setPtrLen(newLocation, aValueLength);
    }
    else if (aNeedReplaceOldValue)
    {
        // overrite
        // first, release the memory for the old value
        releaseOldKeyValueItem(iFieldVals[tableIndex].c_str(), aValueLength);
        // then replace the old value with the new value
        iTotalKeyValueLength -= iFieldVals[tableIndex].length();
        iFieldVals[tableIndex].setPtrLen(newLocation, aValueLength);
        iTotalNumberOfKeyValuePairs--; // for the following iTotalNumberOfKeyValuePairs++;
    }
    else
    {
        // append
        // create StrCSumPtrLenWrapper
        StrCSumPtrLenWrapper aMewValueWrapper(newLocation, aValueLength);
        int32 err = 0;
        OSCL_TRY(err, iStrCSumPtrLenWrapperVector.push_back(aMewValueWrapper));
        if (err) return StringKeyValueStore_NoMemory;
        // link this new field value to the existing field key
        iFieldKeys[tableIndex].push_back(&iStrCSumPtrLenWrapperVector[iStrCSumPtrLenWrapperVector.size()-1]);
        iTotalKeyValueLength += iFieldKeys[tableIndex].length();  // field key still needs to be taken into account
    }

    // update iTotalKeyValueLength and iTotalNumberOfKeyValuePairs
    iTotalKeyValueLength += aValueLength;
    iTotalNumberOfKeyValuePairs++;
    return StringKeyValueStore_Success;
}

int32 StringKeyValueStore::addKeyValuePair(const StrCSumPtrLen &aNewKey, const char *aNewValue, const bool aNeedReplaceOldValue)
{
    if (!aNewValue) return StringKeyValueStore_Failure;
    StrPtrLen newValue(aNewValue);
    return addKeyValuePair(aNewKey, newValue, aNeedReplaceOldValue);
}

int32 StringKeyValueStore::addKeyValuePair(const char *aNewKey, const uint32 aKeyLength, const char *aNewValue, const uint32 aValueLength,
        const bool aNeedReplaceOldValue)
{
    StrCSumPtrLen newKey(aNewKey, aKeyLength);
    StrPtrLen newValue(aNewValue, aValueLength);
    return addKeyValuePair(newKey, newValue, aNeedReplaceOldValue);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// get
uint32 StringKeyValueStore::getCurrentKeyList(StrPtrLen *&aFieldKeyList, const uint32 aListSize)
{
    uint32 requestListSize = (aListSize == 0 ? iFieldKeyTableIndexVector.size() :
                              OSCL_MIN(aListSize, iFieldKeyTableIndexVector.size()));
    uint32 i;
    for (i = 0; i < requestListSize; i++)
    {
        aFieldKeyList[i].setPtrLen(iFieldKeys[iFieldKeyTableIndexVector[i]].c_str(),
                                   iFieldKeys[iFieldKeyTableIndexVector[i]].length());
    }

    return requestListSize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool StringKeyValueStore::getValueByKey(const StrCSumPtrLen &aKey, StrPtrLen &aValue, uint32 index)
{
    // reset aValue
    aValue.setPtrLen("", 0);

    int32 tableIndex = getHashTableIndex(aKey);
    if (tableIndex < 0 || tableIndex > KEYVALUESTORE_HASH_TABLE_SIZE_FOR_KEYS) return false; // no such key in the store

    if (index == 0)
    {
        aValue = iFieldVals[tableIndex];
        return true;
    }
    else
    {
        // index > 0
        uint32 count = 1;
        StrCSumPtrLenWrapper *pStrWrapper = iFieldKeys[tableIndex].getNext();
        while (count < index && pStrWrapper != NULL)
        {
            pStrWrapper = pStrWrapper->getNext();
            count++;
        }

        if (!pStrWrapper) return false;
        if (count == index && pStrWrapper != NULL)
        {
            // find the right one
            aValue.setPtrLen(pStrWrapper->c_str(), pStrWrapper->length());
            return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
uint32 StringKeyValueStore::getNumberOfValuesByKey(const StrCSumPtrLen &aKey)
{
    int32 tableIndex = getHashTableIndex(aKey);
    if (tableIndex < 0) return 0; // no such key in the store

    uint32 count = 1;
    StrCSumPtrLenWrapper *pStrWrapper = iFieldKeys[tableIndex].getNext();
    while (pStrWrapper)
    {
        count++;
        pStrWrapper = pStrWrapper->getNext();
    }
    return count;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// search
bool StringKeyValueStore::isKeyValueAvailable(const StrCSumPtrLen &aKey)
{
    return (getHashTableIndex(aKey) >= 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// remove
bool StringKeyValueStore::removeKeyValuePair(const StrCSumPtrLen &aKey)
{
    // update iTotalNumberOfKeyValuePairs
    uint32 numValues = getNumberOfValuesByKey(aKey);
    if (numValues == 0) return true; //false; // no such key
    iTotalNumberOfKeyValuePairs -= numValues;

    // update iTotalKeyValueLength
    int32 tableIndex = getHashTableIndex(aKey);
    iTotalKeyValueLength -= (iFieldKeys[tableIndex].length() * numValues + iFieldVals[tableIndex].length());
    StrCSumPtrLenWrapper *pStrWrapper = iFieldKeys[tableIndex].getNext();
    while (pStrWrapper)  // for the mulitple values
    {
        // release the key and all appending values
        releaseOldKeyValueItem(pStrWrapper->c_str(), pStrWrapper->length());
        iTotalKeyValueLength -= pStrWrapper->length();
        pStrWrapper = pStrWrapper->getNext();
    }
    // release the value
    releaseOldKeyValueItem(iFieldVals[tableIndex].c_str(), iFieldVals[tableIndex].length());

    // remove the index in iFieldKeyTableIndexVector
    uint32 i = 0;
    for (i = 0; i < iFieldKeyTableIndexVector.size(); i++)
    {
        if (iFieldKeys[iFieldKeyTableIndexVector[i]].isCIEquivalentTo(aKey))
        {
            iFieldKeyTableIndexVector.erase(iFieldKeyTableIndexVector.begin() + i);
        }
    }

    // clear key-value
    iFieldKeys[tableIndex].clear();
    iFieldVals[tableIndex].setPtrLen("", 0);

    // note that we need to upgrade to use resizable memory pool to return the memory for the key-value pair;
    // otherwise we waste the memory.
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// clear
void StringKeyValueStore::clear()
{
    iTotalNumberOfKeyValuePairs = 0;
    iTotalKeyValueLength		= 0;
    iNumConflicts				= 0;

    // clear field key and value tables
    uint32 i;
    for (i = 0; i < KEYVALUESTORE_HASH_TABLE_SIZE_FOR_KEYS; i++)
    {
        iFieldKeys[i].clear();
        iFieldVals[i].setPtrLen("", 0);
    }
    iStrCSumPtrLenWrapperVector.clear();
    iFieldKeyTableIndexVector.clear();
    if (iVariableSizeMemPool) iVariableSizeMemPool->clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// copy
bool StringKeyValueStore::copy(StringKeyValueStore& aStore)
{
    uint32 numKeyValuePairs = aStore.getNumberOfKeyValuePairs();
    uint32 numKeys = aStore.getNumberOfKeys();
    if (numKeyValuePairs == 0 || numKeys == 0) return true;
    OSCL_ASSERT(numKeyValuePairs >= numKeys);

    StrPtrLen *keyList = OSCL_ARRAY_NEW(StrPtrLen, numKeys);
    if (!keyList) return false;
    aStore.getCurrentKeyList(keyList);
    for (uint32 i = 0; i < numKeyValuePairs; i++)
    {
        uint32 numValuesByKey = aStore.getNumberOfValuesByKey(keyList[i]);
        for (uint32 j = 0; j < numValuesByKey; j++)
        {
            StrPtrLen fieldValue;
            if (!aStore.getValueByKey(keyList[i], fieldValue, j))
            {
                OSCL_ARRAY_DELETE(keyList);
                return false;
            }
            if (addKeyValuePair(keyList[i], fieldValue))
            {
                OSCL_ARRAY_DELETE(keyList);
                return false;
            }
        }
    }

    OSCL_ARRAY_DELETE(keyList);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// factory method
StringKeyValueStore* StringKeyValueStore::create(const uint32 aStoreSize)
{
    StringKeyValueStore *store = OSCL_NEW(StringKeyValueStore, ());
    if (!store) return NULL;
    if (!store->construct(aStoreSize))
    {
        OSCL_DELETE(store);
        return NULL;
    }
    return store;
}

bool StringKeyValueStore::construct(const uint32 aStoreSize)
{
    clear();

    // create two vectors
    int32 err = 0;
    OSCL_TRY(err, iStrCSumPtrLenWrapperVector.reserve(KEYVALUESTORE_VECTOR_RESERVE_VALUE);
             iFieldKeyTableIndexVector.reserve(KEYVALUESTORE_VECTOR_RESERVE_VALUE);
            );
    if (err)
    {
        iStrCSumPtrLenWrapperVector.clear();
        iFieldKeyTableIndexVector.clear();
        return false;
    }

    // create iVariableSizeMemPool
    OSCL_TRY(err, iVariableSizeMemPool = new OsclMemPoolVariableChunkAllocator(aStoreSize););
    if (err || iVariableSizeMemPool == NULL) return false;


    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// destructor
StringKeyValueStore::~StringKeyValueStore()
{
    clear();

    // delete memory pool
    OSCL_DELETE(iVariableSizeMemPool);
    iVariableSizeMemPool = NULL;
}

////////////////////////////////////////////////////////////////////////////////////
uint32 StringKeyValueStore::calculateChecksum(const char *aBuffer, uint32 aBufferLength)
{
    uint32 checkSum = 0;
    for (uint32 i = 0; i < aBufferLength; i++)
    {
        if (oscl_isLetter(aBuffer[i]))
        {
            checkSum += (uint32)(aBuffer[i] | OSCL_ASCII_CASE_MAGIC_BIT);
        }
        else
        {
            checkSum += aBuffer[i];
        }
    }

    return (checkSum % KEYVALUESTORE_HASH_TABLE_SIZE_FOR_KEYS) / 2;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int32 StringKeyValueStore::getHashTableIndex(const StrCSumPtrLen &aKey, const bool aFindKey)
{
    StrCSumPtrLenWrapper keyWrapper(aKey);
    int32 aTableIndex = keyWrapper.getChecksum();

    if (iFieldKeys[aTableIndex].empty())
    {
        if (!aFindKey)
        {
            return aTableIndex; // for add, new key to be added
        }
        return query(aKey); // for search, need to check linear search table
    }

    if (iFieldKeys[aTableIndex].isCIEquivalentTo(aKey))
    {
        return aTableIndex;
    }
    int32 index = query(aKey);
    if (aFindKey)
    {
        return index; // for search
    }
    // for add
    if (index >= 0)
    {
        return index; // find an existing one to add
    }
    // create a new index for this new conllision
    if (iNumConflicts + 1 >= KEYVALUESTORE_HASH_TABLE_SIZE_FOR_KEYS / 2)
    {
        return -1;
    }
    return KEYVALUESTORE_HASH_TABLE_SIZE_FOR_KEYS / 2 + (iNumConflicts++);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int32 StringKeyValueStore::query(const StrCSumPtrLen &aKey)
{
    StrCSumPtrLenWrapper *LSTable = iFieldKeys + KEYVALUESTORE_HASH_TABLE_SIZE_FOR_KEYS / 2; // LSTable= Linear Search Table
    uint32 i = 0;
    for (i = 0; i < iNumConflicts; i++)
    {
        if (LSTable[i].isCIEquivalentTo(aKey))
        {
            return (int32)(KEYVALUESTORE_HASH_TABLE_SIZE_FOR_KEYS / 2 + i);
        }
    }
    return -1;
}

