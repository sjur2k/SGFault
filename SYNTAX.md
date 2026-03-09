## General

Bracketed scopes: `{}`\
Vector indexing: `[i]`\
0-indexing is used `[vec[0],vec[1],vec[2]]`

### For loops
Usage: `for i in set {Do operation}`\
Loops are defined with integer iterators. Sets are defined as whole number intervals which can be either open or closed in either end. Interval bounds can be negative, and will iterate in the positive direction unless `-i` is used. If descending bounds are combined with `-i`, it will iterate like normal.

The iterator is a local variable and will not affect other variables with the same name. 
Scope must be bracketed. Parentheses around condition are not needed, but whitespace is required around the "for" token. 
Examples:\
`for i in [0,10]` iterates from 0 to 10. \
`for -i in [0,10]` iterates from 10 to 0, and is equivalent to `for i in [10,0]`\
`for i in (0,10)` iterates from 1 to 9\
`for i in [0,10)` iterates from 0 to 9\
The interval notation is inspired by mathematical notation, but for whole numbers.\
`i in [a,b)` is understood as $i \in \mathbb{Z} \textit{ s.t. } \mathrm{a\leq i < b}$