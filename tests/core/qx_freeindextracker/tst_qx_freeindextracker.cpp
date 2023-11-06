// Qt Includes
#include <QtTest>

// Qx Includes
#include <qx/core/qx-freeindextracker.h>

// Test Includes
//#include <qx_test_common.h>

class tst_qx_freeindextracker : public QObject
{
    Q_OBJECT

private:
    static inline const Qx::FreeIndexTracker CMN_TRACKER = Qx::FreeIndexTracker(5, 50, {6, 7, 35, 36, 37, 38, 39, 40, 41, 50});
    static inline const Qx::FreeIndexTracker CMN_TRACKER_FULL = Qx::FreeIndexTracker(10,12, {10,11,12});

public:
    tst_qx_freeindextracker();

private slots:
    // Init
    // void initTestCase();
    // void initTestCase_data();
    // void cleanupTestCase();
    // void init()
    // void cleanup();

    // Test cases
    void constructor();
    void isReserved();
    void minimum();
    void maximum();
    void range();
    void free();
    void reserved();
    void isBooked();
    void firstReserved();
    void lastReserved();
    void firstFree();
    void lastFree();
    void previousFree();
    void nextFree();
    void nearestFree();
    void reserve();
    void reserveFirstFree();
    void reserveLastFree();
    void reserveNextFree();
    void reservePreviousFree();
    void reserveNearestFree();
    void release();
};

// Setup
tst_qx_freeindextracker::tst_qx_freeindextracker() {}

// Cases
void tst_qx_freeindextracker::constructor()
{
    // Standard
    Qx::FreeIndexTracker tracker(0, 10);
    QCOMPARE(tracker.range(), 11);
    QCOMPARE(tracker.minimum(), 0);
    QCOMPARE(tracker.maximum(), 10);
    QCOMPARE(tracker.free(), 11);

    // Offset with pre-reservations
    QSet<quint64> resv{5,6,9};
    tracker = Qx::FreeIndexTracker(5, 10, {5,6,9});
    QCOMPARE(tracker.range(), 6);
    QCOMPARE(tracker.minimum(), 5);
    QCOMPARE(tracker.maximum(), 10);
    QCOMPARE(tracker.free(), 3);

    for(quint64 i = 5; i <= 10; i++)
        QVERIFY(tracker.isReserved(i) == resv.contains(i));

    // Expand
    tracker =Qx::FreeIndexTracker(10, 11, {5,12});;
    QCOMPARE(tracker.range(), 8);
    QCOMPARE(tracker.minimum(), 5);
    QCOMPARE(tracker.maximum(), 12);
}

void tst_qx_freeindextracker::isReserved() { QVERIFY(CMN_TRACKER.isReserved(36)); }

void tst_qx_freeindextracker::minimum() { QCOMPARE(CMN_TRACKER.minimum(), 5); }

void tst_qx_freeindextracker::maximum() { QCOMPARE(CMN_TRACKER.maximum(), 50); }

void tst_qx_freeindextracker::range() { QCOMPARE(CMN_TRACKER.range(), 46); }

void tst_qx_freeindextracker::free() { QCOMPARE(CMN_TRACKER.free(), 36); }

void tst_qx_freeindextracker::reserved() { QCOMPARE(CMN_TRACKER.reserved(), 10); }

void tst_qx_freeindextracker::isBooked()
{
    QVERIFY(!CMN_TRACKER.isBooked());
    QVERIFY(CMN_TRACKER_FULL.isBooked());
}

void tst_qx_freeindextracker::firstReserved() { QCOMPARE(CMN_TRACKER.firstReserved(), 6); }

void tst_qx_freeindextracker::lastReserved() { QCOMPARE(CMN_TRACKER.lastReserved(), 50); }

void tst_qx_freeindextracker::firstFree(){ QCOMPARE(CMN_TRACKER.firstFree(), 5); }

void tst_qx_freeindextracker::lastFree() { QCOMPARE(CMN_TRACKER.lastFree(), 49); }

void tst_qx_freeindextracker::previousFree()
{
    QCOMPARE(CMN_TRACKER.previousFree(40), 34);
    QCOMPARE(CMN_TRACKER.previousFree(42), 42);
    QCOMPARE(CMN_TRACKER.previousFree(35), 34);
    QCOMPARE(CMN_TRACKER_FULL.previousFree(11), std::nullopt);
}

void tst_qx_freeindextracker::nextFree()
{
    QCOMPARE(CMN_TRACKER.nextFree(40), 42);
    QCOMPARE(CMN_TRACKER.nextFree(45), 45);
    QCOMPARE(CMN_TRACKER.nextFree(6), 8);
    QCOMPARE(CMN_TRACKER.nextFree(50), std::nullopt);
}
void tst_qx_freeindextracker::nearestFree()
{
    QCOMPARE(CMN_TRACKER.nearestFree(40), 42);
    QCOMPARE(CMN_TRACKER.nearestFree(37), 34);
    QCOMPARE(CMN_TRACKER.nearestFree(42), 42);
    QCOMPARE(CMN_TRACKER_FULL.nearestFree(11), std::nullopt);
}
void tst_qx_freeindextracker::reserve()
{
    Qx::FreeIndexTracker tkr(CMN_TRACKER);
    QVERIFY(!tkr.reserve(35));
    QCOMPARE(tkr.free(), 36);
    QVERIFY(!tkr.isReserved(8));
    QVERIFY(tkr.reserve(8));
    QCOMPARE(tkr.free(), 35);
    QVERIFY(tkr.isReserved(8));
}
void tst_qx_freeindextracker::reserveFirstFree()
{
    Qx::FreeIndexTracker tkr(CMN_TRACKER);
    QCOMPARE(tkr.reserveFirstFree(), 5);
    QCOMPARE(tkr.reserveFirstFree(), 8);

    Qx::FreeIndexTracker full(CMN_TRACKER_FULL);
    QCOMPARE(full.reserveFirstFree(), std::nullopt);
}
void tst_qx_freeindextracker::reserveLastFree()
{
    Qx::FreeIndexTracker tkr(CMN_TRACKER);
    QCOMPARE(tkr.reserveLastFree(), 49);
    QCOMPARE(tkr.reserveLastFree(), 48);

    Qx::FreeIndexTracker full(CMN_TRACKER_FULL);
    QCOMPARE(full.reserveLastFree(), std::nullopt);
}
void tst_qx_freeindextracker::reserveNextFree()
{
    Qx::FreeIndexTracker tkr(CMN_TRACKER);
    QCOMPARE(tkr.reserveNextFree(40), 42);
    QCOMPARE(tkr.reserveNextFree(45), 45);
    QCOMPARE(tkr.reserveNextFree(6), 8);
    QCOMPARE(tkr.reserveNextFree(50), std::nullopt);
    QCOMPARE(tkr.free(), 33);
}

void tst_qx_freeindextracker::reservePreviousFree()
{
    Qx::FreeIndexTracker tkr(CMN_TRACKER);
    QCOMPARE(tkr.reservePreviousFree(40), 34);
    QCOMPARE(tkr.reservePreviousFree(42), 42);
    QCOMPARE(tkr.reservePreviousFree(34), 33);
    QCOMPARE(tkr.free(), 33);
}

void tst_qx_freeindextracker::reserveNearestFree()
{
    Qx::FreeIndexTracker tkr(CMN_TRACKER);
    QCOMPARE(tkr.reserveNearestFree(40), 42);
    QCOMPARE(tkr.reserveNearestFree(43), 43);
    QCOMPARE(tkr.reserveNearestFree(35), 34);
    QCOMPARE(tkr.reserveNearestFree(7), 8);
    QCOMPARE(tkr.free(), 32);

    Qx::FreeIndexTracker full(CMN_TRACKER_FULL);
    QCOMPARE(full.reserveNearestFree(11), std::nullopt);
}

void tst_qx_freeindextracker::release()
{
    Qx::FreeIndexTracker tkr(CMN_TRACKER);
    QVERIFY(!tkr.release(5));
    QVERIFY(tkr.release(35));
    QCOMPARE(tkr.free(), 37);
}

QTEST_APPLESS_MAIN(tst_qx_freeindextracker)
#include "tst_qx_freeindextracker.moc"
