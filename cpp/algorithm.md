bin : Two phases: an encoding, then a decoding with a simultaneous calculation of the complementary sequence. Encoding/decoding are done with bitwise operations.

hash : This algorithm is based on hashmap associat A -> T, T -> A, C -> G, G -> C.

hash_allocate : same as hash but allocate the return string at the begin of function

hash_switch : same as hash but run the reverse complement in place.

naive : Just some if else

naive_allocate : same as naive but allocate the return string at the begin of function

naive_constref : same as naive but DNA passed with const ref

naive_switch : same as naive but run the reverse complement in place.

tab_allocate : same as tab but allocate the return string at the begin of function

tab_constref : same as tab but DNA passed with const ref

tab_switch : same as tab but run the reverse complement in place.

pol3 : This algoritm is based one a third degree polynomial obtaine by lagrange interpolation.

pol3_allocate : same as pol3 but allocate the return string at the begin of function

pol3_switch : same as pol3 but run the reverse complement in place.

tab : This algorithm is based on char tabluar with 85 (T char value + 1) case and associated A -> T, T -> A, C -> G, G -> C.

