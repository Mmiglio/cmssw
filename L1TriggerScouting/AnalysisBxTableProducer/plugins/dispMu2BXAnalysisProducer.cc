#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"

#include "DataFormats/L1Scouting/interface/OrbitCollection.h"
#include "DataFormats/L1Scouting/interface/AnalysisTable.h"

#include <vector>

using namespace scoutingRun3;

class dispMu2BXAnalysisProducer : public edm::stream::EDProducer<> {
public:
  explicit dispMu2BXAnalysisProducer(const edm::ParameterSet&);
  ~dispMu2BXAnalysisProducer() override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginStream(edm::StreamID) override;
  void produce(edm::Event&, const edm::EventSetup&) override;
  void endStream() override;

  // ----------member data ---------------------------
  edm::EDGetTokenT<MuonOrbitCollection>    gmtMuonsToken_;
  edm::EDGetTokenT<JetOrbitCollection>     caloJetsToken_;
  edm::EDGetTokenT<EGammaOrbitCollection>  caloEGammasToken_;
  edm::EDGetTokenT<TauOrbitCollection>     caloTausToken_;
  edm::EDGetTokenT<EtSumOrbitCollection>   caloEtSumsToken_;

  int minBx_, maxBx_;

  bool debug_ = false;
};

dispMu2BXAnalysisProducer::dispMu2BXAnalysisProducer(const edm::ParameterSet& iConfig){

  gmtMuonsToken_    = consumes<MuonOrbitCollection>(iConfig.getParameter<edm::InputTag>("gmtMuonsTag"));
  caloJetsToken_    = consumes<JetOrbitCollection>(iConfig.getParameter<edm::InputTag>("caloJetsTag"));
  caloEGammasToken_ = consumes<EGammaOrbitCollection>(iConfig.getParameter<edm::InputTag>("caloEGammasTag"));
  caloTausToken_    = consumes<TauOrbitCollection>(iConfig.getParameter<edm::InputTag>("caloTausTag"));
  caloEtSumsToken_  = consumes<EtSumOrbitCollection>(iConfig.getParameter<edm::InputTag>("caloEtSumsTag"));

  minBx_ = iConfig.getUntrackedParameter<int>("minBx", 0);
  maxBx_ = iConfig.getUntrackedParameter<int>("maxBx", 3564);

  debug_ = iConfig.getUntrackedParameter<bool>("debug", false);

  // products
  produces<AnalysisTable>().setBranchAlias( "AnalysisTable" );
}

dispMu2BXAnalysisProducer::~dispMu2BXAnalysisProducer() {}


void dispMu2BXAnalysisProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;

  // get the GMT muons
  edm::Handle<MuonOrbitCollection> gmtMuons;
  iEvent.getByToken(gmtMuonsToken_, gmtMuons);

  // get the CaloL2 objects
  edm::Handle<JetOrbitCollection> caloJets;
  iEvent.getByToken(caloJetsToken_, caloJets);

  edm::Handle<EGammaOrbitCollection> caloEGammas;
  iEvent.getByToken(caloEGammasToken_, caloEGammas);

  edm::Handle<TauOrbitCollection> caloTaus;
  iEvent.getByToken(caloTausToken_, caloTaus);

  edm::Handle<EtSumOrbitCollection> caloEtSums;
  iEvent.getByToken(caloEtSumsToken_, caloEtSums);

  // create analysis table to store results
  std::unique_ptr<AnalysisTable> analysisTab(new AnalysisTable);

  // run the analysis -> loop over all BX
  std::vector<const l1t::Muon*> bxMuons, nextBxMuons;
  bool vetoNextBx = false;

  for (int bx=minBx_; bx<maxBx_; bx++){
    // use a mask to process only some bunch crossings (e.g. the collision ones)
    // if(maskBx[bx]_): continue;

    if (vetoNextBx){
      vetoNextBx = false;
      continue;
    }

    bxMuons = gmtMuons->getBxVector(bx);
    nextBxMuons = gmtMuons->getBxVector(bx+1);

    bool foundDispMuonBx = false, foundDispMuonNextBx = false;
    for(size_t iMuon=0; iMuon<bxMuons.size(); ++iMuon){
      if(bxMuons[iMuon]->hwDXY()>0){
        foundDispMuonBx = true;
        break;
      }
    }

    if(foundDispMuonBx){
      for(size_t iMuon=0; iMuon<nextBxMuons.size(); ++iMuon){
        if(nextBxMuons[iMuon]->hwDXY()>0){
          foundDispMuonNextBx = true;
          break;
        }
      }
    }

    bool condition = foundDispMuonBx && foundDispMuonNextBx;

    if(debug_ && condition){
      std::cout << "BX: "<< bx <<"\n";
      std::cout << "\tnMuons       = " << bxMuons.size() << "\n";
      std::cout << "\tnMuons bx+1  = " << nextBxMuons.size() << "\n";
    }

    // add infos to the analysis table
    if (condition) {
      analysisTab->addBx(bx);
      analysisTab->addBx(bx+1);
      vetoNextBx = true;
    }
  
  }
  
  iEvent.put( std::move(analysisTab) );
}

// ------------ method called once each stream before processing any runs, lumis or events  ------------
void dispMu2BXAnalysisProducer::beginStream(edm::StreamID) {}

// ------------ method called once each stream after processing all runs, lumis and events  ------------
void dispMu2BXAnalysisProducer::endStream() {}


// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void dispMu2BXAnalysisProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(dispMu2BXAnalysisProducer);
