# Forwhaetver reason, putting these in DOXYGEN_EXPAND_AS_DEFINED does not work, possibly because
# there are nested macros, so we have to define all of the macros used to create symobls that need
# documentation in the SQL module here in a way that allows Doxygen to recognize said symbols.
# The function definition isn't needed, just the declaration.
#
# NOTE: @ seems not to work here, so we need to use \\ for doxygen commands instead (\ itself escaped)

list(APPEND DOXYGEN_PREDEFINED
    # Queries
    "__QX_SQL_QUERY_ADD_KEYWORD_ZERO_ARG_X(keyword, method)=\
        /*! Adds a `keyword` clause to the query and returns a reference to the query. */ \
        auto& method();"
    "__QX_SQL_QUERY_ADD_KEYWORD_ZERO_ARG(keyword)=\
        /*! Adds a `keyword` clause to the query and returns a reference to the query. */ \
        auto& keyword();"
    "__QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG_X(keyword, method)=\
        /*! Adds a `keyword` clause to the query using \\a fs and returns a reference to the query. */ \
        template<sql_stringable First> auto& method(First&& fs);"
    "__QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG_PAREN_X(keyword, method)=\
        /*! Adds a `keyword` clause to the query using \\a fs and returns a reference to the query. */ \
        template<sql_stringable First> auto& method(First&& fs);"
    "__QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG(keyword)=\
        /*! Adds a `keyword` clause to the query using \\a fs and returns a reference to the query. */ \
        template<sql_stringable First> auto& keyword(First&& fs);"
    "__QX_SQL_QUERY_ADD_KEYWORD_SINGLE_ARG_PAREN(keyword)=\
        /*! Adds a `keyword` clause to the query using \\a fs and returns a reference to the query. */ \
        template<sql_stringable First> auto& keyword(First&& fs);"
    "__QX_SQL_QUERY_ADD_KEYWORD_MULTI_ARG_X(keyword, method)=\
        /*! Adds a `keyword` clause to the query using \\a fs through \\a s and returns a reference \
        to the query. */ \
        template<sql_stringable First, sql_stringable ...Rest> auto& method(First&& fs, Rest&&... s);"
    "__QX_SQL_QUERY_ADD_KEYWORD_MULTI_ARG_PAREN_X(keyword, method)=\
        /*! Adds a `keyword` clause to the query using \\a fs through \\a s and returns a reference \
        to the query. */ \
        template<sql_stringable First, sql_stringable ...Rest> auto& method(First&& fs, Rest&&... s);"
    "__QX_SQL_QUERY_ADD_KEYWORD_MULTI_ARG(keyword)=\
        /*! Adds a `keyword` clause to the query using \\a fs through \\a s and returns a reference \
        to the query. */ \
        template<sql_stringable First, sql_stringable ...Rest> auto& keyword(First&& fs, Rest&&... s);"
    "__QX_SQL_QUERY_ADD_KEYWORD_MULTI_ARG_PAREN(keyword)=\
        /*! Adds a `keyword` clause to the query using \\a fs through \\a s and returns a reference \
        to the query. */ \
        template<sql_stringable First, sql_stringable ...Rest> auto& keyword(First&& fs, Rest&&... s);"
    "__QX_SQL_QUERY_ADD_KEYWORD_SUB_QUERY_X(keyword, method)=\
        /*! Adds a `keyword` clause to the query using \\a q as a sub-query and returns a reference \
        to the query. */ \
        auto& method(const SqlQuery& q);"
    "__QX_SQL_QUERY_ADD_KEYWORD_SUB_QUERY(keyword)=\
        /*! Adds a `keyword` clause to the query using \\a q as a sub-query and returns a reference \
        to the query. */ \
        auto& keyword(const SqlQuery& q);"
)
