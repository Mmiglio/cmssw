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
#include <set>

using namespace scoutingRun3;

class FinalBxSelector : public edm::stream::EDProducer<> {
public:
  explicit FinalBxSelector(const edm::ParameterSet&);
  ~FinalBxSelector() override;

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

  std::vector<edm::EDGetTokenT<AnalysisTable>>   analysisTabToken_;

  bool debug_ = false;
};

FinalBxSelector::FinalBxSelector(const edm::ParameterSet& iConfig){

  gmtMuonsToken_    = consumes<MuonOrbitCollection>(iConfig.getParameter<edm::InputTag>("gmtMuonsTag"));
  caloJetsToken_    = consumes<JetOrbitCollection>(iConfig.getParameter<edm::InputTag>("caloJetsTag"));
  caloEGammasToken_ = consumes<EGammaOrbitCollection>(iConfig.getParameter<edm::InputTag>("caloEGammasTag"));
  caloTausToken_    = consumes<TauOrbitCollection>(iConfig.getParameter<edm::InputTag>("caloTausTag"));
  caloEtSumsToken_  = consumes<EtSumOrbitCollection>(iConfig.getParameter<edm::InputTag>("caloEtSumsTag"));

  // get the analysis tabs
  std::vector<edm::InputTag> analysisLabels_ = iConfig.getParameter<std::vector<edm::InputTag> >("analysisLabels");
  for (std::vector<edm::InputTag>::const_iterator analysisLabel = analysisLabels_.begin(), analysisLabelEnd = analysisLabels_.end();
       analysisLabel != analysisLabelEnd;
       ++analysisLabel) {
        analysisTabToken_.push_back(consumes<AnalysisTable>(*analysisLabel));
  }

  debug_ = iConfig.getUntrackedParameter<bool>("debug", false);

  // products
  produces<MuonOrbitCollection>().setBranchAlias( "MuonOrbitCollection" );
  produces<JetOrbitCollection>().setBranchAlias( "JetOrbitCollection" );
  produces<TauOrbitCollection>().setBranchAlias( "TauOrbitCollection" );
  produces<EGammaOrbitCollection>().setBranchAlias( "EGammaOrbitCollection" );
  produces<EtSumOrbitCollection>().setBranchAlias( "EtSumOrbitCollection" );
}

FinalBxSelector::~FinalBxSelector() {}


void FinalBxSelector::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
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


  // create empty Orbit Collections for storage
  std::unique_ptr<MuonOrbitCollection> selectedMuons(new MuonOrbitCollection);
  std::unique_ptr<JetOrbitCollection> selectedJets(new JetOrbitCollection);
  std::unique_ptr<TauOrbitCollection> selectedTaus(new TauOrbitCollection);
  std::unique_ptr<EGammaOrbitCollection> selectedEGammas(new EGammaOrbitCollection);
  std::unique_ptr<EtSumOrbitCollection> selectedEtSums(new EtSumOrbitCollection);


  // get all analysis tables
  std::set<int> uniqueBxs;
  for (unsigned int i = 0; i < analysisTabToken_.size(); ++i){
    edm::Handle<AnalysisTable> analysisTable;
    iEvent.getByToken(analysisTabToken_[i], analysisTable);

    // get the BX list for the analysis
    const std::vector<int>* bxList = analysisTable->getBxList();

    for(const int& selBx: *bxList){
      uniqueBxs.insert(selBx);
    }
  }

  // elements of each bx
  std::vector<const l1t::Muon*> bxMuons;
  std::vector<const l1t::Jet*> bxJets;
  std::vector<const l1t::EGamma*> bxEGammas;
  std::vector<const l1t::Tau*> bxTaus;
  std::vector<const l1t::EtSum*> bxEtSums;

  // loop throught the uniques BX and store them
  for(const int& selBx: uniqueBxs){
    bxMuons   = gmtMuons->getBxVector(selBx);
    bxJets    = caloJets->getBxVector(selBx);
    bxEGammas = caloEGammas->getBxVector(selBx);
    bxTaus    = caloTaus->getBxVector(selBx);
    bxEtSums  = caloEtSums->getBxVector(selBx);

    selectedMuons->push_back_vector(selBx, bxMuons);
    selectedJets->push_back_vector(selBx, bxJets);
    selectedEGammas->push_back_vector(selBx, bxEGammas);
    selectedTaus->push_back_vector(selBx, bxTaus);
    selectedEtSums->push_back_vector(selBx, bxEtSums);
  }

  // flatten collection
  selectedMuons->flatten();
  selectedJets->flatten();
  selectedEGammas->flatten();
  selectedTaus->flatten();
  selectedEtSums->flatten();

  // store collections in the event
  iEvent.put( std::move(selectedMuons) );
  iEvent.put( std::move(selectedJets) );
  iEvent.put( std::move(selectedTaus) );
  iEvent.put( std::move(selectedEGammas) );
  iEvent.put( std::move(selectedEtSums) );

}

// ------------ method called once each stream before processing any runs, lumis or events  ------------
void FinalBxSelector::beginStream(edm::StreamID) {}

// ------------ method called once each stream after processing all runs, lumis and events  ------------
void FinalBxSelector::endStream() {}


// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void FinalBxSelector::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(FinalBxSelector);
