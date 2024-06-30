#!/bin/bash

CURRDIR="$PWD"
cd /tmp/$USER
git clone https://gitlab.cern.ch/pdauncey/hgcal10glinkreceiver.git
cd $CURRDIR
rsync -avP /tmp/$USER/hgcal10glinkreceiver/common .
ln -s /eos/cms/store/group/dpg_hgcal/tb_hgcal/2023/BeamTestSep/HgcalBeamtestSep2023 dat

