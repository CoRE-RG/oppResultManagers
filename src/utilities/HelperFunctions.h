#ifndef HELPERFUNCTIONS_H_
#define HELPERFUNCTIONS_H_

#include <omnetpp.h>

#include <vector>

std::vector< std::pair< simtime_t, simtime_t > > parseIntervals(const char* text);

bool inIntervals(simtime_t t, std::vector< std::pair< simtime_t, simtime_t > > &intervals);

#endif /* HELPERFUNCTIONS_H_ */
