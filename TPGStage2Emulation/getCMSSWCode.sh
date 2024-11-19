#!/bin/bash
cd CMSSWCode
git init
git remote add -t S2Emu_standalone_Nov24 my-cmssw git@github.com:EmyrClement/cmssw.git
git config core.sparseCheckout true
echo L1Trigger/L1THGCal/interface/backend_emulator >> .git/info/sparse-checkout
echo L1Trigger/L1THGCal/src/backend_emulator >> .git/info/sparse-checkout
echo DataFormats/L1TParticleFlow/interface >> .git/info/sparse-checkout
echo DataFormats/L1THGCal/interface >> .git/info/sparse-checkout
git pull my-cmssw S2Emu_standalone_Nov24
git checkout -b S2Emu_standalone_Nov24
cd -