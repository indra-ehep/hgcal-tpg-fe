/**********************************************************************
 Created on : 02/07/2025
 Purpose    : Calculate background rate from CMMSW ntuples
 Author     : Indranil Das, Research Associate, Imperial
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/

int calcBkgRateTuple()
{
  
  const char *server = "root://eosuser.cern.ch/";
  // const char *infile = "/eos/cms/store/group/dpg_hgcal/comm_hgcal/TPG/stage2_emulator_ntuples_semiemulator_clusterProperties/minbias_PU200_2kFiles/ntuple_0.root" ;
  // std::unique_ptr<TFile> fin(TFile::Open(Form("%s/%s",server,infile)));
  // std::unique_ptr<TTree> tr((TTree*)fin->Get("l1tHGCalTriggerNtuplizer/HGCalTriggerNtuple"));
  //std::unique_ptr<TTree> tr((TTree*)fin->Get("l1tHGCalTriggerNtuplizer/HGCalTriggerNtuple"));

  TChain *tr = new TChain("l1tHGCalTriggerNtuplizer/HGCalTriggerNtuple");
  //const char *inputfname = "/tmp/remote_fs.txt";
  const char *inputfname = "/tmp/local_fs.txt";
  std::string s;
  ifstream fin(inputfname);
  int nfiles = 0;
  while(getline(fin,s)){
    std::cout << "Filename : " << s << std::endl;
    //tr->Add(Form("%s/%s",server,s.c_str()));
    tr->Add(Form("%s",s.c_str()));
    nfiles++;
  }
  std::cout << "Nof added files : " << nfiles << std::endl;
  
  tr->SetBranchStatus("*",0);  
  // Int_t event ;
  // tr->SetBranchStatus("event",1);
  // tr->SetBranchAddress("event"	, &event);

  std::vector<float>  *clusE_a16 = 0 ;
  tr->SetBranchStatus("cl3d_p16Tri_pt",1);
  tr->SetBranchAddress("cl3d_p16Tri_pt" , &clusE_a16);

  std::vector<float>  *clusE_a30 = 0 ;
  tr->SetBranchStatus("cl3d_p03Tri_pt",1);
  tr->SetBranchAddress("cl3d_p03Tri_pt" , &clusE_a30);

  std::vector<float>  *clusE_a45 = 0 ;
  tr->SetBranchStatus("cl3d_p045Tri_pt",1);
  tr->SetBranchAddress("cl3d_p045Tri_pt" , &clusE_a45);

  std::cout << "Total number of Events: " << tr->GetEntries() << std::endl;
  Long64_t nofEvents = 0;
  if(nofEvents==0) nofEvents = tr->GetEntries();
  std::map<Long64_t,double> maxptmap16, maxptmap30, maxptmap45;
  for (Long64_t ievent = 0 ; ievent < nofEvents ; ievent++ ) {
    tr->GetEntry(ievent) ;

    double maxpt = 0;
    for(int iclus = 0; iclus < clusE_a16->size() ; iclus++){
      if(clusE_a16->at(iclus)>maxpt) maxpt = clusE_a16->at(iclus);
    }
    maxptmap16[ievent] = maxpt;

    maxpt = 0;
    for(int iclus = 0; iclus < clusE_a30->size() ; iclus++){
      if(clusE_a30->at(iclus)>maxpt) maxpt = clusE_a30->at(iclus);
    }
    maxptmap30[ievent] = maxpt;

    maxpt = 0;
    for(int iclus = 0; iclus < clusE_a45->size() ; iclus++){
      if(clusE_a45->at(iclus)>maxpt) maxpt = clusE_a45->at(iclus);
    }
    maxptmap45[ievent] = maxpt;

    if(ievent%10000==0) std::cout<<"Event : "<< ievent <<", nof Clusters16 : "<< clusE_a16->size() <<", nof Clusters30 : "<< clusE_a30->size() <<", nof Clusters45 : "<< clusE_a45->size() << ", maxpt: " << maxpt << std::endl;
    
    clusE_a16->clear();
    clusE_a30->clear();
    clusE_a45->clear();
  }
  
  TGraph *gr16 = new TGraph(200);
  TGraph *gr30 = new TGraph(200);
  TGraph *gr45 = new TGraph(200);
  int ipoint = 0;
  for(int ithr = 20 ; ithr <= 200 ; ithr = ithr+6){
    int pass16 = 0, pass30 = 0, pass45 = 0;
    for (Long64_t ievent = 0 ; ievent < nofEvents ; ievent++ ) {
      if(maxptmap16[ievent]>double(ithr)) pass16++;
      if(maxptmap30[ievent]>double(ithr)) pass30++;
      if(maxptmap45[ievent]>double(ithr)) pass45++;
    }
    double rate16 = double(pass16) * 40.e3 * 2760. / 3564. / double(nofEvents) ;
    double rate30 = double(pass30) * 40.e3 * 2760. / 3564. / double(nofEvents) ;
    double rate45 = double(pass45) * 40.e3 * 2760. / 3564. / double(nofEvents) ;
    std::cout <<"Threshold :" << ithr << ", Rate16 : " << rate16 << ", Rate30 : " << rate30 << ", Rate45 : " << rate45 << std::endl;
    gr16->SetPoint(ipoint,double(ithr),rate16);
    gr30->SetPoint(ipoint,double(ithr),rate30);
    gr45->SetPoint(ipoint,double(ithr),rate45);
    ipoint++;
  }//threshold loop
  gr16->Set(ipoint);
  gr30->Set(ipoint);
  gr45->Set(ipoint);

  gr16->SetTitle("");
  gr30->SetTitle("");
  gr45->SetTitle("");

  gr16->SetFillColor(kBlack);
  gr30->SetFillColor(kBlue);
  gr45->SetFillColor(kRed);

  gr16->SetLineColor(kBlack);
  gr30->SetLineColor(kBlue);
  gr45->SetLineColor(kRed);

  gr16->SetLineWidth(3);
  gr30->SetLineWidth(3);
  gr45->SetLineWidth(3);

  gr16->SetMinimum(1);
  gr16->SetMaximum(5.e4);
  gr16->GetYaxis()->SetTitle("Single jet rate [kHz]");
  gr16->GetXaxis()->SetTitle("Threshold [GeV]");
  gr16->GetXaxis()->SetRangeUser(0,200);
  
  TLatex *texl = new TLatex(3.369606,58503.36,"CMS");
  texl->SetTextSize(0.035);
  TLatex *texp = new TLatex(26.9,59533.27,"Preliminary");
  texp->SetTextSize(0.025);
  texp->SetTextFont(52);
  auto leg0 = new TLegend(0.19,0.25,0.58,0.42);
  leg0->SetTextSize(0.028);
  leg0->SetHeader(Form("Minbias, PU 200"));
  leg0->AddEntry(gr16,"a = 0.016","lp");
  leg0->AddEntry(gr30,"a = 0.030","lp");
  leg0->AddEntry(gr45,"a = 0.045","lp");
  leg0->SetShadowColor(kWhite);
  
  TCanvas *c1 = new TCanvas("c1","c1",900,900);
  c1->SetLogy();
  c1->SetGridx();
  c1->SetGridy();
  c1->SetTickx();
  c1->SetTicky();
  gr16->Draw("a3c");
  gr30->Draw("3c same");
  gr45->Draw("3c same");
  leg0->Draw();
  texl->Draw("same");
  texp->Draw("same");

  TFile *fout = new TFile("output.root","recreate");
  gr16->Write();
  gr30->Write();
  gr45->Write();
  c1->Write();
  fout->Close();
  
  maxptmap16.clear();
  maxptmap30.clear();
  maxptmap45.clear();
  delete tr;
  delete fout;
  
  return true;
}
