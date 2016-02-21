#include "HelperFunctions.h"

#include <omnetpp.h>

std::vector<std::pair<simtime_t, simtime_t> > parseIntervals(const char* text)
{
    std::vector<std::pair<simtime_t, simtime_t>> parsedIntervals;
    std::vector<std::string> intervals = cStringTokenizer(text, ",").asVector();
    for (std::vector<std::string>::const_iterator interval = intervals.begin(); interval != intervals.end(); ++interval)
    {
        simtime_t starttime = 0;
        simtime_t endtime = 0;
        size_t pos = (*interval).find("..");
        if (pos == std::string::npos)
        {
            throw cRuntimeError("Wrong syntax in interval %s=%s", text, (*interval).c_str());
        }
        else if (pos == 0)
        {
            endtime = STR_SIMTIME(std::string((*interval), 2, (*interval).length() - 2).c_str());
        }
        else if (pos == ((*interval).length() - 2))
        {
            starttime = STR_SIMTIME(std::string((*interval), 0, (*interval).length() - 2).c_str());
        }
        else
        {
            starttime = STR_SIMTIME(std::string((*interval), 0, pos).c_str());
            endtime = STR_SIMTIME(std::string((*interval), pos + 2, (*interval).length() - pos - 2).c_str());
            if (starttime > endtime)
            {
                throw cRuntimeError("Wrong time in interval %s=%s, starttime cannot be later than endtime", text,
                        (*interval).c_str());
            }
        }
        ev << "starttime:"<<starttime<<" endtime:"<<endtime<<endl;
        parsedIntervals.push_back(std::pair<simtime_t, simtime_t>(starttime, endtime));
    }
    return parsedIntervals;
}

bool inIntervals(simtime_t t, std::vector<std::pair<simtime_t, simtime_t> > &intervals)
{
    if (intervals.size() > 0)
    {
        //Check intervals
        bool inIntervals = false;
        for (std::vector<std::pair<simtime_t, simtime_t>>::iterator interval = intervals.begin();
                interval != intervals.end();)
        {
            if (t > (*interval).first && t < (*interval).second)
            {
                inIntervals = true;
                break;
            }
            else if (t > (*interval).first && t > (*interval).second)
            {
                intervals.erase(interval);
            }
            else
            {
                ++interval;
            }
        }
        return inIntervals;
    }
    else{
        //if there are no intervals return true
        return true;
    }

}
