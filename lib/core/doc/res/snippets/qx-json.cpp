//! [0]
{
    "title": "Sample JSON Data",
    "info": {
        "rating": 10,
        "cool": true
    },
    "reviews": [
        "Wicked!",
        "Awesome!",
        "Fantastic!"
    ]
}
//! [0]

//! [1]
#include <qx/core/qx-json.h>

struct Info
{
    int rating;
    bool cool;
    
    QX_JSON_STRUCT(rating, cool);
}

struct MyJson
{
    QString title;
    Info info;
    QList<QString> reviews;
    
    QX_JSON_STRUCT(title, info, reviews);
};

void main()
{
    // Get JSON data
    QFile jsonFile("data.json");
    Q_ASSERT(jsonFile.open(QIODevice::ReadOnly));

    QByteArray jsonData = jsonFile.readAll();
    jsonFile.close();

    MyJson myJsonDoc;

    // Parse raw JSON data
    QJsonDocument jd = QJsonDocument::fromJson(jsonData);
    Q_ASSERT(!jd.isEmpty());

    // Parse into custom structures
    Qx::JsonError je = Qx::parseJson(myJsonDoc, jd);
    Q_ASSERT(!je.isValid());
}
//! [1]

//! [2]
struct MyStruct
{
    int number;
    QString name;
    
    QX_JSON_STRUCT(number, name);
}
//! [2]

//! [3]
struct MySpecialStruct
{
    int number;
    QString name;
    
    QX_JSON_STRUCT(number, name);
    
    QX_JSON_DECLARE_MEMBER_OVERRIDES();
    
    QX_JSON_MEMBER_OVERRIDE(name,
        static Qx::JsonError fromJson(QString& member, const QJsonValue& jv)
        {
            member = "OverridenName";
            return Qx::JsonError();
        }
    )
}
//! [3]

//! [4]
class MyType
{
    ...
};

namespace QxJson
{
    template<>
    struct Converter<MyType>
    {
        static Qx::JsonError fromJson(MyType& value, const QJsonValue& jValue)
        {
            // Assign `value` to complete the conversion
            value = //...
            
            // Return an invalid JsonError upon success, or a valid one if an error occurs
            return Qx::JsonError();
        }
    };
}
//! [4]

//! [5]
struct MyStruct
{
    int number;
    QString name;
    
    QX_JSON_STRUCT(number, name);
};

namespace QxJson
{
    template<>
    QString keygen<QString, MyStruct>(const MyStruct& value)
    {
        // This specialization enables the use of QHash<QString, MyStruct>
        // or QMap<QString, MyStruct> to represent a JSON array of JSON objects
        // that are tied to MyStruct. The 'name' member is used as the key for each value.
        return value.name;
    };
}

// Use in another struct
struct OtherStruct
{
    bool enabled;
    QMap<QString, MyStruct> myStructs;
    
    QX_JSON_STRUCT(enabled, myStructs);
};
//! [5]