//! [1]
struct MyStruct
{
    int number;
    QString name;

    QX_SQL_STRUCT(number, name);
}
//! [1]

//! [2]
struct MyStruct
{
    int number;
    QString name;

    QX_SQL_STRUCT_X(
        QX_SQL_MEMBER(number),
        QX_SQL_MEMBER_ALIASED(name, "aliasName")
    );
}
//! [2]

//! [3]
struct MyStruct
{
    int number;
    QString name;
}

// At global scope
QX_SQL_STRUCT_OUTSIDE(number, name);
//! [3]

//! [4]
struct MySpecialStruct
{
    int number;
    QString name;

    QX_SQL_STRUCT(number, name);
}

QX_SQL_MEMBER_OVERRIDE(MySpecialStruct, name,
    static Qx::SqlError fromSql(QString& member, const QVariant& vv)
    {
        // Add prefix when parsing
        member = "Prefix" + vv.toString();
        return Qx::JsonError();
    }

    static QVariant toSql(const QString& member)
    {
        // Remove prefix when serializing
        return member.remove("Prefix");
    }
)
//! [4]