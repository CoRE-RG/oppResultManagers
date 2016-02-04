#ifndef __OPPPOSTGRESQLUTILS_H__
#define __OPPPOSTGRESQLUTILS_H__

#include <pqxx/pqxx>

#include <omnetpp.h>


int opp_postgresql_substitute(std::string& query, const char *substring, const char *value, pqxx::work *transaction);

int opp_postgresql_substitute(std::string& query, const char *substring, long value, pqxx::work *transaction);

#endif

