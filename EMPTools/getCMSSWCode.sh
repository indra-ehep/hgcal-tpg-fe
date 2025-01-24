#!/bin/bash
mkdir -p CMSSWCode
cd CMSSWCode
git init

BRANCH=hgc-tpg-devel-CMSSW_14_0_0_pre1

git remote add -t $BRANCH hgc-tpg git@github.com:hgc-tpg/cmssw.git

git config core.sparseCheckout true
export BASE_URL=L1Trigger/DemonstratorTools
for f in utilities Frame FileFormat BoardData; do
    echo $BASE_URL/interface/$f.h >> .git/info/sparse-checkout
    echo $BASE_URL/src/$f.cc >> .git/info/sparse-checkout
done
git fetch hgc-tpg $BRANCH
git checkout -b $BRANCH FETCH_HEAD
cd -