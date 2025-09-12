Qx Declarative SQL {#declarativesql}
======================================

Qx features a highly flexible, simple to use, declarative mechanism for querying SQL databases using user structs and other types.

For example, the following SQL table:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
person:
    name: VARCHAR
    id: INTEGER
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

can easily be parsed into a list of C++ data structures like so:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#include <qx/sql/qx-sqldatabase.h>
#include <qx/sql/qx-sqlquery.h>

struct person
{
    QString name;
    int id;

    QX_SQL_STRUCT(id);
};

int main()
{
    Qx::SqlDatabase myDb(u"C:/Some/db.sqlite"_s, "QSQLITE");
    QList<person> allPersons;

    Qx::SqlError err =
        myDb.SELECT<person>()
            .FROM<person>()
            .execute(allPersons);

    Q_ASSERT(!err.isValid());

    ...
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Likewise, the structure can used in DML queries to add data to a database:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
int main()
{
    ...

    // Add to DB
    db.INSERT_INTO(person::Sql::_).VALUES<person>()
    ...
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This system is accessed through the Qx SQL module, predominantly with QX_SQL_STRUCT() et al.

TODO: Make this more complete.