//! [0]
VersionNumber v1(3,0,0,0);
VersionNumber v2(10,1,0);
VersionNumber v3(1,2,0,0);
VersionNumber n1 = v1.normalized(2);
VersionNumber n2 = v2.normalized(8);
VersionNumber n3 = v3.normalized();
// n1 is 3.0
// n2 is 10.1.0
// n3 is 1.2
//! [0]