# hgcal-tpg-fe

## Motivation
Emulation of the Trigger Primitive Generation (TPG) by HGCAL frontend ASICs.

## Download and Setup
Run the setup code to download additional dependencies and setup. This setup is prepared for lxplus machines.

```
./setup.sh
```

## Compile and excution
To process the test-beam binary files for the BC results
```
./compile.sh
./emul_test-beam_Sep23.exe 1695829026 1695829027 1
```
The arguments from left to right are relay number, run number and link number, respectively.
Similarly for the STC4 results
```
./emul_test-beam_Sep23.exe 1695733045 1695733046 1 
```
