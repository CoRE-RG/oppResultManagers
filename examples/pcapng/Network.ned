//
// Copyright (C) 2003 CTIE, Monash University
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//



package oppresultmanagers.examples.pcapng;

import inet.node.ethernet.EtherHost;
import inet.node.ethernet.Eth100M;


channel Eth100M50m extends Eth100M
{
    length = 50m;
}

//
// Sample Ethernet LAN: two hosts directly connected to each other
// via twisted pair.
//
network Network
{
    submodules:
        node1: EtherHost {
            parameters:
                @display("p=60,160");
        }
        node2: EtherHost {
            parameters:
                @display("p=200,140");
        }
    connections:
        node1.ethg <--> Eth100M50m{} <--> node2.ethg;
}