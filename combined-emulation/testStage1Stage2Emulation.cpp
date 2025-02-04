/**********************************************************************
 Created on : 29/01/2025
 Purpose    : Testing a combined stage1 and stage2 emulation
 Author     : Indranil Das, Research Associate, Imperial
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/

#include "TPGStage1Stage2Emulation.hh"

int main()
{
  
  const char* inputEMP = "EMPStage2Input.txt";
  TPGStage1Stage2Emulation::InputOutput stage12IO;
  stage12IO.SetStage1InputEMP(inputEMP);
  stage12IO.Stage1InputInfo();
  
  
  return true;
}
