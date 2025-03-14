void split2(){

  // g4sbs output file
  TString file = "/w/work0/home/rachel/g4sbsout_mtpcsimepc_10000000forg4_jan262022_tree_10M_1p5T.root";

  TFile old(file);
  TTree *oldtree;
  old.GetObject("T",oldtree);

  const auto nentries = oldtree->GetEntries();

  Int_t nfiles = 1000;
  Int_t ev = 10000;//50000;
  Int_t evstart=0;
  Int_t evend=0;

  for(Int_t inc=0; inc<nfiles; inc++){
    evstart = inc*ev;
    evend = evstart+ev;
    // output files
    TString newfilename = "/w/work0/home/rachel//g4sbsout_EPCEvents_";
    newfilename += evstart;
    newfilename += ".root";
    TFile newfile(newfilename,"RECREATE");
    auto newtree = oldtree->CloneTree(0);
    
    for(Int_t i=evstart; i<evend; i++){
      oldtree->GetEntry(i);
      newtree->Fill();
    }
    
    newtree->Write(0,TObject::kOverwrite);
    delete newtree;
    
  }
  
}
