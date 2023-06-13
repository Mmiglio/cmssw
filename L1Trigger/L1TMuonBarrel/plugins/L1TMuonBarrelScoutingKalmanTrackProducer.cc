#include <memory>
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"

#include "L1Trigger/L1TMuonBarrel/interface/L1TMuonBarrelKalmanAlgo.h"
#include "L1Trigger/L1TMuonBarrel/interface/L1TMuonBarrelKalmanTrackFinder.h"
#include "DataFormats/L1TMuon/interface/L1MuKBMTCombinedStub.h"
#include "DataFormats/L1TMuon/interface/RegionalMuonCandFwd.h"

#include "DataFormats/L1Scouting/interface/OrbitCollection.h"

//
// class declaration
//

class L1TMuonBarrelScoutingKalmanTrackProducer : public edm::stream::EDProducer<> {
public:
  explicit L1TMuonBarrelScoutingKalmanTrackProducer(const edm::ParameterSet&);
  ~L1TMuonBarrelScoutingKalmanTrackProducer() override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginStream(edm::StreamID) override;
  void produce(edm::Event&, const edm::EventSetup&) override;
  void endStream() override;
  edm::EDGetTokenT<std::vector<L1MuKBMTCombinedStub> > src_;
  int bxMin_;
  int bxMax_;
  L1TMuonBarrelKalmanAlgo* algo_;
  L1TMuonBarrelKalmanTrackFinder* trackFinder_;
};
L1TMuonBarrelScoutingKalmanTrackProducer::L1TMuonBarrelScoutingKalmanTrackProducer(const edm::ParameterSet& iConfig)
    : src_(consumes<scoutingRun3::BmtfStubOrbitCollection>(iConfig.getParameter<edm::InputTag>("src"))),
      bxMin_(iConfig.getParameter<int>("bxMin")),
      bxMax_(iConfig.getParameter<int>("bxMax")),
      algo_(new L1TMuonBarrelKalmanAlgo(iConfig.getParameter<edm::ParameterSet>("algoSettings"))),
      trackFinder_(new L1TMuonBarrelKalmanTrackFinder(iConfig.getParameter<edm::ParameterSet>("trackFinderSettings"))) {
  produces<L1MuKBMTrackBxCollection>();
  produces<l1t::RegionalMuonCandBxCollection>("BMTF");
}

L1TMuonBarrelScoutingKalmanTrackProducer::~L1TMuonBarrelScoutingKalmanTrackProducer() {
  if (algo_ != nullptr)
    delete algo_;

  if (trackFinder_ != nullptr)
    delete trackFinder_;

  // do anything here that needs to be done at destruction time
  // (e.g. close files, deallocate resources etc.)
}

//
// member functions
//

// ------------ method called to produce the data  ------------
void L1TMuonBarrelScoutingKalmanTrackProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;
  Handle<scoutingRun3::OrbitCollection<L1MuKBMTCombinedStub> > stubHandle;
  iEvent.getByToken(src_, stubHandle);

  std::vector<int> seenBxs;
  L1MuKBMTCombinedStubRefVector stubs;
  for (uint i = 0; i < stubHandle->sizeFlatData(); ++i) {
    L1MuKBMTCombinedStubRef r(stubHandle->getFlatData(), i);
    stubs.push_back(r);
    seenBxs.push_back(stubHandle->getFlatData(i).bxNum());
  }

  std::unique_ptr<l1t::RegionalMuonCandBxCollection> outBMTF(new l1t::RegionalMuonCandBxCollection());
  std::unique_ptr<L1MuKBMTrackBxCollection> out(new L1MuKBMTrackBxCollection());
  outBMTF->setBXRange(bxMin_, bxMax_);
  out->setBXRange(bxMin_, bxMax_);

  std::sort(seenBxs.begin(), seenBxs.end());
  seenBxs.erase(std::unique(seenBxs.begin(), seenBxs.end()), seenBxs.end());

  for (const auto& bx : seenBxs) {
    L1MuKBMTrackCollection tmp = trackFinder_->process(algo_, stubs, bx);
    for (const auto& track : tmp) {
      out->push_back(bx, track);
      algo_->addBMTFMuon(bx, track, outBMTF);
    }
  }
  iEvent.put(std::move(outBMTF), "BMTF");
  iEvent.put(std::move(out));
}

// ------------ method called once each stream before processing any runs, lumis or events  ------------
void L1TMuonBarrelScoutingKalmanTrackProducer::beginStream(edm::StreamID) {}

// ------------ method called once each stream after processing all runs, lumis and events  ------------
void L1TMuonBarrelScoutingKalmanTrackProducer::endStream() {}

void L1TMuonBarrelScoutingKalmanTrackProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(L1TMuonBarrelScoutingKalmanTrackProducer);
