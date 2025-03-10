#include <TROOT.h>
#include <TGraph.h>
#include <iostream>
#include <fstream>
#include <TChainElement.h>
#include <TMath.h>
#include <TH2Poly.h>
#include <TF1.h>
#include <TFitResultPtr.h>
#include <TFitResult.h>
#include <TRandom3.h>
#include <TRandom.h>
#include <string>
#include <TString.h>
#include <TCanvas.h>
#include <vector>
#include <algorithm>
#include <TLorentzVector.h>
// #include <TIter.h>

// this header for g4sbs output will need changed if different g4sbs used
#include "g4sbs_tree_acqu.C"

#include "GetPadLoaded.cpp"
// #include "SampaShapingAndSample_ReturnVals.cpp"

#include <chrono>

using namespace std;

//==============================================================================
// FUNCTIONS
Double_t ZtoGEM(Double_t zpos);


//==============================================================================
// GLOBAL VARIABLES
const Int_t GEMGain = 1000;
const Int_t Nevents = 10000;

//==============================================================================
// START MAIN FUNCTION
// void DigiSimpleFarm(const char *infile = "/w/work0/home/rachel/HallA/TDIS/EventsFromSteve/Sim/g4sbsout_EPCEvents_0"){
void DigiSimpleFarm(Int_t EvStart){
// for local test need to know event start no of split up file
// input and output file names set below - check first if in farm or local mode
  
  //============================================================================
  // FLAGS AND VARIABLES
  Int_t EventCounter = 0;
  Int_t nevent = 0;
  TRandom3 RandomVar(0);

  // he:ch4 70:30 at 1.2kV/cm:
  Double_t DL = 0.015;//0.015;//0.03;//0.015;//sqrt(cm) up to 0.03sqrt(cm)
  Double_t DT = 0.015;
  Double_t MaxDriftTime = 1700.0;// convert 1.7e-6 to ns
  Double_t DriftVelocity = 32.1;//um/ns

  // he:ch4 70:30 at 3.4kV/cm for gem regions
  Double_t DL_GEM = 0.014468; //sqrt(cm)
  Double_t DT_GEM = 0.0263103; //sqrt(cm)
  Double_t DriftVelocity_GEM = 33.04; // um/ns


  // gem dimensions
  const Double_t GEMGap = 0.1; // take 1mm, units cm to match diffusion units
  // smear the transverse drift of each electron to get charge cloud area
  Double_t sigmatransverse_GEM = DT_GEM * TMath::Sqrt(GEMGap);
  // smear the longitudinal drift of each electron to get time window 
  Double_t sigmalongitudinal_GEM = DL_GEM * TMath::Sqrt(GEMGap); 

  //============================================================================
  
  // get histo of readout pads
  TFile *fPads =  TFile::Open("hPads2.root"); //hPads2 is steve's suggested pad phis
  TCanvas *c1 = (TCanvas*)fPads->Get("c1");
  TH2Poly *hReadoutPads = (TH2Poly*)c1->GetPrimitive("hPolyBins");
  // to draw the pads
  // TCanvas *cPads = new TCanvas();
  // cPads->cd();
  // hReadoutPads->Draw();
  fPads->Close();
  // get bin list
  TList *bin_list=hReadoutPads->GetBins();
  if( !bin_list ){
    cout << "Problem in getting bin list from hReadoutPads, exitting." << endl;
    exit (EXIT_FAILURE);
  }
  TIter next(bin_list);
  TObject *obj;
  TH2PolyBin *bin;
  double area;
  vector<Double_t> vbinareas;
  while((obj=next())){
    bin = (TH2PolyBin*) obj;
    vbinareas.push_back(bin->GetArea());
  }

  //  READ IN SIM FILE
  TChain *C = new TChain("T");

  // below for farming
  TString simfile = "/scratch/rachel/tdis/";
  simfile += EvStart;
  simfile += "/g4sbsout_EPCEvents_";
  simfile += EvStart;
  simfile += ".root";

  // for local check
  // TString simfile = "/w/work0/home/rachel/TDIS/EPC/Oct2024/g4sbsout/g4sbsout_EPCEvents_4350000.root";


  C->Add(simfile);
  
  // ASSIGN TREE
  g4sbs_tree_acqu *T = new g4sbs_tree_acqu(C);

  Int_t NEventsChain = C->GetEntries();
  cout << "N events in chain is " << NEventsChain << endl << endl;
  
  // Text file for hit outputs
  // for farming
  TString txtfile = "/scratch/rachel/tdis/";
  txtfile += EvStart;
  txtfile += "/g4sbsout_EPCEvents_";
  txtfile += EvStart;
  txtfile += ".txt";

  // for local check
  // TString txtfile = "testing.txt";


  ofstream TxtFileOut;
  TxtFileOut.open (txtfile);

  
  //============================================================================
  // EVENT LOOP
  
  // Variables outside loop
  //------------------------------------------------------------ start event loop
  for(Int_t i=0; i<NEventsChain; i++){   
  // for(Int_t i=0; i<Nevents; i++){   
  // for(Int_t i=0; i<1; i++){   

    T->GetEntry(i);  

    // cout << "i " << i << endl;
    if (EventCounter % 1000 == 0) cout << EventCounter << "/" << NEventsChain << endl;
    
    Int_t evid = EvStart+i;
    TxtFileOut << "Event " << right << evid << "\n";
    // TxtFileOut << "Event " << right << i << "\n";
    
    // for acqu
    TVector3 vprimary(T->AcquMCPrimaries_Px, T->AcquMCPrimaries_Py, T->AcquMCPrimaries_Pz);
    Double_t thetaprimary = vprimary.Theta()*TMath::RadToDeg();
    Double_t phiprimary = vprimary.Phi()*TMath::RadToDeg();
    TxtFileOut << "\t" << right << T->AcquMCPrimaries_Pt
	       << "\t" << right << thetaprimary
	       << "\t" << right << phiprimary
	       << "\t" << right << T->AcquMCPrimaries_Vz
	       << "\n";
    
    // mTPC hits
    Int_t mTPC_nHitsPerEvent = T->SBS_mTPC_hit_nhits;
    
    Double_t energydepositedbyhit;
    Double_t hitx;
    Double_t hity;
    Double_t hitz;
    Double_t hitr;
    // cout << "new event" << endl;
    
    for(int hitinc = 0; hitinc<T->SBS_mTPC_hit_nhits; hitinc++){
      if( (*(T->SBS_mTPC_hit_pid))[hitinc]!=2212 || 
	  (*(T->SBS_mTPC_hit_mid))[hitinc]!=0)continue;
      
      energydepositedbyhit = (*(T->SBS_mTPC_hit_edep))[hitinc];
      // hitx =  (*(T->SBS_mTPC_hit_xhit))[hitinc];
      // hity =  (*(T->SBS_mTPC_hit_yhit))[hitinc];
      // hitz =  (*(T->SBS_mTPC_hit_zhit))[hitinc];
      hitx =  (*(T->SBS_mTPC_hit_xhitg))[hitinc];
      hity =  (*(T->SBS_mTPC_hit_yhitg))[hitinc];
      hitz =  (*(T->SBS_mTPC_hit_zhitg))[hitinc];
      hitr = TMath::Sqrt(hitx*hitx + hity*hity);
      
      Double_t HitTime = (*(T->SBS_mTPC_hit_tavg))[hitinc];
      
      // ztogem must be in cm since diff coeffs in cm
      Double_t ztogem =  ZtoGEM(hitz*1000.0); // convert argument, from g4, to mm for fncn, return is mm
      ztogem = ztogem/10.0; // convert to cm
      
      Double_t driftime = (ztogem*10000.0)/DriftVelocity;// convert z to gem from cm to um, result will be ns

      // get pad loaded script deals in mm
      // g4 out is in m, smearing is cm, so convert cm to mm in end
      vector<Int_t> vhitpad;
      vhitpad = GetPadLoaded(hitx*1000.0,
			     hity*1000.0,
			     hitz*1000.0,
			     hReadoutPads);
      
      Double_t TotalTime = HitTime+driftime;
      
      TxtFileOut << "\t" << right << TotalTime << "\t"
		 << right << energydepositedbyhit  << "\t"
		 << right << hitx << "\t"
		 << right << hity << "\t"
		 << right << hitz << "\t"
		 << right << vhitpad[1] << "\t"
		 << right << vhitpad[2] << "\t"
		 << right << vhitpad[3] << "\t"
		 << right << ztogem/100.0 << "\t"
		 << endl; // convert z to gem in m
      
      // clear vectors
      vhitpad.clear();
      
    }// hit loop

    EventCounter++;
  } //------------------------------------------------------------ end event loop
  
  cout << endl;
  
  //============================================================================
  
  
  // CLOSE FILES
  TxtFileOut.close();
  
  
} //------------------------------------------------------------ end main function


// get z to nearest gem plane
Double_t ZtoGEM(Double_t zpos){ // it is input in mm, m from g4 multiplied by 1000
  
  Double_t DistanceToGEM = -999.0;
  if(zpos>=-250.0 && zpos<=-200.0)
    DistanceToGEM = fabs(zpos - (-250.0));
  if(zpos>-200.0 && zpos<=-150.0)
    DistanceToGEM = fabs(zpos - (-150.0));
  if(zpos>-150.0 && zpos<=-100.0)
    DistanceToGEM = fabs(zpos - (-150.0));
  if(zpos>-100.0 && zpos<=-50.0)
    DistanceToGEM = fabs(zpos - (-50.0));
  if(zpos>-50.0 && zpos<=0.0)
    DistanceToGEM = fabs(zpos - (-50.0));
  if(zpos>0.0 && zpos<=50.0)
    DistanceToGEM = fabs(zpos - (50.0));
  if(zpos>50.0 && zpos<=100.0)
    DistanceToGEM = fabs(zpos - (50.0));
  if(zpos>100.0 && zpos<=150.0)
    DistanceToGEM = fabs(zpos - (150.0));
  if(zpos>150.0 && zpos<=200.0)
    DistanceToGEM = fabs(zpos - (150.0));
  if(zpos>200.0 && zpos<=250.0)
    DistanceToGEM = fabs(zpos - (250.0));
  
  return DistanceToGEM; // it is in mm
}

// incCell 0 Z of readout is -0.24987;
// incCell 1 Z of readout is -0.15013;
// incCell 2 Z of readout is -0.14987;
// incCell 3 Z of readout is -0.0501305;
// incCell 4 Z of readout is -0.0498695;
// incCell 5 Z of readout is 0.0498695;
// incCell 6 Z of readout is 0.0501305;
// incCell 7 Z of readout is 0.14987;
// incCell 8 Z of readout is 0.15013;
// incCell 9 Z of readout is 0.24987;
