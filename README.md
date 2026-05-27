# cncpp 26

This project is based on the reference repository provided for the course:

- **Reference repository**: https://github.com/pbosetti/cncpp

# Lecture Notes

* check the CMAKELIST.TXT: if it's the same of the professor

## 20260506

1. How to create a complex executable which is worling in every condition:

  * we use the **finite state machine**

  * IoT Techs needed to communicate with the machine


  * per stoppare qualcosa

  kill -SIGINT PID

  cmake -S . -B build -G Ninja --log-level=VERBOSE


  ctr+z -> pausing
  bg -> resume in backgraund
  fg -> resume in foreground
  kill

  ## About Rerun viewer

  * save recording or blue-prints options!

 ## About MADS

* ` -p 5`: period for forward integration
* Triggered mode: waiting a new setpoint that arrived before calculating the current position -> literally triggered
  1. Listen for axes pos.
  2. Calculate the current position
  3. Updates


* The other method the integration follows a defined time step (-p 5ms)


We can't use the normal mode for the FMU: there wont be a perfect synchro, due to the FMU is following another clock


## COme lanciare

Run view, director: fmu through director

Need:
  * viewer
  * director
    * fmus

Launching

1. cnc command: build/cnc test.g:

  1. Z: goes to zero (in alto) -> isn't moving: ok
  2. Space: rapid motion -> start machining
  3. Z: moves to zero

Note: se le linee non sono perfettamente dritte è perché abbiamo la dinamica del sistema!

# Project Work

* mandatory written part: coding and theory + discussion of the written part + an oral

max grade with written part: 26/30

## Or: personal project

* In the CONTRACT.md all the details of the project: what I write in the contract, is what I need to get!

* at least a copule of feature, than propose one (if wnated)