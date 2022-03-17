//! [0]
Index<int>::LAST + 10 // = Index<int>::LAST
Index<int>::LAST - 10 // = Index<int>::LAST
Index<int>(5) + Index<int>::LAST // = Index<int>::LAST
Index<int>(1087) - Index<int>::LAST // = 0
Index<int>::LAST * 130 // = Index<int>::LAST
Index<int>::LAST / 58 // = Index<int>::LAST
Index<int>::LAST / Index<int>::LAST // = 1
Index<int>(98) / Index<int>::LAST // = 0
//! [0]