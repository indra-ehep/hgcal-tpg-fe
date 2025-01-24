#!/bin/bash
mkdir -p CMSSWCode
cd CMSSWCode
git init

BRANCH=S2Emu_standalone_Nov24
REMOTE=emyr-cmssw
echo "Fetching $BRANCH from $REMOTE"
git remote add -t $BRANCH $REMOTE git@github.com:EmyrClement/cmssw.git

git config core.sparseCheckout true

# Stage 2 emulation
echo L1Trigger/L1THGCal/interface/backend_emulator >> .git/info/sparse-checkout
echo L1Trigger/L1THGCal/src/backend_emulator >> .git/info/sparse-checkout
echo DataFormats/L1TParticleFlow/interface >> .git/info/sparse-checkout
echo DataFormats/L1THGCal/interface >> .git/info/sparse-checkout

# EMP Tools
export BASE_URL=L1Trigger/DemonstratorTools
for f in utilities Frame FileFormat BoardData; do
    echo $BASE_URL/interface/$f.h >> .git/info/sparse-checkout
    echo $BASE_URL/src/$f.cc >> .git/info/sparse-checkout
done
git fetch $REMOTE $BRANCH
git checkout -b $BRANCH FETCH_HEAD
cd -
