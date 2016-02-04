

#include "opppostgresqlutils.h"


int opp_postgresql_substitute(std::string& query, const char *substring, const char *value, pqxx::work *transaction)
{
    if (!substring || !substring[0])
        throw cRuntimeError("opp_query_substitute(): substring cannot be NULL or ''");
    if (!value)
        value = "";

    std::string quotedvalue = transaction->esc(value);

    size_t count = 0;
    int pos = 0;
    while ((pos = query.find(substring,pos)) != -1)
    {
        query.replace(pos, strlen(substring), quotedvalue);
        pos += quotedvalue.length();
        count++;
    }

    if (count == 0)
        throw cRuntimeError("opp_postgresql_substitute(): substring '%s' not found in '%s'", substring, query.c_str());

    return count;
}

int opp_postgresql_substitute(std::string& query, const char *substring, long value, pqxx::work *transaction)
{
    char buf[32];
    sprintf(buf, "%ld", value);
    return opp_postgresql_substitute(query, substring, buf, transaction);
}
