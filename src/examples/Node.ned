//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package oppresultmanagers.examples;

//
// TODO auto-generated module
//
simple Node
{
    parameters:
        //Signal is emitted when when message is received. Contains the received message.
        @signal[rxMessageAge](type=simtime_t);
        @statistic[rxMessageAge](title="Message Age"; unit=s; record=max,vector; interpolationmode=none);
        
        string stest = default("teststring");
    gates:
        inout port;
}
