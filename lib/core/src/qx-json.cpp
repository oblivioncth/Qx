// Unit Includes
#include "qx/core/qx-json.h"
#include "qx-json_p.h"

// Qt Includes
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonDocument>

/*!
 *  @file qx-json.h
 *  @ingroup qx-core
 *
 *  @brief The qx-json header file provides various utilities for JSON data manipulation.
 *
 *  The mechanisms of this file introduce a highly flexible, simple to use, declarative
 *  mechanism for parsing JSON data into user structs and other types.
 *
 *  For example, the following JSON data:
 *  @snippet qx-json.cpp 0
 *
 *  can easily be parsed into a corresponding set of C++ data structures like so:
 *  @snippet qx-json.cpp 1
 *
 *  @sa QX_JSON_STRUCT(), and QxJson.
 */

/*!
 *  @def QX_JSON_MEMBER()
 *
 *  Used to denote a JSON struct member whose name matches its key when using QX_JSON_STRUCT_X().
 *
 *  @sa QX_JSON_STRUCT_X()
 */

/*!
 *  @def QX_JSON_MEMBER_ALIASED(member, key)
 *
 *  Used to denote a JSON struct member with a key that is different from its name when
 *  using QX_JSON_STRUCT_X().
 *
 *  @a key is to be a string literal.
 *
 *  @sa QX_JSON_STRUCT_X()
 */

/*!
 *  @def QX_JSON_STRUCT()
 *
 *  Specifies that a struct is a JSON-tied struct, which enables support for automatic
 *  parsing of a corresponding JSON object.
 *
 *  The name of each included member must match the name their corresponding JSON key.
 *
 *  @snippet qx-json.cpp 2
 *
 *  @sa QX_JSON_STRUCT_OUTSIDE() and QX_JSON_STRUCT_X()
 */

/*!
 *  @def QX_JSON_STRUCT_X()
 *
 *  Same as QX_JSON_STRUCT(), but allows for customization of member keys.
 *
 *  Each member must be wrapped in QX_JSON_MEMBER() or QX_JSON_MEMBER_ALIASED().
 *
 *  @snippet qx-json.cpp 3
 */

/*!
 *  @def QX_JSON_STRUCT_OUTSIDE()
 *
 *  Same as QX_JSON_STRUCT(), but is used outside of a struct instead of inside.
 *
 *  @snippet qx-json.cpp 4
 *
 *  This is useful for hiding the JSON parsing implementation details of a public
 *  struct within a different source file.
 *
 *  @sa QX_JSON_STRUCT()
 */

/*!
 *  @def QX_JSON_STRUCT_OUTSIDE_X()
 *
 *  Same as QX_JSON_STRUCT_X(), but is used outside of a struct instead of inside.

 *  @sa QX_JSON_STRUCT_OUTSIDE()
 */


/*!
 *  @def QX_JSON_MEMBER_OVERRIDE()
 *
 *  Used to define a member/key specific value parsing override for a JSON-tried struct.
 *  The specified member will be parsed using the provided function instead of a
 *  potentially available generic one for that type.
 *
 *  Must be used in namespace scope, outside of the struct definition.
 *
 *  @snippet qx-json.cpp 5
 */

namespace Qx
{

//===============================================================================================================
// JsonError
//===============================================================================================================

//-Class Enums-------------------------------------------------------------
/*!
 *  @class JsonError qx/core/qx-json.h
 *
 *  @brief The JsonError class is used to report errors related to JSON manipulation.
 */

/*!
 *  @enum JsonError::Form
 *
 *  This enum represents form of JSON error.
 *
 *  @var JsonError::Form JsonError::NoError
 *  No error occurred.
 *
 *  @var JsonError::Form JsonError::MissingKey
 *  An expected key was missing.
 */

//-Constructor-----------------------------------------------------------------
/*!
 *  Creates an invalid JsonError.
 */
JsonError::JsonError() :
    mAction(),
    mForm(Form::NoError)
{}

/*!
 *  Creates a JSON error with the action @a and error form @a f.
 */
JsonError::JsonError(const QString& a, Form f) :
    mAction(a),
    mForm(f)
{}

//-Instance Functions-------------------------------------------------------------
//Private:
quint32 JsonError::deriveValue() const { return mForm; };
QString JsonError::derivePrimary() const { return mAction; };
QString JsonError::deriveSecondary() const { return ERR_STRINGS.value(mForm); };

QString JsonError::deriveDetails() const
{
    QString details = u"JSON Node Stack:\n"_s;
    for(const auto& node : mContext)
        details += u"- "_s + std::visit([](auto&& n){ return n.string(); }, node) + u"\n"_s;
    details += u"\nAction: "_s + derivePrimary() + u"\n\n"_s + u"Error: "_s + deriveSecondary();

    return details;
}

//Public:
/*!
 *  Returns @c true if an error occurred; otherwise, returns @c false.
 */
bool JsonError::isValid() const { return mForm != NoError; }

/*!
 *  A message noting the attempted action that failed.
 */
QString JsonError::action() const { return mAction; }

/*!
 *  The form of error that occurred.
 */
JsonError::Form JsonError::form() const { return mForm; }

/*!
 *  Provides the full context in which the error occurred.
 */
QList<QxJson::ContextNode> JsonError::context() const { return mContext; }

/*!
 *  Prepends the context node @a node to the error's context and returns a reference
 *  to the updated error object. This function is a no-op if the error is not valid.
 *
 *  Generally this function is of no use in user code, but can be utilized when
 *  providing complex custom parsing for user types via QX_JSON_MEMBER_OVERRIDE
 *  or Converter specializations.
 *
 *  Typically, this function is used in a chain of return calls in order to pass
 *  the context of where an error occurred up the call stack. For example, one might
 *  return information about a JSON array related error like so:
 *
 *  @snippet qx-json.cpp 6
 */
JsonError& JsonError::withContext(const QxJson::ContextNode& node)
{
    if(!isValid())
        return *this;

    mContext.prepend(node); return *this;
}

//===============================================================================================================
// QJsonParseErrorAdapter
//===============================================================================================================

/*!
 *  @class QJsonParseErrorAdapter
 *  @ingroup qx-core
 *
 *  @brief Allows QJsonParseError to be used via the Qx::Error interface.
 *
 *  All errors are assigned the severity of Severity::Err.
 *
 *  @note This class exists in order to provide an implicit conversion between QJsonParseError
 *  and Error, and generally should not be used directly.
 */

//-Constructor---------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs an Error adapter for the type QJsonParseError by observing the reference @a e.
 */
QJsonParseErrorAdapter::QJsonParseErrorAdapter(const QJsonParseError& e) :
    mErrorRef(e)
{}

//Private:
quint32 QJsonParseErrorAdapter::deriveValue() const { return mErrorRef.error; }
QString QJsonParseErrorAdapter::derivePrimary() const { return mErrorRef.errorString(); }
QString QJsonParseErrorAdapter::deriveSecondary() const { return OFFSET_STR.arg(mErrorRef.offset); }

//===============================================================================================================
// <namepace>
//===============================================================================================================

//-Concepts---------------------------------------------------------------------------------------------
/*!
 *  @concept json_root
 *  @brief Specifies that a type is a valid analogue for a JSON document root element.
 *
 *  Satisfied if @a T is a JSON-tied struct (an analogue for a root object), or a container class
 *  with a value type for which there is a QxJson::Converter specialization (an analogue for a root
 *  array).
 */

//-Functions---------------------------------------------------------------------------------------------
//Public:
/*!
 *  @fn JsonError parseJson(T& parsed, const QString& filePath)
 *
 *  Parses the entire JSON document file at path @a filePath and stores the result in @a parsed.
 *  @a T must satisfy the @ref json_root concept.
 *
 *  If parsing fails, a valid JsonError is returned that describes the cause; otherwise, an invalid
 *  error is returned.
 */

/*!
 *  @fn JsonError parseJson(T& parsed, QFile& file)
 *
 *  Parses the entire JSON document file @a file and stores the result in @a parsed.
 *  @a T must satisfy the @ref json_root concept.
 *
 *  If parsing fails, a valid JsonError is returned that describes the cause; otherwise, an invalid
 *  error is returned.
 */

/*!
 *  @fn JsonError parseJson(T& parsed, const QJsonDocument& doc)
 *
 *  Parses the entire JSON document @a doc and stores the result in @a parsed.
 *  @a T must satisfy the @ref json_root concept.
 *
 *  If parsing fails, a valid JsonError is returned that describes the cause; otherwise, an invalid
 *  error is returned.
 */

/*!
 *  @fn JsonError parseJson(T& parsed, const QJsonObject& obj)
 *
 *  @overload
 *
 *  Parses the JSON object @a obj and stores the result in @a parsed.
 *  @a T must satisfy the @ref QxJson::json_struct concept.
 */

/*!
 *  @fn JsonError parseJson(T& parsed, const QJsonArray& array)
 *
 *  @overload
 *
 *  Parses the JSON array @a array and stores the result in @a parsed.
 *  @a T must satisfy the @ref QxJson::json_containing concept.
 */

/*!
 *  Recursively searches @a rootValue for @a key and returns the associated value for
 *  all matches as a list, or an empty list if the key was not found.
 *
 *  If @a rootValue is of any type other than QJsonValue::Array or QJsonValue::Object
 *  then returned list will always be empty.
 */
QList<QJsonValue> findAllValues(const QJsonValue& rootValue, QStringView key)
{
    QList<QJsonValue> hits;
    recursiveValueFinder(hits, rootValue, key);
    return hits;
}

/*!
 *  Returns the JSON string representation of @a value regardless of its type.
 *
 *  If @a value is an object or array, the returned string will be in the compact format.
 */
QString asString(const QJsonValue& value)
{
    QJsonValue::Type valueType = value.type();
    if(valueType == QJsonValue::Type::Array)
    {
        QJsonDocument formatter(value.toArray());
        return QString::fromLatin1(formatter.toJson(QJsonDocument::Compact));
    }
    else if(valueType == QJsonValue::Type::Object)
    {
        QJsonDocument formatter(value.toObject());
        return QString::fromLatin1(formatter.toJson(QJsonDocument::Compact));
    }
    else if(valueType == QJsonValue::Type::Bool)
        return value.toBool() ? u"true"_s : u"false"_s;
    else if(valueType == QJsonValue::Type::Double)
        return QString::number(value.toDouble());
    else if(valueType == QJsonValue::Type::String)
        return value.toString();
    else // Covers Null & Undefined
        return QString();
}

} // namespace Qx

namespace QxJson
{

/*!
 *  @namespace QxJson
 *
 *  @brief The @c QxJson namespace encapsulates the user-extensible implementation of Qx's JSON parsing facilities.
 */

//===============================================================================================================
// Nodes
//===============================================================================================================

/*!
 *  @class File
 *  @brief The file class represents a JSON file node for use in error contexts.
 *
 *  @note This class is irrelevant in user code except for some instances of complex
 *  custom JSON parsing
 *
 *  @sa Qx::JsonError::withContext().
 */

/*!
 *  Constructs a file node with the identifier of @a filename and file specific error
 *  information @a fileError, if any.
 */
File::File(const QString& filename, const QString& fileError) :
    mIdentifier(filename),
    mFileError(fileError)
{}

/*!
 *  Constructs a file node with the identifier set to the filename of @a docFile and
 *  file specific error information @a fileError, if any.
 */
File::File(const QFile& docFile, const QString& fileError) :
    mIdentifier(docFile.fileName()),
    mFileError(fileError)
{}

/*!
 *  Constructs a file node with the identifier set to the absolute path of @a docFile
 *  and file specific error information @a fileError, if any.
 */
File::File(const QFileInfo& docFile, const QString& fileError) :
    mIdentifier(docFile.absoluteFilePath()),
    mFileError(fileError)
{}

/*!
 *  Returns the string representation of the node.
 */
QString File::string() const
{
    QString str = u"File: "_s + mIdentifier;
    if(!mFileError.isEmpty())
        str += (u"\n  [%1]"_s).arg(mFileError);

    return str;
}

/*!
 *  @class Document
 *  @brief The document class represents a JSON document node for use in error contexts.
 *
 *  @note This class is irrelevant in user code except for some instances of complex
 *  custom JSON parsing
 *
 *  @sa Qx::JsonError::withContext().
 */

/*!
 *  Constructs a document element node with the name @a name, which defaults to no name.
 */
Document::Document(const QString& name) : mName(name) {}

/*!
 *  Returns the string representation of the node.
 */
QString Document::string() const { return u"Document"_s + (!mName.isNull() ? u": "_s + mName : QString()); }

/*!
 *  @class Object
 *  @brief The object class represents a JSON object node for use in error contexts.
 *
 *  @note This class is irrelevant in user code except for some instances of complex
 *  custom JSON parsing
 *
 *  @sa Qx::JsonError::withContext().
 */

/*!
 *  Constructs an object element node.
 */
Object::Object() {}

/*!
 *  Returns the string representation of the node.
 */
QString Object::string() const { return u"Object"_s; }

/*!
 *  @class ObjectKey
 *  @brief The object key class represents a JSON Object key node for use in error contexts.
 *
 *  @note This class is irrelevant in user code except for some instances of complex
 *  custom JSON parsing
 *
 *  @sa Qx::JsonError::withContext().
 */

/*!
 *  Constructs an object key node with the key name @a name.
 */
ObjectKey::ObjectKey(const QString& name) : mName(name) {}

/*!
 *  Returns the string representation of the node.
 */
QString ObjectKey::string() const { return u"Key: "_s + mName; }

/*!
 *  @class Array
 *  @brief The array class represents a JSON array node for use in error contexts.
 *
 *  @note This class is irrelevant in user code except for some instances of complex
 *  custom JSON parsing
 *
 *  @sa Qx::JsonError::withContext().
 */

/*!
 *  Constructs an array element node.
 */
Array::Array() {}

/*!
 *  Returns the string representation of the node.
 */
QString Array::string() const { return u"Array"_s; }

/*!
 *  @class ArrayElement
 *  @brief The array element key class represents a JSON Array element node for use in error contexts.
 *
 *  @note This class is irrelevant in user code except for some instances of complex
 *  custom JSON parsing
 *
 *  @sa Qx::JsonError::withContext().
 */

/*!
 *  Constructs an array element node with the element number @a element.
 */
ArrayElement::ArrayElement(uint element) : mElement(element) {}

/*!
 *  Returns the string representation of the node.
 */
QString ArrayElement::string() const { return (u"Element: %1"_s).arg(mElement); }

/*!
 *  @typedef ContextNode
 *  Any JSON context node type.
 */

//===============================================================================================================
// <namepace>
//===============================================================================================================

/*!
 *  @struct Converter
 *
 *  @brief The Converter template struct acts as an interface that carries details on how to parse JSON to various types.
 *
 *  JSON data can be converted to an object of any type as Converter provides a specialization for that type that
 *  contains a corresponding fromJson() function.
 *
 *  By default, conversions are provided for:
 *  - bool
 *  - double
 *  - Integer Types
 *  - QString
 *  - QJsonArray
 *  - QJsonObject
 *  - QList<T>
 *  - QSet<T>
 *  - QHash<K, T> (when a keygen() specialization exists for K)
 *  - QMap<K, T> (when a keygen() specialization exists for K)
 *
 *  Support for additional, non-structural types can be added like so:
 *  @snippet qx-json.cpp 7
 *
 *  If a structural type needs to be registered, the QX_JSON_STRUCT and QX_JSON_MEMBER macros should be used
 *  instead.
 *
 *  @sa qx-json.h and keygen().
 */

/*!
 *  @fn template<typename Key, class Value> Key keygen(const Value& value)
 *
 *  @brief The keygen template function acts as an interface through which the derivation of a key
 *  for a given type when used in associative containers is defined.
 *
 *  Any otherwise convertible JSON type can be parsed into a map as long as a specialization of keygen() exists
 *  for that type.
 *
 *  Support for additional types can be added like so:
 *  @snippet qx-json.cpp 8
 *
 *  @sa qx-json.h and Converter.
 */

}
