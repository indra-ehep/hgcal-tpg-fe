#!/bin/bash

CURRDIR="$PWD"

if [ ! -d /tmp/$USER/hgcal10glinkreceiver ] ; then
    cd /tmp/$USER
    git clone https://gitlab.cern.ch/pdauncey/hgcal10glinkreceiver.git
else
    cd /tmp/$USER/hgcal10glinkreceiver
    git pull
fi

# if [ ! -d /tmp/$USER/slow_control_configuration ] ; then
#     cd /tmp/$USER
#     git clone https://gitlab.cern.ch/hgcal-daq-sw/serenity-daq/slow_control_configuration.git
# else
#     cd /tmp/$USER/slow_control_configuration
#     git pull
# fi


cd $CURRDIR
rsync -avP /tmp/$USER/hgcal10glinkreceiver/common .
#rsync -avP /tmp/$USER/slow_control_configuration cfgmap/

if [ ! -L dat ] ; then
    ln -s /eos/cms/store/group/dpg_hgcal/tb_hgcal/2024/BeamTestAug/HgcalBeamtestAug2024 dat
fi
