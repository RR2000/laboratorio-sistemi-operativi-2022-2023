# Cargo Ship Traffic Simulator

This project aims to simulate the traffic of cargo ships transporting various types of goods through ports. It is achieved through several processes:
- A master process responsible for creating other processes and managing the simulation as necessary
- A number of ship processes (SO_NAVI ≥ 1)
- A number of port processes (SO_PORTI ≥ 4)

In the simulation, a day of simulated time lasts a real-time second.

## The Goods

In the simulation, there are SO_MERCI different types of goods. Each type of goods is characterized by:
- A quantity (in tons), randomly drawn between 1 and SO_SIZE at the start of the simulation
- A lifespan (in days), randomly drawn between SO_MIN_VITA and SO_MAX_VITA

Goods can be generated at the ports and they always have the same characteristics when created at runtime. Goods can disappear when their lifespan expires, no matter where they are (ship or port). 

## The Map

The world map is represented by a square of side SO_LATO (measured in kilometers). Both ports and ships have a position on the map. Navigation between two points on the map can always happen in a straight line.

## The Ship Process

Each ship has:
- A speed SO_SPEED (in kilometers per day)
- A position (a pair of coordinates within the map)
- A capacity SO_CAPACITY (in tons) measuring the total transportable load

Ships can't carry an amount of goods that exceeds their capacity. The ships start from random positions and without any goods.

## The Port Process

The port is located at a certain position on the map and manages a random number of docks between 1 and SO_BANCHINE. When the port processes are created, the demand and supply of goods are also randomly created. 

## Simulation State Dump

Every day, a provisional report is displayed containing:
- Total goods broken down by type and state
- Number of ships: in the sea with a load on board, in the sea without a load, in port, carrying out loading/unloading operations.

## Termination of the Simulation

The simulation ends in one of the following circumstances:
- After a simulated time of SO_DAYS
- When for every type of goods, the supply is zero or the demand is zero

The final report should indicate:
- Number of ships still at sea with a load on board
- Number of ships still at sea without a load
- Number of ships occupying a dock
- Total goods broken down by type and state
