// Qt Includes
#include <QtTest>

// Qx Includes
#include <qx/core/qx-array.h>

// Test Includes
//#include <qx_test_common.h>

class tst_qx_array : public QObject
{
    Q_OBJECT

public:
    tst_qx_array();

private slots:
    // Init
    // void initTestCase();
    // void initTestCase_data();
    // void cleanupTestCase();
    // void init()
    // void cleanup();

    // Test cases
    void mostFrequent();
};

// Setup
tst_qx_array::tst_qx_array() {}

// Cases
void tst_qx_array::mostFrequent()
{
    int a[8] = {1,1,2,3,4,5,5,5};
    int b[5] = {1,2,2,3,4};
    int c[5] = {1,1,2,3,3};
    QVERIFY(Qx::Array::mostFrequent(a) == 5);
    QVERIFY(Qx::Array::mostFrequent(b) == 2);
    int cRes = Qx::Array::mostFrequent(c);
    QVERIFY( cRes == 1 || cRes == 3);
}

QTEST_APPLESS_MAIN(tst_qx_array)
#include "tst_qx_array.moc"
