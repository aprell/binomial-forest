#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "binomial_forest.h"

// Deallocate binomial tree rooted at node `a`
static void binomial_tree_free(BinomialTree *a, struct binomial_tree_options options)
{
    if (!a) return;

    binomial_tree_free(a->children, options);
    binomial_tree_free(a->siblings, options);

    if (options.free_nodes) {
        free(a);
    }
}

// Merge two binomial trees `a` and `b` of equal order by making `b` the
// leftmost child of `a`
static BinomialTree *binomial_tree_link(BinomialTree *a, BinomialTree *b)
{
    assert(a != NULL && b != NULL);
    assert(a->order == b->order);
    assert(b->siblings == NULL);

    b->siblings = a->children;
    a->children = b;
    a->order = a->order + 1;

    return a;
}

// Split binomial tree `a` of order k, k > 0, into two binomial trees `a` and
// `b` of order k-1.
static BinomialTree *binomial_tree_unlink(BinomialTree *a)
{
    assert(a != NULL);

    if (a->order == 0) return NULL;

    assert(a->children != NULL);
    assert(a->siblings == NULL);

    BinomialTree *b = a->children;
    a->children = b->siblings;
    b->siblings = NULL;
    a->order = a->order - 1;

    return b;
}

#define POW_2(n) (1u << (n))

// Constructor short form
#define Tree(a, b) binomial_tree_link(a, b)

// Allocate an empty binomial forest of order `n` for storing up to
// 2^0 + 2^1 + ... + 2^(n-1) = 2^n - 1 nodes
BinomialForest *binomial_forest_alloc(unsigned int n)
{
    // Limit the maximum number of nodes to UINT_MAX
    assert(n <= 32);

    BinomialForest *forest = (BinomialForest *)malloc(sizeof(BinomialForest));
    if (!forest) {
        fprintf(stderr, "Warning: binomial_forest_alloc failed\n");
        return NULL;
    }

    forest->trees = (BinomialTree **)malloc(n * sizeof(BinomialTree *));
    if (!forest->trees) {
        fprintf(stderr, "Warning: binomial_forest_alloc failed\n");
        free(forest);
        return NULL;
    }

    for (unsigned int i = 0; i < n; i++) {
        forest->trees[i] = NULL;
    }

    forest->order = n;
    forest->num_nodes = 0;

    return forest;
}

// Deallocate binomial forest `f`, using `options` to decide if remaining nodes
// should be freed
void binomial_forest_free_(BinomialForest *f, struct binomial_tree_options options)
{
    if (!f) return;

    for (unsigned int i = 0; i < f->order; i++) {
        binomial_tree_free(f->trees[i], options);
    }

    free(f->trees);
    free(f);
}

// Test whether binomial forest `f` is empty
bool binomial_forest_empty(BinomialForest *f)
{
    assert(f != NULL);

    for (unsigned int i = 0; i < f->order; i++) {
        if (f->trees[i]) return false;
    }

    assert(f->num_nodes == 0);

    return true;
}

// Test whether binomial forest `f` is full
bool binomial_forest_full(BinomialForest *f)
{
    assert(f != NULL);

    for (unsigned int i = 0; i < f->order; i++) {
        if (!f->trees[i]) return false;
    }

    assert(f->num_nodes == POW_2(f->order) - 1);

    return true;
}

// Return the binary representation of binomial forest `f`, which is equivalent
// to determining the number of nodes in `f`
unsigned int binomial_forest_rep(BinomialForest *f)
{
    assert(f != NULL);

    unsigned int n = 0;

    for (unsigned int i = 0; i < f->order; i++) {
        n += f->trees[i] ? POW_2(i) : 0;
    }

    assert(n == f->num_nodes);

    return n;
}

// Insert single-node binomial tree `t` (of order 0) into binomial forest `f`
// (push semantics)
void binomial_forest_push(BinomialForest *f, BinomialTree *t)
{
    assert(f != NULL);
    assert(t != NULL);
    assert(t->order == 0);

    unsigned int i;

    // Merge it with the forest
    for (i = 0; i < f->order; i++) {
        if (f->trees[i]) {
            t = Tree(t, f->trees[i]);
            f->trees[i] = NULL;
        } else {
            f->trees[i] = t;
            break;
        }
    }

    f->num_nodes++;

    assert(i < f->order && "Insertion overflow");
}

// Remove binomial tree `t`'s children and return them to binomial forest `f`
static void binomial_forest_merge(BinomialForest *f, BinomialTree *t)
{
    assert(f != NULL);
    assert(t != NULL);

    BinomialTree *children = t->children;
    unsigned int order = t->order;

    while (children) {
        BinomialTree *child = children;
        order--;
        assert(child->order == order);
        children = child->siblings;
        child->siblings = NULL;
        f->trees[order] = child;
    }

    assert(order == 0);
    t->children = NULL;
}

// Remove and return a single-node binomial tree from the front of binomial
// forest `f` (pop semantics)
BinomialTree *binomial_forest_pop(BinomialForest *f)
{
    assert(f != NULL);

    BinomialTree *t = NULL;
    unsigned int i;

    // Find first tree
    for (i = 0; i < f->order; i++) {
        if (f->trees[i]) {
            t = f->trees[i];
            f->trees[i] = NULL;
            break;
        }
    }

    if (!t) {
        assert(binomial_forest_empty(f));
        return NULL;
    }

    assert(t != NULL);
    assert(t->order == i);
    assert(t->siblings == NULL);

    // Merge i children of orders i-1, i-2, ..., 1, 0 with the forest
    binomial_forest_merge(f, t);
    assert(t->children == NULL);

    f->num_nodes--;

    return t;
}

// Remove the highest-order binomial tree from binomial forest `f`
static BinomialTree *remove_highest(BinomialForest *f)
{
    assert(f != NULL);

    if (binomial_forest_empty(f)) return NULL;

    BinomialTree *t = NULL;
    unsigned int i;

    assert(f->order > 0);
    
    // Find last tree
    for (i = f->order; i > 0; i--) {
        if (f->trees[i-1]) {
            t = f->trees[i-1];
            f->trees[i-1] = NULL;
            break;
        }
    }

    // Guaranteed to be found
    assert(i > 0);
    assert(t != NULL);
    assert(t->order == i-1);
    assert(t->siblings == NULL);

    f->num_nodes -= POW_2(i-1);

    return t;
}

// Split off leftmost child from binomial tree `t` and return `t` to binomial
// forest `f`
static BinomialTree *split_merge(BinomialForest *f, BinomialTree *t)
{
    assert(f != NULL);

    if (!t) return NULL;

    // Split off leftmost child of order t->order-1
    BinomialTree *c = binomial_tree_unlink(t);
    if (!c) {
        assert(t->order == 0);
        // Return last node
        assert(binomial_forest_empty(f));
        return t;
    } else {
        assert(t->order == c->order);
        // Merge t with f
        if (f->trees[t->order]) {
            BinomialTree *tt = Tree(f->trees[t->order], t);
            assert(tt->order = t->order+1);
            f->trees[tt->order] = tt;
            f->trees[t->order] = NULL;
        } else {
            f->trees[t->order] = t;
        }
        f->num_nodes += POW_2(c->order);
        return c;
    }
}

// Remove and return a binomial tree from the back of binomial forest `f`
// (steal semantics; steals between ~1/4 and 1/2 of `f`'s nodes
BinomialTree *binomial_forest_steal_1(BinomialForest *f)
{
    assert(f != NULL);

    return split_merge(f, remove_highest(f));
}

// Remove and return a complete binomial tree from the back of binomial forest
// `f` (steal semantics; steals between ~1/2 and all of a `f`'s nodes)
BinomialTree *binomial_forest_steal_2(BinomialForest *f)
{
    assert(f != NULL);

    return remove_highest(f);
}

// Remove and return either a complete binomial tree or a subtree from the back
// of binomial forest `f` (steal semantics; steals between 1/3 and ~2/3 of a
// `f`'s nodes)
BinomialTree *binomial_forest_steal_3(BinomialForest *f)
{
    assert(f != NULL);

    BinomialTree *t = remove_highest(f);

    if (!t) return NULL;

    unsigned int x = POW_2(t->order); // 2^floor(log_2 n)

    // f->num_nodes + x > x + x/2 | -x
    if (f->num_nodes > x/2) {
        // Steal complete tree
        return t;
    } else {
        // Steal subtree
        return split_merge(f, t);
    }
}

// Insert binomial tree `t` into empty binomial forest `f` and return `f`
// If NULL is passed as first argument, allocate a binomial forest large enough
// to hold `t` and return it after insertion
BinomialForest *binomial_forest_seed(BinomialForest *f, BinomialTree *t)
{
    assert(t != NULL);

    if (f == NULL) {
        f = binomial_forest_alloc(t->order + 1);
        if (!f) return NULL;
    }

    assert(f != NULL);
    assert(binomial_forest_empty(f));
    assert(t->order < f->order);

    f->trees[t->order] = t;
    f->num_nodes = POW_2(t->order);

    return f;
}

#ifdef TEST

typedef struct test {
    BinomialTree tree;
    int data;
} Test;

static void test_push_pop_stack_data(void)
{
    BinomialForest *f = binomial_forest_alloc(5);

    assert(binomial_forest_empty(f));
    assert(binomial_forest_rep(f) == /* 0b00000000 */ 0x00);

    Test numbers[31];

    // Push numbers [0, 30]
    for (int i = 0; i < 31; i++) {
        numbers[i].tree = BINOMIAL_TREE_INIT;
        numbers[i].data = i;
        binomial_forest_push(f, &numbers[i].tree);
    }

    assert(binomial_forest_rep(f) == /* 0b00011111 */ 0x1F);
    assert(binomial_forest_full(f));

    // Pop numbers [30, 0]
    for (int i = 0; i < 31; i++) {
        assert((Test *)binomial_forest_pop(f) == &numbers[30-i]);
    }

    assert(binomial_forest_empty(f));
    assert(binomial_forest_rep(f) == /* 0b00000000 */ 0x00);

    binomial_forest_free(f);
}

static void test_push_pop_heap_data(void)
{
    BinomialForest *f = binomial_forest_alloc(5);

    assert(binomial_forest_empty(f));
    assert(binomial_forest_rep(f) == /* 0b00000000 */ 0x00);

    // Push numbers [0, 30]
    for (int i = 0; i < 31; i++) {
        Test *t = malloc(sizeof(Test));
        assert(t);
        t->tree = BINOMIAL_TREE_INIT;
        t->data = i;
        binomial_forest_push(f, &t->tree);
    }

    assert(binomial_forest_rep(f) == /* 0b00011111 */ 0x1F);
    assert(binomial_forest_full(f));

    // Pop numbers [30, 20]
    for (int i = 0; i < 11; i++) {
        Test *t = (Test *)binomial_forest_pop(f);
        assert(t->data == 30-i);
        free(t);
    }

    assert(!binomial_forest_empty(f));
    assert(!binomial_forest_full(f));
    assert(binomial_forest_rep(f) == /* 0b00010100 */ 0x14);

    binomial_forest_free(f, .free_nodes = true);
}

static void test_push_steal(BinomialTree *(*steal)(BinomialForest *))
{
    BinomialForest *f = binomial_forest_alloc(10);
    BinomialForest *g = NULL;

    assert(binomial_forest_empty(f));
    assert(binomial_forest_rep(f) == /* 0b00000000 */ 0x00);

    // Push numbers [0, 99]
    for (int i = 0; i < 100; i++) {
        Test *t = malloc(sizeof(Test));
        assert(t);
        t->tree = BINOMIAL_TREE_INIT;
        t->data = i;
        binomial_forest_push(f, &t->tree);
    }

    assert(binomial_forest_rep(f) == /* 0b01100100 */ 0x64);

    // Steal and pop numbers until f is empty
    while (!binomial_forest_empty(f)) {
        // Allocates in first iteration when g is NULL
        g = binomial_forest_seed(g, steal(f));
        unsigned int n = f->num_nodes;
        unsigned int m = g->num_nodes;
        for (unsigned int i = 0; i < m; i++) {
            Test *t = (Test *)binomial_forest_pop(g);
            assert((unsigned int)t->data == 99-n-i);
            free(t);
        }
        assert(binomial_forest_empty(g));
    }

    assert(binomial_forest_empty(f));
    assert(binomial_forest_empty(g));
    assert(binomial_forest_rep(f) == /* 0b00000000 */ 0x00);
    assert(binomial_forest_rep(g) == /* 0b00000000 */ 0x00);

    binomial_forest_free(f);
    binomial_forest_free(g);
}

int main(void)
{
    test_push_pop_stack_data();
    test_push_pop_heap_data();
    test_push_steal(binomial_forest_steal_1);
    test_push_steal(binomial_forest_steal_2);
    test_push_steal(binomial_forest_steal_3);

    return 0;
}

#endif // TEST
