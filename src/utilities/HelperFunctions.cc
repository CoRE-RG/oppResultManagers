//Copyright (c) 2016, CoRE Research Group, Hamburg University of Applied Sciences
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without modification,
//are permitted provided that the following conditions are met:
//
//1. Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
//2. Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
//3. Neither the name of the copyright holder nor the names of its contributors
//   may be used to endorse or promote products derived from this software without
//   specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
//ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
//ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
