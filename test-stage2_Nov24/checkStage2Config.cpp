/**********************************************************************
 Created on : 04/02/2025
 Purpose    : Check stage2 configuration
 Author     : Indranil Das, Research Associate, Imperial
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/
#include <iostream>
#include "TPGStage2Configuration.hh"
#include <string>

int main()
{
  std::string config = "input/stage2/configuration/Stage2Configuration.yaml" ;
  TPGStage2Configuration::Stage2Board sb;
  sb.readConfigYaml(config.c_str());
  sb.print();
  return true;
}
