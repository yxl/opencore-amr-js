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
#ifndef STRING_KEYVALUE_STORE_H_INCLUDED
#define STRING_KEYVALUE_STORE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif
#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif
#ifndef OSCL_STR_PTR_LEN_H_INCLUDED
#include "oscl_str_ptr_len.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

#define KEYVALUESTORE_HASH_TABLE_SIZE_FOR_KEYS 1000
#define KEYVALUESTORE_MAX_SIZE 4000					// same as the RTSP parcom, but this size shouldn't include entity body size, which the composer library has no control
#define KEYVALUESTORE_VECTOR_RESERVE_VALUE 10		// for iStrCSumPtrLenWrapperVector.reserve()


// This StrCSumPtrLen wrapper wraps StrCSumPtrLen with the following new features.
// (1) changed checksum calculation algorithm to make checksum as an identifier to differentiate different StrCSumPtrLen object.
//	   This will be used in quick string comparison/search for field key of HTTP header.
//	   Note that in StrCSumPtrLen, the checksum is calulated as the sum of the ASCII codes of all string charaters (lowercase).
//     And this is not quite efficient. The modified version is contrained within 500 (see the following getChecksum()), with tiny
//	   performance loss.
// (2) support simplified link list. This is mainly used for multiple same header field cases (i.e. same field key, but multiple field
//     values).

struct StrCSumPtrLenWrapper
{
public:
    // constructor
    StrCSumPtrLenWrapper() : iNext(NULL)
    {
        ;
    }

    // copy constructor
    StrCSumPtrLenWrapper(const StrCSumPtrLen &x) : iStr(x), iNext(NULL)
    {
        ;
    }
    StrCSumPtrLenWrapper(const char *x) : iStr(x), iNext(NULL)
    {
        ;
    }
    StrCSumPtrLenWrapper(const char *x, const uint32 len) : iStr(x, len), iNext(NULL)
    {
        ;
    }

    // destructor
    ~StrCSumPtrLenWrapper()
    {
        clear();
    }

    // use checksum (with some changes) as an identifier, will be used to differentiate different StrCSumPtrLen objects.
    uint32 getChecksum()
    {
        // algorithm: checksum = (checksum % 1000) / 2
        return (uint32)((uint32)iStr.getCheckSum() % KEYVALUESTORE_HASH_TABLE_SIZE_FOR_KEYS) / 2;
    }

    // add to linked list
    void push_back(StrCSumPtrLenWrapper *aStrLinkTo)
    {
        StrCSumPtrLenWrapper **pWrapper = &iNext;
        while (*pWrapper) pWrapper = &((*pWrapper)->iNext);
        *pWrapper = aStrLinkTo;
    }

    // get next element
    StrCSumPtrLenWrapper *getNext()
    {
        return iNext;
    }

    // clear
    void clear()
    {
        iStr.setPtrLen("", 0);
        iNext = NULL;
    }

    // empty
    bool empty()
    {
        return (iNext == NULL && iStr.length() == 0);
    }

    // wrap some functions of StrCSumPtrLen
    const char* c_str() const
    {
        return iStr.c_str();
    }
    uint32 length() const
    {
        return (uint32)iStr.length();
    }
    uint32 size() const
    {
        return (uint32)iStr.size();
    }
    bool isCIEquivalentTo(const StrCSumPtrLen& rhs) const
    {
        return (iStr.isCIEquivalentTo(rhs) > 0);
    }
    bool isCIEquivalentTo(const char*s, const uint32 len) const
    {
        StrCSumPtrLen str(s, len);
        return (iStr.isCIEquivalentTo(str) > 0);
    }
    void setPtrLen(const char* newPtr, uint32 newLen)
    {
        iStr.setPtrLen(newPtr, newLen);
        iNext = NULL;
    }

    void setPtrLen(const StrPtrLen &aString)
    {
        iStr.setPtrLen(aString.c_str(), aString.length());
        iNext = NULL;
    }

    // operator =
    StrCSumPtrLenWrapper& operator=(const StrCSumPtrLen& rhs)
    {
        iStr = rhs;
        iNext = NULL;
        return *this;
    }

    StrCSumPtrLenWrapper& operator=(const StrPtrLen& rhs)
    {
        iStr = rhs;
        iNext = NULL;
        return *this;
    }

    StrCSumPtrLenWrapper& operator=(const StrCSumPtrLenWrapper& rhs)
    {
        iStr = rhs.iStr;
        iNext = rhs.iNext;
        return *this;
    }

private:
    StrCSumPtrLen iStr;
    StrCSumPtrLenWrapper *iNext;
};

// class declaration
class OsclMemPoolVariableChunkAllocator;

// class for encapsulating string based key-value pair handlings
// The major feature for this class is hash-table based key search. To handle hash table conllision, linear search
// table is used. Basically a whole hash table is divided into two parts, the first part is for real hash table,
// the second part is for collision.
class StringKeyValueStore
{
    public:
        // add a key-value pair, key-value will be made a copy inside the store
        // return code is one of following StringKeyValueStoreReturnCodes
        int32 addKeyValuePair(const StrCSumPtrLen &aNewKey, const StrPtrLen &aNewValue, const bool aNeedReplaceOldValue = false);
        int32 addKeyValuePair(const StrCSumPtrLen &aNewKey, const char *aNewValue, const bool aNeedReplaceOldValue = false);

        // This overloaded function is for this case, when the key and value are parsed from the data stream, to avoid memory copy,
        // we locate the segment in the data stream for the key and value, which has no NULL terminatior.
        int32 addKeyValuePair(const char *aNewKey, const uint32 aKeyLength, const char *aNewValue, const uint32 aValueLength,
                              const bool aNeedReplaceOldValue = false);

        // get number of key-value pairs, which could be different from the number of keys,
        // when there might be a key with multiple values
        uint32 getNumberOfKeyValuePairs()
        {
            return iTotalNumberOfKeyValuePairs;
        }
        uint32 getNumberOfKeys()
        {
            return iFieldKeyTableIndexVector.size();
        }
        uint32 getTotalKeyValueLength()
        {
            return iTotalKeyValueLength;
        }
        // aListSize=0 means retrieves whatever the library has
        // return the actual list size
        uint32 getCurrentKeyList(StrPtrLen *&aFieldKeyList, const uint32 aListSize = 0);
        bool getValueByKey(const StrCSumPtrLen &aKey, StrPtrLen &aValue, const uint32 index = 0);

        // for one key multiple value cases.
        // return value: 0 => no value for the given key, 1 or more => number of values for the given key
        uint32 getNumberOfValuesByKey(const StrCSumPtrLen &aKey);

        // search
        bool isKeyValueAvailable(const StrCSumPtrLen &aKey);

        // remove
        bool removeKeyValuePair(const StrCSumPtrLen &aKey);

        // clear
        void clear();

        // copy
        bool copy(StringKeyValueStore& aStore);


        // query store infomation
        uint32 getCurrentMemoryUsage();
        uint32 getStoreSize();
        uint32 getAvailableSize();

        // constructor
        StringKeyValueStore() : iVariableSizeMemPool(NULL)
        {
            ;
        }

        // destructor
        ~StringKeyValueStore();

        // factory method
        static StringKeyValueStore* create(const uint32 aStoreSize = KEYVALUESTORE_MAX_SIZE);

        enum StringKeyValueStoreReturnCodes
        {
            StringKeyValueStore_Success = 0,
            StringKeyValueStore_Failure = -1,
            StringKeyValueStore_NoMemory = -2
        };

    private:
        uint32 calculateChecksum(const char *aBuffer, uint32 aBufferLength);
        bool construct(const uint32 aStoreSize);

        // for conflict handling
        // To handle hash table collisions, the hash table is divided into two parts, one part is for first hit elements, and
        // all the conflict elements is collected into another part. The reason for not choosing link list approach is, usually
        // conllision possibilities are very low, so no need to keep link list for every table element. (Note that if the collision
        // possibilities go high, hash function should be updated to keep this collision possibility low) Another reason is, don't
        // want to change the current structure too much.

        // aFindKey=true, the purpose of getting table index is for element search or removal;
        // aFindKey=false, the purpose is for adding a new element
        // return value: table index, for operation failure, return -1
        int32 getHashTableIndex(const StrCSumPtrLen &aKey, const bool aFindKey = true);
        // linear seach in linear search area of hash table for the given key
        // return table index for linear search area, -1 means the input key is not found
        int32 query(const StrCSumPtrLen &aKey);

        // supporting function for addKeyValuePair() and removeKeyValuePair()
        // return code is one of StringKeyValueStoreReturnCodes
        int32 addKeyToStore(const StrCSumPtrLen &aNewKey, int32 tableIndex);

        // centralize memory related operations within these two fucntions
        bool storeNewKeyValueItem(const char *aItem, const int32 aItemLength, char *&aNewLocation);
        void releaseOldKeyValueItem(const char *aItem, const int32 aItemLength);

    private:

        uint32 iTotalNumberOfKeyValuePairs;
        uint32 iTotalKeyValueLength;

        // field key@value tables (field key table is a hash table per se)
        StrCSumPtrLenWrapper iFieldKeys[KEYVALUESTORE_HASH_TABLE_SIZE_FOR_KEYS];
        StrPtrLen iFieldVals[KEYVALUESTORE_HASH_TABLE_SIZE_FOR_KEYS];

        // use variable-size memory pool to replace fixed-size memory storage
        // OsclMemPoolResizableAllocator is not used because of overhead.
        // That variable-size memory pool has fixed 28-byte header for each allocated memory segment. This
        // overhead is too much in the current user scenario, storing field key and value. Especially for
        // field key, it is usually less than 20 bytes. So that allocator is two expensive.
        OsclMemPoolVariableChunkAllocator* iVariableSizeMemPool;

        // storage for multiple field values with the same field key
        Oscl_Vector<StrCSumPtrLenWrapper, OsclMemAllocator> iStrCSumPtrLenWrapperVector;

        // save the parsed field key index of the field key table
        // The reason for this, usually less than a dozen of header fields are set for a message, comparing the
        // relatively big hash table, saving the table indices will save lots of search operations.
        Oscl_Vector<uint32, OsclMemAllocator> iFieldKeyTableIndexVector;
        uint32 iNumConflicts;
};

#endif // STRING_KEYVALUE_STORE_H_INCLUDED

