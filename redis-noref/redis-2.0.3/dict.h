/* Hash Tables Implementation.
 *
 * This file implements in memory hash tables with insert/del/replace/find/
 * get-random-element operations. Hash tables will auto resize if needed
 * tables of power of two in size are used, collisions are handled by
 * chaining. See the source code for more information... :)
 *
 * Copyright (c) 2006-2010, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __DICT_H
#define __DICT_H

#define DICT_OK 0
#define DICT_ERR 1

/* Unused arguments generate annoying warnings... */
#define DICT_NOTUSED(V) ((void) V)

struct dictEntry {
    void * E_T(@k) key;
    void * E_T(@v) val;
    struct dictEntry E_G(@k, @v) *next;
} E_GENERIC(@k, @v);

typedef struct dictEntry E_G(@k, @v) dictEntry E_GENERIC(@k, @v);

typedef struct dictType {
    unsigned int (*hashFunction)(const void *key);
    void *(*keyDup)(void *privdata, const void *key);
    void *(*valDup)(void *privdata, const void *obj);
    int (*keyCompare)(void *privdata, const void *key1, const void *key2);
    void (*keyDestructor)(void *privdata, void *key);
    void (*valDestructor)(void *privdata, void *obj);
} dictType;

/* This is our hash table structure. Every dictionary has two of this as we
 * implement incremental rehashing, for the old to the new table. */
struct dictht {
    dictEntry E_G(@k, @v) ** E_PTRARRAY(self.size) table;
    unsigned long size;
    unsigned long sizemask;
    unsigned long used;
} E_GENERIC(@k, @v);
typedef struct dictht E_G(@k, @v) dictht E_GENERIC(@k, @v);

struct dict {
    int dtype;
    void * E_OPAQUE privdata;
    dictht E_G(@k, @v) ht[2];
    int rehashidx; /* rehashing not in progress if rehashidx == -1 */
    int iterators; /* number of iterators currently running */
} E_GENERIC(@k, @v);
typedef struct dict E_G(@k, @v) dict E_GENERIC(@k, @v);

typedef struct dictIterator {
    dict *d;
    int table;
    int index;
    dictEntry *entry, *nextEntry;
} dictIterator;

/* This is the initial size of every hash table */
#define DICT_HT_INITIAL_SIZE     4

/* ------------------------------- Macros ------------------------------------*/
#define dictFreeEntryVal(d, entry) \
    if ((allDictTypes[(d)->dtype])->valDestructor) \
        (allDictTypes[(d)->dtype])->valDestructor((d)->privdata, (entry)->val)

#define dictSetHashVal(d, entry, _val_) do { \
    if ((allDictTypes[(d)->dtype])->valDup) \
        entry->val = (allDictTypes[(d)->dtype])->valDup((d)->privdata, _val_); \
    else \
        entry->val = (_val_); \
} while(0)

#define dictFreeEntryKey(d, entry) \
    if ((allDictTypes[(d)->dtype])->keyDestructor) \
        (allDictTypes[(d)->dtype])->keyDestructor((d)->privdata, (entry)->key)

#define dictSetHashKey(d, entry, _key_) do { \
    if ((allDictTypes[(d)->dtype])->keyDup) \
        entry->key = (allDictTypes[(d)->dtype])->keyDup((d)->privdata, _key_); \
    else \
        entry->key = (_key_); \
} while(0)

#define dictCompareHashKeys(d, key1, key2) \
    (((allDictTypes[(d)->dtype])->keyCompare) ? \
        (allDictTypes[(d)->dtype])->keyCompare((d)->privdata, key1, key2) : \
        (key1) == (key2))

#define dictHashKey(d, key) (allDictTypes[(d)->dtype])->hashFunction(key)

#define dictGetEntryKey(he) ((he)->key)
#define dictGetEntryVal(he) ((he)->val)
#define dictSlots(d) ((d)->ht[0].size+(d)->ht[1].size)
#define dictSize(d) ((d)->ht[0].used+(d)->ht[1].used)
#define dictIsRehashing(ht) ((ht)->rehashidx != -1)

/* API */
dict *dictCreate(int type, void *privDataPtr);
int dictExpand(dict *d, unsigned long size);
int dictAdd(dict *d, void *key, void *val);
int dictReplace(dict *d, void *key, void *val);
int dictDelete(dict *d, const void *key);
int dictDeleteNoFree(dict *d, const void *key);
void dictRelease(dict *d);
dictEntry * dictFind(dict *d, const void *key);
void *dictFetchValue(dict *d, const void *key);
int dictResize(dict *d);
dictIterator *dictGetIterator(dict *d);
dictEntry *dictNext(dictIterator *iter);
void dictReleaseIterator(dictIterator *iter);
dictEntry *dictGetRandomKey(dict *d);
void dictPrintStats(dict *d);
unsigned int dictGenHashFunction(const unsigned char *buf, int len);
void dictEmpty(dict *d);
void dictEnableResize(void);
void dictDisableResize(void);
int dictRehash(dict *d, int n);
int dictRehashMilliseconds(dict *d, int ms);

/* Hash table types */
extern dictType dictTypeHeapStringCopyKey;
extern dictType dictTypeHeapStrings;
extern dictType dictTypeHeapStringCopyKeyValue;

#endif /* __DICT_H */