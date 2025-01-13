//! [3]
struct MyStruct
{
    int number;
    QString name;
    
    QX_JSON_STRUCT(number, name);
}
//! [3]

//! [4]
struct MyStruct
{
    int number;
    QString name;
    
    QX_JSON_STRUCT_X(
        QX_JSON_MEMBER(number),
        QX_JSON_MEMBER_ALIASED(name, "aliasName")
    );
}
//! [4]

//! [5]
struct MyStruct
{
    int number;
    QString name;
}

// At global scope
QX_JSON_STRUCT_OUTSIDE(number, name);
//! [5]

//! [6]
struct MySpecialStruct
{
    int number;
    QString name;
    
    QX_JSON_STRUCT(number, name);     
}

QX_JSON_MEMBER_OVERRIDE(MySpecialStruct, name,
    static Qx::JsonError fromJson(QString& member, const QJsonValue& jv)
    {
        // Add prefix when parsing
        member = "Prefix" + jv.toString();
        return Qx::JsonError();
    }
    
    static QString toJson(const QString& member)
    {
        // Remove prefix when serializing
        return QString(member).remove("Prefix");
    }
)
//! [6]

//! [7]
QJsonArray ja;
// Somehow populate array...

for(int i = 0; i < ja.size(); ++i)
{
    // Parse element
    if(Qx::JsonError je = parseMyElement(ja[i]); je.isValid())
        return je.withContext(QxJson::Array()).withContext(QxJson::ArrayElement(i));
}
//! [7]

//! [8]
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
        
        static QJsonObject toJson(const MyType& value)
        {
            // Use value to return a valid JSON type (i.e. qjson_type concept)
            return objFromMyType(value);
        }
    };
}
//! [8]

//! [9]
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
//! [9]