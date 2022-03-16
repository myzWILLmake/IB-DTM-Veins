# IB-DTM-Veins

This project is based on [IB_DTM-framework](https://github.com/pga2rn/IB-DTM_framework). Simulation built on Veins provides more realistic vehicle movement and network between components.

# Overview

The simulation framework implements a N * M grid, each cross in the map represents a **RSU**. The simulated **vehicles** will move from one cross to another cross, enter or leave the map, from time to time. The RSU collects the **trust value offset** of each vehicle, and further calculate the **trust value** of each vehicle.

The time stream of simulation is divided into many **epochs**, each epoch contains several **slots** which lasts for many seconds. The whole map will update at every slot, move vehicles and generate trust value offsets for each vehicle. At the end of each epoch, the trust value of each vehicles will be generated.

# Usage

1. install OMNeT++ and Veins(5.1)
2. import the project into OMNeT++
3. run `veins_launchd` in the background, and run `omnetpp.ini` in `ib_dtm` project folder.


