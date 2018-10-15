#ifndef BINOMIAL_FOREST_H
#define BINOMIAL_FOREST_H

#include <stdbool.h>

typedef struct binomial_forest BinomialForest;

typedef struct binomial_tree BinomialTree;

struct binomial_forest {
    unsigned int order;
    unsigned int num_nodes;
    // List of binomial trees of increasing order
    BinomialTree **trees;
};

struct binomial_tree {
    unsigned int order;
    // List of children of orders k-1, k-2, ..., 1, 0
    struct binomial_tree *children;
    struct binomial_tree *siblings;
};

struct binomial_tree_options {
    bool free_nodes;
};

// Initializer for binomial trees of order 0 (single nodes)
#define BINOMIAL_TREE_INIT \
    (BinomialTree){ .order = 0, .children = NULL, .siblings = NULL }

// Allocate an empty binomial forest of order `n` for storing up to
// 2^0 + 2^1 + ... + 2^(n-1) = 2^n - 1 nodes
BinomialForest *binomial_forest_alloc(unsigned int n);
//@requires n <= 32

// Deallocate binomial forest `f`, using `options` to decide if remaining nodes
// should be freed
void binomial_forest_free_(BinomialForest *f, struct binomial_tree_options options);

// Convenience function for deallocating binomial forest `f`
// Default: don't free remaining nodes (may leak memory, but avoids freeing
// stack-allocated objects)
#define binomial_forest_free(f, ...) \
    binomial_forest_free_(f, (struct binomial_tree_options){ .free_nodes = false, __VA_ARGS__ })

// Test whether binomial forest `f` is empty
bool binomial_forest_empty(BinomialForest *f);
//@requires f != NULL

// Test whether binomial forest `f` is full
bool binomial_forest_empty(BinomialForest *f);
//@requires f != NULL

// Return the binary representation of binomial forest `f`, which is equivalent
// to determining the total number of nodes in `f`
unsigned int binomial_forest_rep(BinomialForest *f);
//@requires f != NULL
//@ensures \result == f->num_nodes

// Insert single-node binomial tree `t` (of order 0) into binomial forest `f`
// (push semantics)
void binomial_forest_push(BinomialForest *f, BinomialTree *t);
//@requires f != NULL && t != NULL
//@requires !binomial_forest_full(f)
//@requires t->order == 0

// Remove and return a single-node binomial tree from the front of binomial
// forest `f` (pop semantics)
BinomialTree *binomial_forest_pop(BinomialForest *f);
//@requires f != NULL

// Remove and return a binomial tree from the back of binomial forest `f`
// (steal semantics; steals between ~1/4 and 1/2 of `f`'s nodes)
BinomialTree *binomial_forest_steal_1(BinomialForest *f);
//@requires f != NULL

// Remove and return a complete binomial tree from the back of binomial forest
// `f` (steal semantics; steals between ~1/2 and all of `f`'s nodes)
BinomialTree *binomial_forest_steal_2(BinomialForest *f);
//@requires f != NULL

// Remove and return either a complete binomial tree or a subtree from the back
// of binomial forest `f` (steal semantics; steals between 1/3 and ~2/3 of
// `f`'s nodes)
BinomialTree *binomial_forest_steal_3(BinomialForest *f);
//@requires f != NULL

// Insert binomial tree `t` into empty binomial forest `f` and return `f`
// If NULL is passed as first argument, allocate a binomial forest large enough
// to hold `t` and return it after insertion
BinomialForest *binomial_forest_seed(BinomialForest *f, BinomialTree *t);
//@requires t != NULL

#endif // BINOMIAL_FOREST_H
