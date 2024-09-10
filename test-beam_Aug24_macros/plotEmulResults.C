/**********************************************************************
 Created on : 03/09/2024
 Purpose    : Plotting emulation results
 Author     : Indranil Das, Visiting Fellow
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/
#include "TH1D.h"
#include "TH2D.h"
#include "TMath.h"
#include "TProfile.h"
#include "TSystem.h"
#include "TCanvas.h"
#include "TFile.h"

#include <sstream>

int plotEmulResults(uint32_t relayNumber = 1723627575)
{
  
  
  TFile *fin = TFile::Open(Form("Diff_Relay-%u.root",relayNumber));

  TH1D *hElDiff[2][2][3]; //2 modes(tctp==any or 1), 2 lpGBTs, 3 modules in each lpGBT
  TH1D *hUnpkWordDiff[2][2][3][8]; //2 modes(tctp==any or 1), 2 lpGBTs, 3 modules in each lpGBT 8 words max
  TH1D *hTCEDiff[2][2][3][8]; //2 modes(tctp==any or 1), 2 lpGBTs, 3 modules in each lpGBT, 8 TC energies 
  TH1D *hTCADiff[2][2][3][8]; //2 modes(tctp==any or 1), 2 lpGBTs, 3 modules in each lpGBT, 8 TC addresses

  TH1D *hElinkDiff = (TH1D *)fin->Get("diff_plots/hElinkDiff");
  TH1D *hElinkDiff_0 = (TH1D *)fin->Get("diff_plots/hElinkDiff_0");
  TH1D *hElinkDiff_1 = (TH1D *)fin->Get("diff_plots/hElinkDiff_1");
  TH1D *hUWord = (TH1D *)fin->Get("diff_plots/hUWord");
  TH1D *hUWord_0 = (TH1D *)fin->Get("diff_plots/hUWord_0");
  TH1D *hUWord_1 = (TH1D *)fin->Get("diff_plots/hUWord_1");
 
  for(int imode=0;imode<2;imode++){
    for(int ilp=0;ilp<2;ilp++){
      for(int imdl=0;imdl<3;imdl++){
	hElDiff[imode][ilp][imdl] = (TH1D *)fin->Get(Form("diff_plots/hElDiff_%d_%d_%d",imode,ilp,imdl));
	for(int itc=0;itc<8;itc++){
	  hTCEDiff[imode][ilp][imdl][itc] = (TH1D *)fin->Get(Form("diff_plots/hTCEDiff_%d_%d_%d_%d",imode,ilp,imdl,itc));
	  hTCADiff[imode][ilp][imdl][itc] = (TH1D *)fin->Get(Form("diff_plots/hTCADiff_%d_%d_%d_%d",imode,ilp,imdl,itc));
	  hUnpkWordDiff[imode][ilp][imdl][itc] = (TH1D *)fin->Get(Form("diff_plots/hUnpkWordDiff_%d_%d_%d_%d",imode,ilp,imdl,itc));
	}//itc
      }//module
    }//ilp
  }//mode

  TCanvas *c1 = new TCanvas("c1","c1",800,600);
  c1->SetLogy();
  hElinkDiff->Draw();
  c1->SaveAs(Form("~/temp/figs/%u_01_ElinkDiff.png",relayNumber));
  c1->SaveAs(Form("~/temp/figs/%u_01_ElinkDiff.pdf",relayNumber));

  TCanvas *c2 = new TCanvas("c2","c2",1600,600);
  c2->Divide(2,1);
  c2->cd(1)->SetLogy();
  hElinkDiff_0->Draw();
  c2->cd(2)->SetLogy();
  hElinkDiff_1->Draw();
  c2->SaveAs(Form("~/temp/figs/%u_02_ElinkDiff_tctp.png",relayNumber));
  c2->SaveAs(Form("~/temp/figs/%u_02_ElinkDiff_tctp.pdf",relayNumber));

  TCanvas *c3 = new TCanvas("c3","c3",800,600);
  c3->SetLogy();
  hUWord->Draw();
  c3->SaveAs(Form("~/temp/figs/%u_03_UWordDiff.png",relayNumber));
  c3->SaveAs(Form("~/temp/figs/%u_03_UWordDiff.pdf",relayNumber));

  TCanvas *c4 = new TCanvas("c4","c4",1600,600);
  c4->Divide(2,1);
  c4->cd(1)->SetLogy();
  hUWord_0->Draw();
  c4->cd(2)->SetLogy();
  hUWord_1->Draw();
  c4->SaveAs(Form("~/temp/figs/%u_04_UWordDiff_tctp.png",relayNumber));
  c4->SaveAs(Form("~/temp/figs/%u_04_UWordDiff_tctp.pdf",relayNumber));

  int itctp = 0;
  TCanvas *c5 = new TCanvas("c5","c5",1600,600);
  c5->Divide(3,2);
  int ipad = 1;
  for(int ilp=0;ilp<2;ilp++){
    for(int imdl=0;imdl<3;imdl++){
      c5->cd(ipad++)->SetLogy();
      hElDiff[itctp][ilp][imdl]->Draw();
    }
  }
  c5->SaveAs(Form("~/temp/figs/%u_05_ElinkDiff_tctp-%d.png",relayNumber,itctp));
  c5->SaveAs(Form("~/temp/figs/%u_05_ElinkDiff_tctp-%d.pdf",relayNumber,itctp));

  itctp = 1;
  TCanvas *c6 = new TCanvas("c6","c6",1600,600);
  c6->Divide(3,2);
  ipad = 1;
  for(int ilp=0;ilp<2;ilp++){
    for(int imdl=0;imdl<3;imdl++){
      c6->cd(ipad++)->SetLogy();
      hElDiff[itctp][ilp][imdl]->Draw();
    }
  }
  c6->SaveAs(Form("~/temp/figs/%u_06_ElinkDiff_tctp-%d.png",relayNumber,itctp));
  c6->SaveAs(Form("~/temp/figs/%u_06_ElinkDiff_tctp-%d.pdf",relayNumber,itctp));

  for(int itctp = 0; itctp < 2 ; itctp++){
    for(int ilp=0;ilp<2;ilp++){
      for(int imdl=0;imdl<3;imdl++){
	std::string modtype = "";
	switch(imdl){
	case 0:
	  modtype = "BestC";
	  break;
	case 1:
	  modtype = "STC4A";
	  break;
	case 2:
	  modtype = (relayNumber>1722881092) ? "STC16" : "CTC4A";
	  break;
	default:
	  break;
	}

	///////Enegy////////
	std::string mname = Form("%u_07_Energy_tctp-%d_lpGBT-%d_%s",relayNumber,itctp,ilp,modtype.c_str());
	TCanvas *c = new TCanvas(Form("canvas_%s",mname.c_str()),Form("canvas_%s",mname.c_str()),1800,800);
	c->Divide(4,2);
	for(int itc=0;itc<8;itc++){
	  c->cd(itc+1)->SetLogy();
	  hTCEDiff[itctp][ilp][imdl][itc]->Draw();
	}//itc
	c->SaveAs(Form("~/temp/figs/%s.png",mname.c_str()));
	c->SaveAs(Form("~/temp/figs/%s.pdf",mname.c_str()));

	/////////Address///////////
	mname = Form("%u_08_Address_tctp-%d_lpGBT-%d_%s",relayNumber,itctp,ilp,modtype.c_str());
	c = new TCanvas(Form("canvas_%s",mname.c_str()),Form("canvas_%s",mname.c_str()),1800,800);
	c->Divide(4,2);
	for(int itc=0;itc<8;itc++){
	  c->cd(itc+1)->SetLogy();
	  hTCADiff[itctp][ilp][imdl][itc]->Draw();
	}//itc
	c->SaveAs(Form("~/temp/figs/%s.png",mname.c_str()));
	c->SaveAs(Form("~/temp/figs/%s.pdf",mname.c_str()));

	////////Words////////////
	mname = Form("%u_09_UnpkStage1_tctp-%d_lpGBT-%d_%s",relayNumber,itctp,ilp,modtype.c_str());
	c = new TCanvas(Form("canvas_%s",mname.c_str()),Form("canvas_%s",mname.c_str()),1800,800);
	c->Divide(4,2);
	for(int itc=0;itc<8;itc++){
	  c->cd(itc+1)->SetLogy();
	  hUnpkWordDiff[itctp][ilp][imdl][itc]->Draw();
	}//itc
	c->SaveAs(Form("~/temp/figs/%s.png",mname.c_str()));
	c->SaveAs(Form("~/temp/figs/%s.pdf",mname.c_str()));
      
      }//module
    }//ilp
  }//itctp
  
  fin->Close();
  delete fin;
  
  return true;
}
