// Qt Includes
#include <QtTest>

// Qx Includes
#include <qx/core/qx-json.h>
#include <qx/core/qx-error.h>

// Test Includes
//#include <qx_test_common.h>

// Some tests are incompatible with GCC < 11
#if !defined(Q_CC_GNU) || defined(Q_CC_CLANG) || Q_CC_GNU >= 1100
    #define COMPATIBLE_COMPILER
#endif

class tst_qx_json : public QObject
{
    Q_OBJECT

public:
    tst_qx_json();

private slots:
    // Init
    // void initTestCase();
    // void initTestCase_data();
    // void cleanupTestCase();
    // void init()
    // void cleanup();

    // Test cases
    // void generateCheckum();
    void full_declarative_suite();
};

//-Tools---------------------------------------------------------
class CustomClass // Essentially just a struct, but tests custom Converter specialization
{
private:
    bool ccb = false;
    double ccd = 0.0;

public:
    CustomClass() {}
    CustomClass(bool b, double d) :
        ccb(b),
        ccd(d)
    {}

    bool b() const { return ccb; }
    double d() const { return ccd; }
    void setB(bool b) { ccb = b; }
    void setD(double d) { ccd = d; }
    bool operator==(const CustomClass& other) const = default;
};

struct IntKeyable
{
    int key;
    int value;

    bool operator==(const IntKeyable& other) const = default;

    // Basic inner struct macro
    QX_JSON_STRUCT(key, value);
};

struct StringKeyable
{
    QString key;
    int value;

    bool operator==(const StringKeyable& other) const = default;

    // Basic inner struct macro
    QX_JSON_STRUCT(key, value);
};

struct OutsideTestee
{
    bool value;

    bool operator==(const OutsideTestee& other) const = default;
};
QX_JSON_STRUCT_OUTSIDE(OutsideTestee, value);

struct ExtendedOutsideTestee
{
    bool value;

    bool operator==(const ExtendedOutsideTestee& other) const = default;
};
QX_JSON_STRUCT_OUTSIDE_X(ExtendedOutsideTestee, QX_JSON_MEMBER(value));

struct Root
{
    // Basic types
    bool b;
    double d;
    QString s;
    QJsonArray ja;
    QJsonObject jo;

    // // Custom containers
    QSet<QString> ss;
    QList<std::optional<bool>> lob; // Optional in container

    // Custom associative containers
    QHash<int, IntKeyable> hii;
    QMap<QString, StringKeyable> mss;

    // Optional in object
    std::optional<QString> osp; // Present optional
    std::optional<QString> osm; // Missing optional

    // Integers
    int i;
    long li;
    short si;

    // Custom object
    CustomClass cc;

    // Member with override conversion
    QString ocs;

    // Aliased member
    double ad;

    // Outside macro tests
    OutsideTestee omt;
    ExtendedOutsideTestee eomt;

    // Extended inner struct macro
    QX_JSON_STRUCT_X(
        QX_JSON_MEMBER(b),
        QX_JSON_MEMBER(d),
        QX_JSON_MEMBER(s),
        QX_JSON_MEMBER(ja),
        QX_JSON_MEMBER(jo),
        QX_JSON_MEMBER(ss),
        QX_JSON_MEMBER(lob),
        QX_JSON_MEMBER(hii),
        QX_JSON_MEMBER(mss),
        QX_JSON_MEMBER(osp),
        QX_JSON_MEMBER(osm),
        QX_JSON_MEMBER(i),
        QX_JSON_MEMBER(li),
        QX_JSON_MEMBER(si),
        QX_JSON_MEMBER(cc),
        QX_JSON_MEMBER(ocs),
        QX_JSON_MEMBER_ALIASED(ad, "aliasedDouble"),
        QX_JSON_MEMBER(omt),
        QX_JSON_MEMBER(eomt)
    );

    bool operator==(const Root& other) const = default;
};

QX_JSON_MEMBER_OVERRIDE(Root, ocs,
    static Qx::JsonError fromJson(QString& member, const QJsonValue& jv)
    {
        // Add prefix when parsing
        member = "Prefix: " + jv.toString();
        return Qx::JsonError();
    }

    static QString toJson(const QString& member)
    {
        // Remove prefix when serializing

        return QString(member).remove("Prefix: ");
    }
)

namespace QxJson
{

// Keygens
template<>
int keygen<int, IntKeyable>(const IntKeyable& value)
{
    return value.key;
};

template<>
QString keygen<QString, StringKeyable>(const StringKeyable& value)
{
    return value.key;
};


template<>
struct Converter<CustomClass>
{
    static Qx::JsonError fromJson(CustomClass& value, const QJsonValue& jValue)
    {
        const QString ERR_ACTION = u"Error converting Custom Class"_s;
        if(!jValue.isObject())
            return Qx::JsonError(ERR_ACTION, Qx::JsonError::TypeMismatch);

        QJsonObject jo = jValue.toObject();

        if(!jo.contains("ccb") || !jo.contains("ccd"))
            return Qx::JsonError(ERR_ACTION, Qx::JsonError::MissingKey);

        QJsonValue jvCcb = jo.value("ccb");
        QJsonValue jvCcd = jo.value("ccd");

        if(!jvCcb.isBool() || !jvCcd.isDouble())
            return Qx::JsonError(ERR_ACTION, Qx::JsonError::TypeMismatch);

        value.setB(jvCcb.toBool());
        value.setD(jvCcd.toDouble());

        return Qx::JsonError();
    }

    static QJsonObject toJson(const CustomClass& value)
    {
        // Use value to return a valid JSON type (i.e. qjson_type concept)
        return {{"ccb", value.b()}, {"ccd", value.d()}};
    }
};
}

// Setup
tst_qx_json::tst_qx_json() {}

// Cases
void tst_qx_json::full_declarative_suite()
{
#ifdef COMPATIBLE_COMPILER // JSON-tied structs crash GCC until version 11 (12?)
    // Populate the very complex root
    Root rOut{
        .b = true,
        .d = 4.0,
        .s = "string",
        .ja = {true, "2", 3},
        .jo = {{"key1", 1.0}, {"key2", "2"}},
        .ss = {"setOne", "setTwo"},
        .lob = {true, std::nullopt, false},
        .hii = {{1, {.key = 1, .value = 1}}}, // One value to keep Root comparable since QHash iteration order is undefined
        .mss = {{"1", {.key = "1", .value = 1}}, {"2", {.key = "2", .value = 2}}},
        .osp = "present_optional",
        .osm = std::nullopt,
        .i = 10,
        .li = 20,
        .si = 30,
        .cc = CustomClass(true, 4.0),
        .ocs = "Prefix: OriginalString",
        .ad = 17.4,
        .omt = {.value = true},
        .eomt = {.value = false}
    };

    // Test string path overload, which tests all other overloads recursively
    QTemporaryDir td;
    QString filePath = td.filePath("full_declarative_suite.json");

    // Serialize
    Qx::JsonError serializeError = Qx::serializeJson(filePath, rOut);
    QVERIFY2(!serializeError, qPrintable("Error serializing root! " + Qx::Error(serializeError).toString()));

    /* Adjust for nullopt in array, should be ignored when serialized so we need to remove it from the
     * struct before comparing against the result
     */
    rOut.lob.removeAt(1);

    // Parse back file
    Root rIn;
    Qx::JsonError parseError = Qx::parseJson(rIn, filePath);
    QVERIFY2(!parseError, qPrintable("Error parsing root! " + Qx::Error(parseError).toString()));

    // Ensure process is lossless
    QCOMPARE(rIn, rOut);    
#else
    QSKIP("GCC < 11 suffers an ICE from the compilation of declarative Qx JSON");
#endif
}

QTEST_APPLESS_MAIN(tst_qx_json)
#include "tst_qx_json.moc"
