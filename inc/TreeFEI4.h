//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Tue Feb 17 17:55:12 2015 by ROOT version 5.34/05
// from TTree Table/Converted HDF5 table
// found on file: FEI4_17_2_2015.root
//////////////////////////////////////////////////////////

#ifndef TreeFEI4_h
#define TreeFEI4_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include <vector>
using namespace std;
using std::vector;

// Fixed size dimensions of array or collections stored in the TTree if any.

class TreeFEI4 {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

   // Declaration of leaf types
   Long64_t        event_number;
   UInt_t          trigger_number;
   UChar_t         relative_BCID;
   UShort_t        LVL1ID;
   UChar_t         column;
   UShort_t        row;
   UChar_t         tot;
   UShort_t        BCID;
   UShort_t        TDC;
   UChar_t         TDC_time_stamp;
   UChar_t         trigger_status;
   UInt_t          service_record;
   UShort_t        event_status;
   Double_t        timestamp_start;
   Double_t        timestamp_stop;

   // List of branches
   TBranch        *b_event_number;   //!
   TBranch        *b_trigger_number;   //!
   TBranch        *b_relative_BCID;   //!
   TBranch        *b_LVL1ID;   //!
   TBranch        *b_column;   //!
   TBranch        *b_row;   //!
   TBranch        *b_tot;   //!
   TBranch        *b_BCID;   //!
   TBranch        *b_TDC;   //!
   TBranch        *b_TDC_time_stamp;   //!
   TBranch        *b_trigger_status;   //!
   TBranch        *b_service_record;   //!
   TBranch        *b_event_status;   //!
   TBranch        *b_timestamp_start;   //!
   TBranch        *b_timestamp_stop;   //!

   TreeFEI4(TTree *tree=0);
   virtual ~TreeFEI4();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef TreeFEI4_cxx
TreeFEI4::TreeFEI4(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("FEI4_17_2_2015.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("FEI4_17_2_2015.root");
      }
      f->GetObject("Table",tree);

   }
   Init(tree);
}

TreeFEI4::~TreeFEI4()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t TreeFEI4::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t TreeFEI4::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void TreeFEI4::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("event_number", &event_number, &b_event_number);
   fChain->SetBranchAddress("trigger_number", &trigger_number, &b_trigger_number);
   fChain->SetBranchAddress("relative_BCID", &relative_BCID, &b_relative_BCID);
   fChain->SetBranchAddress("LVL1ID", &LVL1ID, &b_LVL1ID);
   fChain->SetBranchAddress("column", &column, &b_column);
   fChain->SetBranchAddress("row", &row, &b_row);
   fChain->SetBranchAddress("tot", &tot, &b_tot);
   fChain->SetBranchAddress("BCID", &BCID, &b_BCID);
   fChain->SetBranchAddress("TDC", &TDC, &b_TDC);
   fChain->SetBranchAddress("TDC_time_stamp", &TDC_time_stamp, &b_TDC_time_stamp);
   fChain->SetBranchAddress("trigger_status", &trigger_status, &b_trigger_status);
   fChain->SetBranchAddress("service_record", &service_record, &b_service_record);
   fChain->SetBranchAddress("event_status", &event_status, &b_event_status);
   fChain->SetBranchAddress("timestamp_start", &timestamp_start, &b_timestamp_start);
   fChain->SetBranchAddress("timestamp_stop", &timestamp_stop, &b_timestamp_stop);
   Notify();
}

Bool_t TreeFEI4::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void TreeFEI4::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t TreeFEI4::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef TreeFEI4_cxx
