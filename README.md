# mtpc-digi
Script to analyse g4sbs output and perform digitisation of hits in the mTPC. Produces text files of digitised hits.
At the moment only the most basic digi method is uploaded here. More advanced version with diffusion and electronics to be uploaded later.

Branches:
- Main branch is the most basic digitisation. Hits in mTPC gas volume are "drifted" to GEM readouts. The readout pad is recorded, and the time since the start of the event is recorded (which includes drift time). No diffusion or SAMPA shaping is in this version. At the moment the code is slow because it uses the IsInside function of a THPoly histogram to find the hit readout pad. In the future this will be upgraded to the newer quicker method from Root using cells of THPoly's. Due to the runtime, it is recommended to farm the code.

How to run:
- Example local batch farm scripts are in the Farm folder to show the stages. They will need to be modified for swif2 usage.
  -- split2.C is used to divide the g4sbs output into smaller root files. Change the g4sbs file name at line 4, and the desired output filenames at line 21
  -- CreateRunList.sh is used to create a runlist matching the g4sbs split files, The run list numbers should be the start event of each file.
  -- submitdigi.sh sends the jobs to the batch farm, it used the run list as a command line argument and it calls FarmDigi. FarmDigi then calls the root digitisation macro. In submittodigi one has to set the running envorinment for the digitisation script and the temporary batch directory for the output files. In FarmDigi one has to set the input and output file locations and the filenames
- DigiSimpleFarm.cpp is the main macro which runs the digitisation.
