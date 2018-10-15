**A binomial forest data structure [[1][1]] for efficient work stealing in
runtime systems using private deques [[2][2], [3][3]]**

A binomial forest of order _n_ is an array of length _n_ that contains up to
_n_ distinct binomial trees, arranged in increasing order from 0 to _n_-1.
Binomial trees are defined inductively as follows: (1) A binomial tree of
order 0 is a single node. (2) A binomial tree of order _k_, _k_ > 0, is made
up of two binomial trees of order _k_-1, one being the leftmost child of the
other. Another useful property of binomial trees is that the root of a
binomial tree of order _k_ has _k_ children, which are themselves roots of
binomial trees of orders _k_-1, _k_-2, ..., 1, 0 (from leftmost child to
rightmost child).

A binomial tree of order _k_ has 2<sup>_k_</sup> nodes, and the total number
of nodes in a binomial forest is derived from its binary representation, where
a '1' at array index _i_ marks the presence of a binomial tree of order _i_
with 2<sup>_i_</sup> tasks. Thus, a binomial forest of order _n_ has up to
&#931;<sub>0&le;_i_<_n_</sub> 2<sup>_i_</sup> = 2<sup>_n_</sup>-1 nodes.

Work-stealing deques provide three main operations: `push`, `pop`, and
`steal`, with `pop` returning the task that was pushed last (LIFO), and
`steal` returning the oldest task(s) to be handed off to other workers (FIFO).

- `push` creates a single-node tree of order 0 and adds it to the forest,
  following binary addition: if there are two trees of the same order _k_, a
  carry occurs, and the two trees are merged into a tree of order _k_+1 by
  making one tree the left child of the other. A tree is either inserted or
  merged until the resulting tree can be inserted. When merging, the new node
  becomes the root and the existing tree the leftmost child.

- `pop` returns the least recently inserted node, which is the root node of
  the lowest-order binomial tree. Thus, `pop` finds the first tree in the
  array of trees, splits off all children of the root node, reinserts them
  into the forest, and returns the root node. This corresponds to binary
  subtraction.

- `steal`, unlike `pop`, looks at the highest-order binomial tree, which is
  known to contain the oldest nodes. We can choose to steal the entire tree or
  its leftmost child. Given a tree of order _k_, this means either
  2<sup>_k_</sup> or 2<sup>_k_-1</sup> nodes are removed from the forest. In
  the latter case, by removing the leftmost child of order _k_-1, we end up
  with another tree of order _k_-1 that must be reinserted into the forest,
  possibly after merging once (at most once because there is no other tree of
  order _k_ or higher).

<!-- References -->

[1]: https://en.wikipedia.org/wiki/Binomial_heap
[2]: https://dl.acm.org/citation.cfm?id=2442538
[3]: https://epub.uni-bayreuth.de/2990
