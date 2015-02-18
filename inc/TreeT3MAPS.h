//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Tue Feb 17 17:08:44 2015 by ROOT version 5.34/05
// from TTree TreeT3MAPS/TreeT3MAPS
// found on file: T3MAPS_17_2_2015.root
//////////////////////////////////////////////////////////

#ifndef TreeT3MAPS_h
#define TreeT3MAPS_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include <vector>
using namespace std;
using std::vector;

// Fixed size dimensions of array or collections stored in the TTree if any.

class TreeT3MAPS {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

   // Declaration of leaf types
   Double_t        timestamp_start;
   Double_t        timestamp_stop;
   vector<int>     *hit_row;
   vector<int>     *hit_column;

   // List of branches
   TBranch        *b_timestamp_start;   //!
   TBranch        *b_timestamp_stop;   //!
   TBranch        *b_hit_row;   //!
   TBranch        *b_hit_column;   //!

   TreeT3MAPS(TTree *tree=0);
   virtual ~TreeT3MAPS();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef TreeT3MAPS_cxx
TreeT3MAPS::TreeT3MAPS(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("T3MAPS_17_2_2015.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("T3MAPS_17_2_2015.root");
      }
      f->GetObject("TreeT3MAPS",tree);

   }
   Init(tree);
}

TreeT3MAPS::~TreeT3MAPS()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t TreeT3MAPS::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t TreeT3MAPS::LoadTree(Long64_t entry)
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

void TreeT3MAPS::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set object pointer
   hit_row = 0;
   hit_column = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("timestamp_start", &timestamp_start, &b_timestamp_start);
   fChain->SetBranchAddress("timestamp_stop", &timestamp_stop, &b_timestamp_stop);
   fChain->SetBranchAddress("hit_row", &hit_row, &b_hit_row);
   fChain->SetBranchAddress("hit_column", &hit_column, &b_hit_column);
   Notify();
}

Bool_t TreeT3MAPS::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void TreeT3MAPS::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t TreeT3MAPS::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef TreeT3MAPS_cxx
