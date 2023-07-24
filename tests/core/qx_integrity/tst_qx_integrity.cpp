// Qt Includes
#include <QtTest>

// Qx Includes
#include <qx/core/qx-integrity.h>

// Test Includes
//#include <qx_test_common.h>

class tst_qx_integrity : public QObject
{
    Q_OBJECT

public:
    tst_qx_integrity();

private slots:
    // Init
    // void initTestCase();
    // void initTestCase_data();
    // void cleanupTestCase();
    // void init()
    // void cleanup();

    // Test cases
    // void generateCheckum();
    void crc32();
};

// Setup
tst_qx_integrity::tst_qx_integrity() {}

// Cases
void tst_qx_integrity::crc32()
{
    quint32 res = Qx::Integrity::crc32(QByteArrayLiteral("ThisIsForCRC32Testing"));
    QCOMPARE(res, 0x926CE4A1);
}

QTEST_APPLESS_MAIN(tst_qx_integrity)
#include "tst_qx_integrity.moc"
