# oppResultManagers

oppResultManagers aims to simplify result recording and analysis of [OMNEST/OMNeT++](https://omnetpp.org/) simulations. 

<img src="/doc/images/oppresultmanagers.png" alt="oppResultManagers Environment" width="35%">


## News
* Update dependencies to OMNeT++ 5.5.1 and INET 3.6.6


## Features

### Check Vector Manager
A vector manager to check vectors of modules against an XML constraint specification 

### Database Manager
Records into Databases instead of .vec and .sca files. Supports:
* postgreSQL
* SQLite 

Databases can be analyzed with SQL statements or attached to other tools (e.g. R) 

### PCAPng Manager
Records PCAPng files instead of .elog format. PCAPng files can be opened with [Wireshark](https://www.wireshark.org/download.html). PCAPng supports multiple interfaces.

### GCTA Eventlog Manager
Records an eventlog in the .tlog format for the analysis using the Gantt Chart Timing Analyzer (additional OMNeT++ Plugin)

### Multiple Manager
Allows to use multiple Vector, Scalar or Eventlog Managers in parallel 


## Continuous Integration

The build state of the master branch is monitored:
* Building:
<p><img src="https://jenkins.core-rg.de/buildStatus/icon?job=oppResultManagers/oppResultManagers"></p>
* Tests:
<p><img src="https://jenkins.core-rg.de/buildStatus/icon?job=oppResultManagers/oppResultManagers_tests"></p>

<table>
  <tr>
    <th></th>
    <th>Ubuntu 18.04</th>
    <th>Windows 10</th>
  </tr>
  <tr>
    <td>Building</td>
    <td><img src="https://jenkins.core-rg.de/buildStatus/icon?job=oppResultManagers/oppResultManagers/Nodes=Ubuntu_18.04"></td>
    <td><img src="https://jenkins.core-rg.de/buildStatus/icon?job=oppResultManagers/oppResultManagers/Nodes=Windows_10"></td>
  </tr>
  <tr>
    <td>Tests</td>
    <td><img src="https://jenkins.core-rg.de/buildStatus/icon?job=oppResultManagers/oppResultManagers_tests/Nodes=Ubuntu_18.04"></td>
    <td><img src="https://jenkins.core-rg.de/buildStatus/icon?job=oppResultManagers/oppResultManagers_tests/Nodes=Windows_10"></td>
  </tr>
</table>


## Further Information
* [CoRE simulation models overview](https://core-researchgroup.de/projects/simulation.html)
* [Abstract Network Description Language (ANDL) overview](https://core-researchgroup.de/projects/simulation/abstract-network-description-language.html)

### Installation
Please see [INSTALL](/INSTALL)

### Documentation
Please see [doc/](/doc)


## IMPORTANT
The oppResultManagers is under continuous development: new parts are added, bugs are corrected, and so on. We cannot assert that the implementation will work fully according to the specifications. YOU ARE RESPONSIBLE YOURSELF TO MAKE SURE THAT THE MODELS YOU USE IN YOUR SIMULATIONS WORK CORRECTLY, AND YOU'RE GETTING VALID RESULTS.
