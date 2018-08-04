#include <stdint.h>
#include <assert.h>

#define FNV1_32_PRIME ((uint32_t)16777619)
#define FNV1_32_INIT ((uint32_t)0x811c9dc5)
#define FNV1_TINY_MASK(num_bits) (((uint32_t)1<<(num_bits))-1)


static uint32_t fnv1_32_buf(uint8_t *buf, size_t len)
{
    uint32_t hash = FNV1_32_INIT;
    for (size_t i = 0; i < len; ++i) {
        hash = hash ^ buf[i];
        hash = hash * FNV1_32_PRIME;
    }

    return hash;
}

static uint16_t fold_fnv1_32_hash_tiny(uint32_t hash, uint8_t num_bits)
{
    assert(num_bits != 0);
    assert(num_bits < 16);
    return (((hash>>num_bits) ^ hash) & FNV1_TINY_MASK(num_bits));
}


struct bytevec {
    uint8_t *mem;
    size_t len;
    size_t cap;
}


static void bytevec_init(struct bytevec *bv)
{
    size_t capacity = 256;

    uint8_t *mem = malloc(capacity);
    bv->mem = mem;
    bv->len = 0;
    if (NULL == mem) {
        bv->capacity = 0;
    } else {
        bv->capacity = capacity;
    }
}

static bool bytevec_grow(struct bytevec *bv)
{
    size_t capacity = bv->capacity * 2;
    uint8_t *new_mem = realloc(bv->mem, capacity);
    if (NULL == new_mem) {
        return false;
    }

    bv->mem = new_mem;
    bv->capacity = capacity;

    return true;
}

static bool bytevec_append(struct bytevec *bv, uint8_t *members, size_t num_members)
{
    // Inefficient, but simple :)
    while (bv->capacity - bv->len < num_members) {
        if (!bytevec_grow(bv)) {
            return false;
        }
    }

    memcpy(bv->mem, members, num_members);
    bv->len += num_members;
}

static void bytevec_free(struct bytevec *bv)
{
    free(bv->mem);
    bv->mem = NULL;
    bv->capacity = 0
    bv->len = 0
}


struct hash_entry {
    uint16_t string; // offset in the backing storage
    uint16_t hash_value;
}

struct intern_hash {
    struct hash_entry *backing_array;
    size_t count;
    size_t cap;
}

static struct hash_entry hash_entry_replace(struct hash_entry *location, struct hash_entry value)
{
    struct hash_entry old = *location;
    *location = value;
    return old;
}

static struct hash_entry *hash_fetch(struct intern_hash *ih, uint32_t index)
{
    return &ih.backing_array[index % ih->cap];
}

static void hash_insert_internal(struct intern_hash *ih, struct hash_entry entry)
{
    uint16_t travelled = 0;

    while (true)
    {
        uint16_t hash_value = entry.hash_value;
        uint16_t index = ((uint32_t)hash_value + (uint32_t)travelled) % ih->cap;
        struct hash_entry *insetrtion_slot = &ih->backing_array[index];

        // Doesn't support deletion.
        if (0 == insetrtion_slot->string)
        {
            *insetrtion_slot = entry;
            break;
        }

        uint32_t insetrtion_slot_dib = (ih->cap + index - insetrtion_slot->hash_value) % ih->cap;

        if (travelled > insetrtion_slot_dib) {
            entry = hash_entry_replace(insetrtion_slot, entry);
            travelled = insetrtion_slot_dib + 1;
        }
        else {
            travelled += 1;
        }
    }
}
