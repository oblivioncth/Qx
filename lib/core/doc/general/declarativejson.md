Qx Declarative JSON {#declarativejson}
======================================

Qx features a highly flexible, simple to use, declarative mechanism for parsing/serializing JSON data into user structs and other types.

For example, the following JSON data:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.json}
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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

can easily be parsed into a corresponding set of C++ data structures like so:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#include <qx/core/qx-json.h>

struct Info
{
    int rating;
    bool cool;
    
    QX_JSON_STRUCT(rating, cool);
};

struct MyJson
{
    QString title;
    Info info;
    QList<QString> reviews;
    
    QX_JSON_STRUCT(title, info, reviews);
};

int main()
{
    QFile jsonFile("data.json");
    MyJson myJsonDoc;

    // Parse into custom structures
    Qx::JsonError je = Qx::parseJson(myJsonDoc, jsonFile);
    Q_ASSERT(!je.isValid());
    
    ...
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Likewise, the structure can be serialized back out into textual JSON data with:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
int main()
{
    ...
    
    // Serialize to JSON
    je = Qx::serializeJson(jsonFile, myJsonDoc);
    Q_ASSERT(!je.isValid());
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This system is accessed through the qx-json.h header, predominantly with QX_JSON_STRUCT().