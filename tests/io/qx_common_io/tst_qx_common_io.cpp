// Qt Includes
#include <QtTest>

// Qx Includes
#include <qx/io/qx-common-io.h>

// Test Includes
//#include <qx_test_common.h>

class tst_qx_common_io : public QObject
{
    Q_OBJECT

private:
    QTemporaryDir mWriteDir;

public:
    tst_qx_common_io();

private slots:
    // Init
    // void initTestCase();
    // void initTestCase_data();
    // void cleanupTestCase();
    // void init()
    // void cleanup();

    // Test cases
    void writeStringToFile_data();
    void writeStringToFile();
};

// Setup
tst_qx_common_io::tst_qx_common_io()
{
    QVERIFY(mWriteDir.isValid());
}

// Cases
void tst_qx_common_io::writeStringToFile_data()
{
    QTest::addColumn<QFile*>("file");
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");

    // TODO: Make this more generic with helper functions once more test files are added

    // Overwrite
    QString overwritePath = mWriteDir.filePath("overwrite_file.txt");
    QVERIFY(QFile::copy(u":/data/writeStringToFile/overwrite_original.txt"_s, overwritePath));
    QFile::setPermissions(overwritePath, QFile::WriteOwner); // Required since copying from a file marked read-only

    QFile inputFile(u":/data/writeStringToFile/overwrite_input.txt"_s);
    QVERIFY(inputFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString input = inputFile.readAll();
    inputFile.close();

    QFile expectedFile(u":/data/writeStringToFile/overwrite_expected.txt"_s);
    QVERIFY(expectedFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString expected = expectedFile.readAll();
    expectedFile.close();

    QTest::newRow("Overwrite") << new QFile(overwritePath, this) << input << expected;
}

void tst_qx_common_io::writeStringToFile()
{
    // Fetch data from test table
    QFETCH(QFile*, file);
    QFETCH(QString, input);
    QFETCH(QString, expected);

    // Write to file
    // TODO: Handle different arguments for writeStringToFile in _data
    Qx::IoOpReport rp = Qx::writeStringToFile(*file, input, Qx::Overwrite, Qx::TextPos(1,2));
    QVERIFY(!rp.isFailure());

    // Get file's new contents and compare
    QVERIFY(file->open(QIODevice::ReadOnly | QIODevice::Text));
    QTextStream ts(file); // Handles proper conversion of \r\n to \n
    QString newData = ts.readAll();
    QCOMPARE(newData, expected);
    file->close();
}

QTEST_APPLESS_MAIN(tst_qx_common_io)
#include "tst_qx_common_io.moc"
