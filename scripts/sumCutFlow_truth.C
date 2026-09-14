// sumCutFlow_truth.C
#include "TSystem.h"
#include "TFile.h"
#include "TH1.h"
#include "TTree.h"
#include "TMath.h"
#include "TString.h"
#include "iomanip"
#include <iostream>

Double_t totalEntries_check = 0.0;
Double_t totalSumW_check    = 0.0;

void verifyCutFlow(TFile *f, Double_t &cutEntries, Double_t &cutSumW) {
  TTree *tree = nullptr;
  f->GetObject("HWWTree_ee", tree);  // Main ntuple
  if (!tree) {
    std::cerr << "No TTree found" << std::endl;
    return;
  }

  Long64_t treeEntries = tree->GetEntries();

  Double_t mcWeight = 0.0;
  Double_t directSumW = 0.0;
  tree->SetBranchAddress("mcEventWeight", &mcWeight);

  Long64_t nentries = tree->GetEntries();
  for (Long64_t i = 0; i < nentries; ++i) {
    tree->GetEntry(i);
    directSumW += mcWeight;
    // Progress every 10k events
    if ((i+1) % 10000 == 0) {
        std::cout << "\r" << (i+1) << "/" << nentries 
                  << " (" << 100.*(i+1)/nentries << "%)" << std::flush;
    }
  }

  std::cout << "\nTree entries:     " << treeEntries << std::endl;
  std::cout << "Direct sum w:     " << directSumW << std::endl;
  std::cout << "CutFlow entries:  " << cutEntries << std::endl;
  std::cout << "CutFlow sum w:    " << cutSumW << std::endl;

  bool entriesOK  = TMath::Abs(cutEntries  - (Double_t)treeEntries) < 1;
  bool sumwOK     = TMath::Abs(cutSumW     - directSumW) < 1;

  if (entriesOK && sumwOK) {
    std::cout << "✅ BOTH CutFlow bins MATCH tree!" << std::endl;
  } else {
    std::cout << "❌ MISMATCH detected!" << std::endl;
    if (!entriesOK) std::cout << "   Entries: " << cutEntries << " vs " << treeEntries << std::endl;
    if (!sumwOK)    std::cout << "   SumW:    " << cutSumW << " vs " << directSumW << std::endl;
  }

  totalEntries_check += treeEntries;
  totalSumW_check += directSumW;
}

void sumCutFlow_truth(const char *dir = ".", const char *pattern = "user.dcamarer.")
{
  std::cout << std::fixed << std::setprecision(2);

  TSystemDirectory d(dir, dir);
  TList *files = d.GetListOfFiles();
  if (!files) {
    std::cerr << "No files found in directory: " << dir << std::endl;
    return;
  }

  TString pat(pattern);
  TIter next(files);
  TSystemFile *file;
  Double_t total_1 = 0.0;
  Double_t total_2 = 0.0;
  Double_t total_3 = 0.0;
  Double_t total_4 = 0.0;
  Double_t total_5 = 0.0;
  Double_t total_6 = 0.0;
  Double_t totalEntries = 0.0;
  Double_t totalSumW    = 0.0;
  Int_t nfiles = 0;

  while ((file = (TSystemFile*)next())) {
    TString fname(file->GetName());
    if (!file->IsDirectory() && fname.EndsWith(".root") && fname.Contains(pat)) {
      TString fullpath = TString::Format("%s/%s", dir, fname.Data());
      TFile *f = TFile::Open(fullpath, "READ");
      if (!f || f->IsZombie()) {
        std::cerr << "Problem opening file: " << fullpath << std::endl;
        delete f;
        continue;
      }

      TH1 *CutFlow = nullptr;
      f->GetObject("CutFlow", CutFlow);
      if (!CutFlow) {
        std::cerr << "Histogram CutFlow not found in " << fullpath << std::endl;
        f->Close();
        delete f;
        continue;
      }

      Double_t allevents = CutFlow->GetBinContent(1);
      Double_t grl = CutFlow->GetBinContent(2);
      Double_t corrupted = CutFlow->GetBinContent(3);
      Double_t pv = CutFlow->GetBinContent(4);
      Double_t twoleptons = CutFlow->GetBinContent(5);
      Double_t goodpair = CutFlow->GetBinContent(6);
      //
      Double_t entries = CutFlow->GetBinContent(40);
      Double_t sumw    = CutFlow->GetBinContent(41);

      total_1 += allevents;
      total_2 += grl;
      total_3 += corrupted;
      total_4 += pv;
      total_5 += twoleptons;
      total_6 += goodpair;
      totalEntries += entries;
      totalSumW    += sumw;
      nfiles++;

      std::cout << "Added " << fname << ": entries=" << entries << ", sumw=" << sumw << std::endl;

      verifyCutFlow(f, entries, sumw);
      std::cout << std::endl;

      f->Close();
      delete f;
    }
  }

  std::cout << "\nSummary over " << nfiles << " files:" << std::endl;
  std::cout << "* All events (bin 1)       = " << total_1 << std::endl;
  std::cout << "* GRL cut (bin 2)          = " << total_2 << std::endl;
  std::cout << "* Bad/corrupted (bin 3)    = " << total_3 << std::endl;
  std::cout << "* PV (bin 4)               = " << total_4 << std::endl;
  std::cout << "* >=2 good leptons (bin 5) = " << total_5 << std::endl;
  std::cout << "* Good pair (bin 6)        = " << total_6 << std::endl;
  std::cout << "* Total entries (bin 40)   = " << totalEntries << std::endl;
  std::cout << "* Total sumW    (bin 41)   = " << totalSumW << std::endl;
  std::cout << "* Total entries (manual)   = " << totalEntries_check << std::endl;
  std::cout << "* Total sumW    (manual)   = " << totalSumW_check << std::endl;
}
