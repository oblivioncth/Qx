//! [0]
Index<int>(Qx::Last) + 10 // = Index<int>(Qx::Last)
Index<int>(Qx::Last) - 10 // = Index<int>(Qx::Last)
Index<int>(5) + Index<int>(Qx::Last) // = Index<int>(Qx::Last)
Index<int>(1087) - Index<int>(Qx::Last) // = 0
Index<int>(Qx::Last) * 130 // = Index<int>(Qx::Last)
Index<int>(Qx::Last) / 58 // = Index<int>(Qx::Last)
Index<int>(Qx::Last) / Index<int>(Qx::Last) // = 1
Index<int>(98) / Index<int>(Qx::Last) // = 0
//! [0]