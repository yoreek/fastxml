#ifndef _XH_RUBY_ST_H_
#define _XH_RUBY_ST_H_

#include "xh_config.h"
#include "xh_core.h"
#include "xh_ruby_internal.h"

#ifdef __GNUC__
#define PREFETCH(addr, write_p) __builtin_prefetch(addr, write_p)
#define EXPECT(expr, val) __builtin_expect(expr, val)
#define ATTRIBUTE_UNUSED  __attribute__((unused))
#else
#define PREFETCH(addr, write_p)
#define EXPECT(expr, val) (expr)
#define ATTRIBUTE_UNUSED
#endif

#ifdef ST_DEBUG
#define st_assert(cond) assert(cond)
#else
#define st_assert(cond) ((void)(0 && (cond)))
#endif

/* The type of hashes.  */
typedef st_index_t st_hash_t;

struct st_table_entry {
    st_hash_t hash;
    st_data_t key;
    st_data_t record;
};

#define type_numhash st_hashtype_num
const struct st_hash_type st_hashtype_num = {
    st_numcmp,
    st_numhash,
};

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

/* Value used to catch uninitialized entries/bins during debugging.
   There is a possibility for a false alarm, but its probability is
   extremely small.  */
#define ST_INIT_VAL 0xafafafafafafafaf
#define ST_INIT_VAL_BYTE 0xafa

#ifdef RUBY
#undef malloc
#undef realloc
#undef calloc
#undef free
#define malloc ruby_xmalloc
#define calloc ruby_xcalloc
#define realloc ruby_xrealloc
#define free ruby_xfree
#endif

#define EQUAL(tab,x,y) ((x) == (y) || (*(tab)->type->compare)((x),(y)) == 0)
#define PTR_EQUAL(tab, ptr, hash_val, key) \
    ((ptr)->hash == (hash_val) && EQUAL((tab), (key), (ptr)->key))

/* As PRT_EQUAL only its result is returned in RES.  REBUILT_P is set
   up to TRUE if the table is rebuilt during the comparison.  */
#define DO_PTR_EQUAL_CHECK(tab, ptr, hash_val, key, res, rebuilt_p) \
    do {							    \
	unsigned int _old_rebuilds_num = (tab)->rebuilds_num;       \
	res = PTR_EQUAL(tab, ptr, hash_val, key);		    \
	rebuilt_p = _old_rebuilds_num != (tab)->rebuilds_num;	    \
    } while (FALSE)

/* Features of a table.  */
struct st_features {
    /* Power of 2 used for number of allocated entries.  */
    unsigned char entry_power;
    /* Power of 2 used for number of allocated bins.  Depending on the
       table size, the number of bins is 2-4 times more than the
       number of entries.  */
    unsigned char bin_power;
    /* Enumeration of sizes of bins (8-bit, 16-bit etc).  */
    unsigned char size_ind;
    /* Bins are packed in words of type st_index_t.  The following is
       a size of bins counted by words.  */
    st_index_t bins_words;
};

/* Features of all possible size tables.  */
#if SIZEOF_ST_INDEX_T == 8
#define MAX_POWER2 62
static const struct st_features features[] = {
    {0, 1, 0, 0x0},
    {1, 2, 0, 0x1},
    {2, 3, 0, 0x1},
    {3, 4, 0, 0x2},
    {4, 5, 0, 0x4},
    {5, 6, 0, 0x8},
    {6, 7, 0, 0x10},
    {7, 8, 0, 0x20},
    {8, 9, 1, 0x80},
    {9, 10, 1, 0x100},
    {10, 11, 1, 0x200},
    {11, 12, 1, 0x400},
    {12, 13, 1, 0x800},
    {13, 14, 1, 0x1000},
    {14, 15, 1, 0x2000},
    {15, 16, 1, 0x4000},
    {16, 17, 2, 0x10000},
    {17, 18, 2, 0x20000},
    {18, 19, 2, 0x40000},
    {19, 20, 2, 0x80000},
    {20, 21, 2, 0x100000},
    {21, 22, 2, 0x200000},
    {22, 23, 2, 0x400000},
    {23, 24, 2, 0x800000},
    {24, 25, 2, 0x1000000},
    {25, 26, 2, 0x2000000},
    {26, 27, 2, 0x4000000},
    {27, 28, 2, 0x8000000},
    {28, 29, 2, 0x10000000},
    {29, 30, 2, 0x20000000},
    {30, 31, 2, 0x40000000},
    {31, 32, 2, 0x80000000},
    {32, 33, 3, 0x200000000},
    {33, 34, 3, 0x400000000},
    {34, 35, 3, 0x800000000},
    {35, 36, 3, 0x1000000000},
    {36, 37, 3, 0x2000000000},
    {37, 38, 3, 0x4000000000},
    {38, 39, 3, 0x8000000000},
    {39, 40, 3, 0x10000000000},
    {40, 41, 3, 0x20000000000},
    {41, 42, 3, 0x40000000000},
    {42, 43, 3, 0x80000000000},
    {43, 44, 3, 0x100000000000},
    {44, 45, 3, 0x200000000000},
    {45, 46, 3, 0x400000000000},
    {46, 47, 3, 0x800000000000},
    {47, 48, 3, 0x1000000000000},
    {48, 49, 3, 0x2000000000000},
    {49, 50, 3, 0x4000000000000},
    {50, 51, 3, 0x8000000000000},
    {51, 52, 3, 0x10000000000000},
    {52, 53, 3, 0x20000000000000},
    {53, 54, 3, 0x40000000000000},
    {54, 55, 3, 0x80000000000000},
    {55, 56, 3, 0x100000000000000},
    {56, 57, 3, 0x200000000000000},
    {57, 58, 3, 0x400000000000000},
    {58, 59, 3, 0x800000000000000},
    {59, 60, 3, 0x1000000000000000},
    {60, 61, 3, 0x2000000000000000},
    {61, 62, 3, 0x4000000000000000},
    {62, 63, 3, 0x8000000000000000},
};

#else
#define MAX_POWER2 30

static const struct st_features features[] = {
    {0, 1, 0, 0x1},
    {1, 2, 0, 0x1},
    {2, 3, 0, 0x2},
    {3, 4, 0, 0x4},
    {4, 5, 0, 0x8},
    {5, 6, 0, 0x10},
    {6, 7, 0, 0x20},
    {7, 8, 0, 0x40},
    {8, 9, 1, 0x100},
    {9, 10, 1, 0x200},
    {10, 11, 1, 0x400},
    {11, 12, 1, 0x800},
    {12, 13, 1, 0x1000},
    {13, 14, 1, 0x2000},
    {14, 15, 1, 0x4000},
    {15, 16, 1, 0x8000},
    {16, 17, 2, 0x20000},
    {17, 18, 2, 0x40000},
    {18, 19, 2, 0x80000},
    {19, 20, 2, 0x100000},
    {20, 21, 2, 0x200000},
    {21, 22, 2, 0x400000},
    {22, 23, 2, 0x800000},
    {23, 24, 2, 0x1000000},
    {24, 25, 2, 0x2000000},
    {25, 26, 2, 0x4000000},
    {26, 27, 2, 0x8000000},
    {27, 28, 2, 0x10000000},
    {28, 29, 2, 0x20000000},
    {29, 30, 2, 0x40000000},
    {30, 31, 2, 0x80000000},
};

#endif

/* The reserved hash value and its substitution.  */
#define RESERVED_HASH_VAL (~(st_hash_t) 0)
#define RESERVED_HASH_SUBSTITUTION_VAL ((st_hash_t) 0)

/* Return hash value of KEY for table TAB.  */
static inline st_hash_t
do_hash(st_data_t key, st_table *tab)
{
    st_hash_t hash = (st_hash_t)(tab->type->hash)(key);

    /* RESERVED_HASH_VAL is used for a deleted entry.  Map it into
       another value.  Such mapping should be extremely rare.  */
    return hash == RESERVED_HASH_VAL ? RESERVED_HASH_SUBSTITUTION_VAL : hash;
}

/* Power of 2 defining the minimal number of allocated entries.  */
#define MINIMAL_POWER2 2

#if MINIMAL_POWER2 < 2
#error "MINIMAL_POWER2 should be >= 2"
#endif

/* If the power2 of the allocated `entries` is less than the following
   value, don't allocate bins and use a linear search.  */
#define MAX_POWER2_FOR_TABLES_WITHOUT_BINS 4

/* Entry and bin values returned when we found a table rebuild during
   the search.  */
#define REBUILT_TABLE_ENTRY_IND (~(st_index_t) 1)
#define REBUILT_TABLE_BIN_IND (~(st_index_t) 1)

/* Return value of N-th bin in array BINS of table with bins size
   index S.  */
static inline st_index_t
get_bin(st_index_t *bins, int s, st_index_t n)
{
  return (s == 0 ? ((unsigned char *) bins)[n]
	  : s == 1 ? ((unsigned short *) bins)[n]
	  : s == 2 ? ((unsigned int *) bins)[n]
	  : ((st_index_t *) bins)[n]);
}

/* Set up N-th bin in array BINS of table with bins size index S to
   value V.  */
static inline void
set_bin(st_index_t *bins, int s, st_index_t n, st_index_t v)
{
    if (s == 0) ((unsigned char *) bins)[n] = (unsigned char) v;
    else if (s == 1) ((unsigned short *) bins)[n] = (unsigned short) v;
    else if (s == 2) ((unsigned int *) bins)[n] = (unsigned int) v;
    else ((st_index_t *) bins)[n] = v;
}

/* These macros define reserved values for empty table bin and table
   bin which contains a deleted entry.  We will never use such values
   for an entry index in bins.  */
#define EMPTY_BIN    0
#define DELETED_BIN  1
/* Base of a real entry index in the bins.  */
#define ENTRY_BASE 2

/* Mark I-th bin of table TAB as empty, in other words not
   corresponding to any entry.  */
#define MARK_BIN_EMPTY(tab, i) (set_bin((tab)->bins, get_size_ind(tab), i, EMPTY_BIN))

/* Values used for not found entry and bin with given
   characteristics.  */
#define UNDEFINED_ENTRY_IND (~(st_index_t) 0)
#define UNDEFINED_BIN_IND (~(st_index_t) 0)

/* Mark I-th bin of table TAB as corresponding to a deleted table
   entry.  Update number of entries in the table and number of bins
   corresponding to deleted entries. */
#define MARK_BIN_DELETED(tab, i)				\
    do {                                                        \
        st_assert(i != UNDEFINED_BIN_IND);			\
	st_assert(! IND_EMPTY_OR_DELETED_BIN_P(tab, i)); 	\
        set_bin((tab)->bins, get_size_ind(tab), i, DELETED_BIN); \
    } while (0)

/* Macros to check that value B is used empty bins and bins
   corresponding deleted entries.  */
#define EMPTY_BIN_P(b) ((b) == EMPTY_BIN)
#define DELETED_BIN_P(b) ((b) == DELETED_BIN)
#define EMPTY_OR_DELETED_BIN_P(b) ((b) <= DELETED_BIN)

/* Macros to check empty bins and bins corresponding to deleted
   entries.  Bins are given by their index I in table TAB.  */
#define IND_EMPTY_BIN_P(tab, i) (EMPTY_BIN_P(get_bin((tab)->bins, get_size_ind(tab), i)))
#define IND_DELETED_BIN_P(tab, i) (DELETED_BIN_P(get_bin((tab)->bins, get_size_ind(tab), i)))
#define IND_EMPTY_OR_DELETED_BIN_P(tab, i) (EMPTY_OR_DELETED_BIN_P(get_bin((tab)->bins, get_size_ind(tab), i)))

/* Macros for marking and checking deleted entries given by their
   pointer E_PTR.  */
#define MARK_ENTRY_DELETED(e_ptr) ((e_ptr)->hash = RESERVED_HASH_VAL)
#define DELETED_ENTRY_P(e_ptr) ((e_ptr)->hash == RESERVED_HASH_VAL)

/* Return bin size index of table TAB.  */
static inline unsigned int
get_size_ind(const st_table *tab)
{
    return tab->size_ind;
}

/* Return the number of allocated bins of table TAB.  */
static inline st_index_t
get_bins_num(const st_table *tab)
{
    return ((st_index_t) 1)<<tab->bin_power;
}

/* Return mask for a bin index in table TAB.  */
static inline st_index_t
bins_mask(const st_table *tab)
{
    return get_bins_num(tab) - 1;
}

/* Return the index of table TAB bin corresponding to
   HASH_VALUE.  */
static inline st_index_t
hash_bin(st_hash_t hash_value, st_table *tab)
{
    return hash_value & bins_mask(tab);
}

/* Return the number of allocated entries of table TAB.  */
static inline st_index_t
get_allocated_entries(const st_table *tab)
{
    return ((st_index_t) 1)<<tab->entry_power;
}

/* Return size of the allocated bins of table TAB.  */
static inline st_index_t
bins_size(const st_table *tab)
{
    return features[tab->entry_power].bins_words * sizeof (st_index_t);
}

/* Mark all bins of table TAB as empty.  */
static void
initialize_bins(st_table *tab)
{
    memset(tab->bins, 0, bins_size(tab));
}

#ifdef ST_DEBUG
#define st_assert_notinitial(ent) \
    do { \
	st_assert(ent.hash != (st_hash_t) ST_INIT_VAL);  \
	st_assert(ent.key != ST_INIT_VAL); \
	st_assert(ent.record != ST_INIT_VAL); \
    } while (0)
/* Check the table T consistency.  It can be extremely slow.  So use
   it only for debugging.  */
static void
st_check(st_table *tab)
{
    st_index_t d, e, i, n, p;

    for (p = get_allocated_entries(tab), i = 0; p > 1; i++, p>>=1)
        ;
    p = i;
    st_assert(p >= MINIMAL_POWER2);
    st_assert(tab->entries_bound <= get_allocated_entries(tab));
    st_assert(tab->entries_start <= tab->entries_bound);
    n = 0;
    return;
    if (tab->entries_bound != 0)
        for (i = tab->entries_start; i < tab->entries_bound; i++) {
	    st_assert_notinitial(tab->entries[i]);
	    if (! DELETED_ENTRY_P(&tab->entries[i]))
	        n++;
	}
    st_assert(n == tab->num_entries);
    if (tab->bins == NULL)
        st_assert(p <= MAX_POWER2_FOR_TABLES_WITHOUT_BINS);
    else {
        st_assert(p > MAX_POWER2_FOR_TABLES_WITHOUT_BINS);
	for (n = d = i = 0; i < get_bins_num(tab); i++) {
	    st_assert(get_bin(tab->bins, tab->size_ind, i) != ST_INIT_VAL);
	    if (IND_DELETED_BIN_P(tab, i)) {
	        d++;
		continue;
	    }
	    else if (IND_EMPTY_BIN_P(tab, i))
	        continue;
	    n++;
	    e = get_bin(tab->bins, tab->size_ind, i) - ENTRY_BASE;
	    st_assert(tab->entries_start <= e && e < tab->entries_bound);
	    st_assert(! DELETED_ENTRY_P(&tab->entries[e]));
	    st_assert_notinitial(tab->entries[e]);
	}
	st_assert(n == tab->num_entries);
	st_assert(n + d < get_bins_num(tab));
    }
}
#endif

static st_index_t
find_table_bin_ind_direct(st_table *table, st_hash_t hash_value, st_data_t key);

static st_index_t
find_table_bin_ptr_and_reserve(st_table *tab, st_hash_t *hash_value,
			       st_data_t key, st_index_t *bin_ind);

#ifdef HASH_LOG
static void
count_collision(const struct st_hash_type *type)
{
    collision.all++;
    if (type == &type_numhash) {
        collision.num++;
    }
    else if (type == &type_strhash) {
        collision.strcase++;
    }
    else if (type == &type_strcasehash) {
        collision.str++;
    }
}

#define COLLISION (collision_check ? count_collision(tab->type) : (void)0)
#define FOUND_BIN (collision_check ? collision.total++ : (void)0)
#define collision_check 0
#else
#define COLLISION
#define FOUND_BIN
#endif

/* If the number of entries in the table is at least REBUILD_THRESHOLD
   times less than the entry array length, decrease the table
   size.  */
#define REBUILD_THRESHOLD 4

#if REBUILD_THRESHOLD < 2
#error "REBUILD_THRESHOLD should be >= 2"
#endif

/* Rebuild table TAB.  Rebuilding removes all deleted bins and entries
   and can change size of the table entries and bins arrays.
   Rebuilding is implemented by creation of a new table or by
   compaction of the existing one.  */
static void
rebuild_table(st_table *tab)
{
    st_index_t i, ni, bound;
    unsigned int size_ind;
    st_table *new_tab;
    st_table_entry *entries, *new_entries;
    st_table_entry *curr_entry_ptr;
    st_index_t *bins;
    st_index_t bin_ind;

    st_assert(tab != NULL);
    bound = tab->entries_bound;
    entries = tab->entries;
    if ((2 * tab->num_entries <= get_allocated_entries(tab)
	 && REBUILD_THRESHOLD * tab->num_entries > get_allocated_entries(tab))
	|| tab->num_entries < (1 << MINIMAL_POWER2)) {
        /* Compaction: */
        tab->num_entries = 0;
	if (tab->bins != NULL)
	    initialize_bins(tab);
	new_tab = tab;
	new_entries = entries;
    }
    else {
        new_tab = st_init_table_with_size(tab->type,
					  2 * tab->num_entries - 1);
	new_entries = new_tab->entries;
    }
    ni = 0;
    bins = new_tab->bins;
    size_ind = get_size_ind(new_tab);
    for (i = tab->entries_start; i < bound; i++) {
        curr_entry_ptr = &entries[i];
	PREFETCH(entries + i + 1, 0);
	if (EXPECT(DELETED_ENTRY_P(curr_entry_ptr), 0))
	    continue;
	if (&new_entries[ni] != curr_entry_ptr)
	    new_entries[ni] = *curr_entry_ptr;
	if (EXPECT(bins != NULL, 1)) {
	    bin_ind = find_table_bin_ind_direct(new_tab, curr_entry_ptr->hash,
						curr_entry_ptr->key);
	    st_assert(bin_ind != UNDEFINED_BIN_IND
		      && (tab == new_tab || new_tab->rebuilds_num == 0)
		      && IND_EMPTY_BIN_P(new_tab, bin_ind));
	    set_bin(bins, size_ind, bin_ind, ni + ENTRY_BASE);
	}
	new_tab->num_entries++;
	ni++;
    }
    if (new_tab != tab) {
        tab->entry_power = new_tab->entry_power;
	tab->bin_power = new_tab->bin_power;
	tab->size_ind = new_tab->size_ind;
	st_assert(tab->num_entries == ni);
	st_assert(new_tab->num_entries == ni);
	if (tab->bins != NULL)
	    free(tab->bins);
	tab->bins = new_tab->bins;
	free(tab->entries);
	tab->entries = new_tab->entries;
	free(new_tab);
    }
    tab->entries_start = 0;
    tab->entries_bound = tab->num_entries;
    tab->rebuilds_num++;
#ifdef ST_DEBUG
    st_check(tab);
#endif
}

/* Return the next secondary hash index for table TAB using previous
   index IND and PERTERB.  Finally modulo of the function becomes a
   full *cycle linear congruential generator*, in other words it
   guarantees traversing all table bins in extreme case.

   According the Hull-Dobell theorem a generator
   "Xnext = (a*Xprev + c) mod m" is a full cycle generator iff
     o m and c are relatively prime
     o a-1 is divisible by all prime factors of m
     o a-1 is divisible by 4 if m is divisible by 4.

   For our case a is 5, c is 1, and m is a power of two.  */
static inline st_index_t
secondary_hash(st_index_t ind, st_table *tab, st_index_t *perterb)
{
    *perterb >>= 11;
    ind = (ind << 2) + ind + *perterb + 1;
    return hash_bin(ind, tab);
}

/* Find an entry with HASH_VALUE and KEY in TABLE using a linear
   search.  Return the index of the found entry in array `entries`.
   If it is not found, return UNDEFINED_ENTRY_IND.  If the table was
   rebuilt during the search, return REBUILT_TABLE_ENTRY_IND.  */
static inline st_index_t
find_entry(st_table *tab, st_hash_t hash_value, st_data_t key)
{
    int eq_p, rebuilt_p;
    st_index_t i, bound;
    st_table_entry *entries;

    bound = tab->entries_bound;
    entries = tab->entries;
    for (i = tab->entries_start; i < bound; i++) {
	DO_PTR_EQUAL_CHECK(tab, &entries[i], hash_value, key, eq_p, rebuilt_p);
	if (EXPECT(rebuilt_p, 0))
	    return REBUILT_TABLE_ENTRY_IND;
	if (eq_p)
	    return i;
    }
    return UNDEFINED_ENTRY_IND;
}

/* Find and return index of table TAB bin corresponding to an entry
   with HASH_VALUE and KEY.  The entry should be in the table
   already.  */
static st_index_t
find_table_bin_ind_direct(st_table *tab, st_hash_t hash_value, st_data_t key)
{
    st_index_t ind;
#ifdef QUADRATIC_PROBE
    st_index_t d;
#else
    st_index_t peterb;
#endif
    st_index_t bin;
    st_table_entry *entries = tab->entries;

    st_assert(tab != NULL);
    st_assert(tab->bins != NULL);
    ind = hash_bin(hash_value, tab);
#ifdef QUADRATIC_PROBE
    d = 1;
#else
    peterb = hash_value;
#endif
    FOUND_BIN;
    for (;;) {
        bin = get_bin(tab->bins, get_size_ind(tab), ind);
        if (EMPTY_OR_DELETED_BIN_P(bin))
	    return ind;
	st_assert (entries[bin - ENTRY_BASE].hash != hash_value);
#ifdef QUADRATIC_PROBE
	ind = hash_bin(ind + d, tab);
	d++;
#else
        ind = secondary_hash(ind, tab, &peterb);
#endif
        COLLISION;
    }
}

/* Return index of table TAB bin for HASH_VALUE and KEY through
   BIN_IND and the pointed value as the function result.  Reserve the
   bin for inclusion of the corresponding entry into the table if it
   is not there yet.  We always find such bin as bins array length is
   bigger entries array.  Although we can reuse a deleted bin, the
   result bin value is always empty if the table has no entry with
   KEY.  Return the entries array index of the found entry or
   UNDEFINED_ENTRY_IND if it is not found.  If the table was rebuilt
   during the search, return REBUILT_TABLE_ENTRY_IND.  */
static st_index_t
find_table_bin_ptr_and_reserve(st_table *tab, st_hash_t *hash_value,
			       st_data_t key, st_index_t *bin_ind)
{
    int eq_p, rebuilt_p;
    st_index_t ind;
    st_hash_t curr_hash_value = *hash_value;
#ifdef QUADRATIC_PROBE
    st_index_t d;
#else
    st_index_t peterb;
#endif
    st_index_t entry_index;
    st_index_t first_deleted_bin_ind;
    st_table_entry *entries;

    st_assert(tab != NULL);
    st_assert(tab->bins != NULL);
    st_assert(tab->entries_bound <= get_allocated_entries(tab));
    st_assert(tab->entries_start <= tab->entries_bound);
    ind = hash_bin(curr_hash_value, tab);
#ifdef QUADRATIC_PROBE
    d = 1;
#else
    peterb = curr_hash_value;
#endif
    FOUND_BIN;
    first_deleted_bin_ind = UNDEFINED_BIN_IND;
    entries = tab->entries;
    for (;;) {
        entry_index = get_bin(tab->bins, get_size_ind(tab), ind);
        if (EMPTY_BIN_P(entry_index)) {
            tab->num_entries++;
	    entry_index = UNDEFINED_ENTRY_IND;
            if (first_deleted_bin_ind != UNDEFINED_BIN_IND) {
                /* We can reuse bin of a deleted entry.  */
                ind = first_deleted_bin_ind;
                MARK_BIN_EMPTY(tab, ind);
            }
            break;
	}
	else if (! DELETED_BIN_P(entry_index)) {
	    DO_PTR_EQUAL_CHECK(tab, &entries[entry_index - ENTRY_BASE], curr_hash_value, key, eq_p, rebuilt_p);
	    if (EXPECT(rebuilt_p, 0))
		return REBUILT_TABLE_ENTRY_IND;
            if (eq_p)
                break;
	}
	else if (first_deleted_bin_ind == UNDEFINED_BIN_IND)
            first_deleted_bin_ind = ind;
#ifdef QUADRATIC_PROBE
	ind = hash_bin(ind + d, tab);
	d++;
#else
        ind = secondary_hash(ind, tab, &peterb);
#endif
        COLLISION;
    }
    *bin_ind = ind;
    return entry_index;
}

/* Check the table and rebuild it if it is necessary.  */
static inline void
rebuild_table_if_necessary (st_table *tab)
{
    st_index_t bound = tab->entries_bound;

    if (bound == get_allocated_entries(tab))
        rebuild_table(tab);
    st_assert(tab->entries_bound < get_allocated_entries(tab));
}

#define FNV1_32A_INIT 0x811c9dc5

/*
 * 32 bit magic FNV-1a prime
 */
#define FNV_32_PRIME 0x01000193

#ifndef UNALIGNED_WORD_ACCESS
# if defined(__i386) || defined(__i386__) || defined(_M_IX86) || \
     defined(__x86_64) || defined(__x86_64__) || defined(_M_AMD64) || \
     defined(__powerpc64__) || \
     defined(__mc68020__)
#   define UNALIGNED_WORD_ACCESS 1
# endif
#endif
#ifndef UNALIGNED_WORD_ACCESS
# define UNALIGNED_WORD_ACCESS 0
#endif

/* This hash function is quite simplified MurmurHash3
 * Simplification is legal, cause most of magic still happens in finalizator.
 * And finalizator is almost the same as in MurmurHash3 */
#define BIG_CONSTANT(x,y) ((st_index_t)(x)<<32|(st_index_t)(y))
#define ROTL(x,n) ((x)<<(n)|(x)>>(SIZEOF_ST_INDEX_T*CHAR_BIT-(n)))

#if ST_INDEX_BITS <= 32
#define C1 (st_index_t)0xcc9e2d51
#define C2 (st_index_t)0x1b873593
#else
#define C1 BIG_CONSTANT(0x87c37b91,0x114253d5);
#define C2 BIG_CONSTANT(0x4cf5ad43,0x2745937f);
#endif
static inline st_index_t
murmur_step(st_index_t h, st_index_t k)
{
#if ST_INDEX_BITS <= 32
#define r1 (17)
#define r2 (11)
#else
#define r1 (33)
#define r2 (24)
#endif
    k *= C1;
    h ^= ROTL(k, r1);
    h *= C2;
    h = ROTL(h, r2);
    return h;
}
#undef r1
#undef r2

static inline st_index_t
murmur_finish(st_index_t h)
{
#if ST_INDEX_BITS <= 32
#define r1 (16)
#define r2 (13)
#define r3 (16)
    const st_index_t c1 = 0x85ebca6b;
    const st_index_t c2 = 0xc2b2ae35;
#else
/* values are taken from Mix13 on http://zimbry.blogspot.ru/2011/09/better-bit-mixing-improving-on.html */
#define r1 (30)
#define r2 (27)
#define r3 (31)
    const st_index_t c1 = BIG_CONSTANT(0xbf58476d,0x1ce4e5b9);
    const st_index_t c2 = BIG_CONSTANT(0x94d049bb,0x133111eb);
#endif
#if ST_INDEX_BITS > 64
    h ^= h >> 64;
    h *= c2;
    h ^= h >> 65;
#endif
    h ^= h >> r1;
    h *= c1;
    h ^= h >> r2;
    h *= c2;
    h ^= h >> r3;
    return h;
}
#undef r1
#undef r2
#undef r3

static st_index_t
strhash(st_data_t arg)
{
    register const char *string = (const char *)arg;
    return st_hash(string, strlen(string), FNV1_32A_INIT);
}

PUREFUNC(static st_index_t strcasehash(st_data_t));
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

static st_data_t *
st_store(st_table *tab, st_data_t key, st_data_t value, xh_bool_t update)
{
    st_table_entry *entry;
    st_index_t bin;
    st_index_t ind;
    st_hash_t hash_value;
    st_index_t bin_ind;
    int new_p;
    st_data_t *lval;


    hash_value = do_hash(key, tab);
retry:
    rebuild_table_if_necessary(tab);
    if (tab->bins == NULL) {
        bin = find_entry(tab, hash_value, key);
        if (EXPECT(bin == REBUILT_TABLE_ENTRY_IND, 0))
            goto retry;
        new_p = bin == UNDEFINED_ENTRY_IND;
        if (new_p)
            tab->num_entries++;
        bin_ind = UNDEFINED_BIN_IND;
    } else {
        bin = find_table_bin_ptr_and_reserve(tab, &hash_value, key, &bin_ind);
        if (EXPECT(bin == REBUILT_TABLE_ENTRY_IND, 0))
            goto retry;
        new_p = bin == UNDEFINED_ENTRY_IND;
        bin -= ENTRY_BASE;
    }

    if (new_p) {
        ind = tab->entries_bound++;
        entry = &tab->entries[ind];
        entry->hash = hash_value;
        entry->key = key;
        entry->record = value;
        if (bin_ind != UNDEFINED_BIN_IND)
            set_bin(tab->bins, get_size_ind(tab), bin_ind, ind + ENTRY_BASE);
    }
    else {
        entry = &tab->entries[bin];
    }

    lval = &entry->record;

    if (update) *lval = value;

    return lval;
}

#endif /* _XH_RUBY_ST_H_ */
