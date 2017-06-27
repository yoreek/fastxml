#ifndef _XH_RUBY_ST_H_
#define _XH_RUBY_ST_H_

#include "xh_config.h"
#include "xh_core.h"
#include "xh_ruby_internal.h"
#include "ccan/list/list.h"

typedef struct st_table_entry st_table_entry;

struct st_table_entry {
    st_index_t hash;
    st_data_t key;
    st_data_t record;
    st_table_entry *next;
    struct list_node olist;
};

typedef struct st_packed_entry {
    st_index_t hash;
    st_data_t key, val;
} st_packed_entry;

#define ST_DEFAULT_MAX_DENSITY 5
#define ST_DEFAULT_INIT_TABLE_SIZE 16
#define ST_DEFAULT_PACKED_TABLE_SIZE 18
#define PACKED_UNIT (int)(sizeof(st_packed_entry) / sizeof(st_table_entry*))
#define MAX_PACKED_HASH (int)(ST_DEFAULT_PACKED_TABLE_SIZE * sizeof(st_table_entry*) / sizeof(st_packed_entry))

#define FNV1_32A_INIT 0x811c9dc5
#define FNV_32_PRIME 0x01000193

#define type_numhash st_hashtype_num
const struct st_hash_type st_hashtype_num = {
    st_numcmp,
    st_numhash,
};

static st_index_t
strcasehash(st_data_t arg)
{
    register const char *string = (const char *)arg;
    register st_index_t hval = FNV1_32A_INIT;

    /*
     * FNV-1a hash each octet in the buffer
     */
    while (*string) {
        unsigned int c = (unsigned char)*string++;
        if ((unsigned int)(c - 'A') <= ('Z' - 'A')) c += 'a' - 'A';
        hval ^= c;

        /* multiply by the 32 bit FNV magic prime mod 2^32 */
        hval *= FNV_32_PRIME;
    }
    return hval;
}

static st_index_t
strhash(st_data_t arg)
{
    register const char *string = (const char *)arg;
    return st_hash(string, strlen(string), FNV1_32A_INIT);
}

/* extern int strcmp(const char *, const char *); */
static st_index_t strhash(st_data_t);
static const struct st_hash_type type_strhash = {
    strcmp,
    strhash,
};

static st_index_t strcasehash(st_data_t);
static const struct st_hash_type type_strcasehash = {
    st_locale_insensitive_strcasecmp,
    strcasehash,
};

#define EQUAL(table,x,ent) ((x)==(ent)->key || (*(table)->type->compare)((x),(ent)->key) == 0)

#define do_hash(key,table) (st_index_t)(*(table)->type->hash)((key))
#define hash_pos(h,n) ((h) & (n - 1))

#define PTR_NOT_EQUAL(table, ptr, hash_val, key) \
    ((ptr) != 0 && ((ptr)->hash != (hash_val) || !EQUAL((table), (key), (ptr))))

/* preparation for possible allocation improvements */
#define st_alloc_entry() (st_table_entry *)malloc(sizeof(st_table_entry))
#define st_free_entry(entry) free(entry)
#define st_alloc_table() (st_table *)malloc(sizeof(st_table))
#define st_dealloc_table(table) free(table)
#define st_alloc_bins(size) (st_table_entry **)calloc(size, sizeof(st_table_entry *))
#define st_free_bins(bins, size) free(bins)
static inline st_table_entry**
st_realloc_bins(st_table_entry **bins, st_index_t newsize, st_index_t oldsize)
{
    bins = (st_table_entry **)realloc(bins, newsize * sizeof(st_table_entry *));
    MEMZERO(bins, st_table_entry*, newsize);
    return bins;
}

/* Shortcut */
#define bins as.big.bins
#define real_entries as.packed.real_entries

/* preparation for possible packing improvements */
#define PACKED_BINS(table) ((table)->as.packed.entries)
#define PACKED_ENT(table, i) PACKED_BINS(table)[i]
#define PKEY(table, i) PACKED_ENT((table), (i)).key
#define PVAL(table, i) PACKED_ENT((table), (i)).val
#define PHASH(table, i) PACKED_ENT((table), (i)).hash
#define PKEY_SET(table, i, v) (PKEY((table), (i)) = (v))
#define PVAL_SET(table, i, v) (PVAL((table), (i)) = (v))
#define PHASH_SET(table, i, v) (PHASH((table), (i)) = (v))

static struct list_head *
st_head(const st_table *tbl)
{
    uintptr_t addr = (uintptr_t)&tbl->as.big.private_list_head;
    return (struct list_head *)addr;
}

static st_index_t
next_pow2(st_index_t x)
{
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
#if SIZEOF_ST_INDEX_T == 8
    x |= x >> 32;
#endif
    return x + 1;
}

static st_index_t
new_size(st_index_t size)
{
    st_index_t n;

    if (size && (size & ~(size - 1)) == size) /* already a power-of-two? */
        return size;

    n = next_pow2(size);
    if (n > size)
        return n;
#ifndef NOT_RUBY
    rb_raise(rb_eRuntimeError, "st_table too big");
#endif
    return -1; /* should raise exception */
}

static void
rehash(register st_table *table)
{
    register st_table_entry *ptr = 0, **new_bins;
    st_index_t new_num_bins, hash_val;

    new_num_bins = new_size(table->num_bins+1);
    new_bins = st_realloc_bins(table->bins, new_num_bins, table->num_bins);
    table->num_bins = new_num_bins;
    table->bins = new_bins;

    list_for_each(st_head(table), ptr, olist) {
        hash_val = hash_pos(ptr->hash, new_num_bins);
        ptr->next = new_bins[hash_val];
        new_bins[hash_val] = ptr;
    }
}

static st_table_entry *
find_entry(const st_table *table, st_data_t key, st_index_t hash_val,
           st_index_t bin_pos)
{
    register st_table_entry *ptr = table->bins[bin_pos];
    if (PTR_NOT_EQUAL(table, ptr, hash_val, key)) {
        while (PTR_NOT_EQUAL(table, ptr->next, hash_val, key)) {
            ptr = ptr->next;
        }
        ptr = ptr->next;
    }
    return ptr;
}

static inline st_index_t
find_packed_index_from(const st_table *table, st_index_t hash_val,
                       st_data_t key, st_index_t i)
{
    while (i < table->real_entries &&
           (PHASH(table, i) != hash_val || !EQUAL(table, key, &PACKED_ENT(table, i)))) {
        i++;
    }
    return i;
}

static inline st_index_t
find_packed_index(const st_table *table, st_index_t hash_val, st_data_t key)
{
    return find_packed_index_from(table, hash_val, key, 0);
}

static inline st_table_entry *
new_entry(st_table * table, st_data_t key, st_data_t value,
          st_index_t hash_val, register st_index_t bin_pos)
{
    register st_table_entry *entry = st_alloc_entry();

    entry->next = table->bins[bin_pos];
    table->bins[bin_pos] = entry;
    entry->hash = hash_val;
    entry->key = key;
    entry->record = value;

    return entry;
}

static inline st_data_t *
add_direct(st_table *table, st_data_t key, st_data_t value,
           st_index_t hash_val, register st_index_t bin_pos)
{
    register st_table_entry *entry;
    if (table->num_entries > ST_DEFAULT_MAX_DENSITY * table->num_bins) {
        rehash(table);
        bin_pos = hash_pos(hash_val, table->num_bins);
    }

    entry = new_entry(table, key, value, hash_val, bin_pos);

    list_add_tail(st_head(table), &entry->olist);

    table->num_entries++;
    return &entry->record;
}

static void
unpack_entries(register st_table *table)
{
    st_index_t i;
    st_packed_entry packed_bins[MAX_PACKED_HASH];
    register st_table_entry *entry;
    st_table tmp_table = *table;

    MEMCPY(packed_bins, PACKED_BINS(table), st_packed_entry, MAX_PACKED_HASH);
    table->as.packed.entries = packed_bins;
    tmp_table.entries_packed = 0;
#if ST_DEFAULT_INIT_TABLE_SIZE == ST_DEFAULT_PACKED_TABLE_SIZE
    MEMZERO(tmp_table.bins, st_table_entry*, tmp_table.num_bins);
#else
    tmp_table.bins = st_realloc_bins(tmp_table.bins, ST_DEFAULT_INIT_TABLE_SIZE, tmp_table.num_bins);
    tmp_table.num_bins = ST_DEFAULT_INIT_TABLE_SIZE;
#endif

    /*
     * order is important here, we need to keep the original table
     * walkable during GC (GC may be triggered by new_entry call)
     */
    i = 0;
    list_head_init(st_head(&tmp_table));
    do {
        st_data_t key = packed_bins[i].key;
        st_data_t val = packed_bins[i].val;
        st_index_t hash = packed_bins[i].hash;
        entry = new_entry(&tmp_table, key, val, hash,
                          hash_pos(hash, ST_DEFAULT_INIT_TABLE_SIZE));
        list_add_tail(st_head(&tmp_table), &entry->olist);
    } while (++i < MAX_PACKED_HASH);
    *table = tmp_table;
    list_head_init(st_head(table));
    list_append_list(st_head(table), st_head(&tmp_table));
}

static st_data_t *
add_packed_direct(st_table *table, st_data_t key, st_data_t value, st_index_t hash_val)
{
    st_data_t *lval;

    if (table->real_entries < MAX_PACKED_HASH) {
        st_index_t i = table->real_entries++;
        PKEY_SET(table, i, key);
        PVAL_SET(table, i, value);
        PHASH_SET(table, i, hash_val);
        table->num_entries++;
        lval = &PVAL(table, i);
    }
    else {
        unpack_entries(table);
        lval = add_direct(table, key, value, hash_val, hash_pos(hash_val, table->num_bins));
    }

    return lval;
}

static st_data_t *
st_store(register st_table *table, register st_data_t key, st_data_t value, xh_bool_t update)
{
    st_index_t hash_val;
    register st_index_t bin_pos;
    register st_table_entry *ptr;
    st_data_t *lval;

    hash_val = do_hash(key, table);

    if (table->entries_packed) {
        st_index_t i = find_packed_index(table, hash_val, key);
        if (i < table->real_entries) {
            lval = &PVAL(table, i);
        }
        else {
            lval = add_packed_direct(table, key, value, hash_val);
        }
    }
    else {
        ptr = find_entry(table, key, hash_val, bin_pos = hash_pos(hash_val, table->num_bins));

        if (ptr == 0) {
            lval = add_direct(table, key, value, hash_val, bin_pos);
        }
        else {
            lval = &ptr->record;
        }
    }

    if (update) *lval = value;

    return lval;
}

#endif /* _XH_RUBY_ST_H_ */
