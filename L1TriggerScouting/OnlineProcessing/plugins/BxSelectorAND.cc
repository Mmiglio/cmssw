#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Utilities/interface/EDPutToken.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Utilities/interface/StreamID.h"
#include "FWCore/Utilities/interface/Span.h"

#include <vector>
#include <map>


/*
 * Find BX selected by a list of analyses
 */
class BxSelectorAND : public edm::stream::EDProducer<> {
public:
  explicit BxSelectorAND(const edm::ParameterSet&);
  ~BxSelectorAND() {}
  static void fillDescriptions(edm::ConfigurationDescriptions&);

private:
  void produce(edm::Event&, const edm::EventSetup&) override;

  // tokens for BX selected by each analysis
  std::vector<edm::EDGetTokenT<std::vector<unsigned>>> selectedBxsToken_;
};

BxSelectorAND::BxSelectorAND(const edm::ParameterSet& iPSet) {
  // get the list of selected BXs
  std::vector<edm::InputTag> bxLabels = iPSet.getParameter<std::vector<edm::InputTag>>("analysisLabels");
  for (const auto& bxLabel : bxLabels) {
    selectedBxsToken_.push_back(consumes<std::vector<unsigned>>(bxLabel));
  }

  produces<std::vector<unsigned>>("SelBx").setBranchAlias("SelectedBxs");
}

// ------------ method called for each ORBIT  ------------
void BxSelectorAND::produce(edm::Event& iEvent, const edm::EventSetup&) {

  std::map<int, int> bxCounts;
  for (const auto& token : selectedBxsToken_) {
    edm::Handle<std::vector<unsigned>> bxList;
    iEvent.getByToken(token, bxList);

    for (const unsigned& bx : *bxList) {
      bxCounts[bx] ++;
    }
  }
  
  // the AND is valid if a BX has been selected by all the selectors
  std::unique_ptr<std::vector<unsigned>> selectedBxs(new std::vector<unsigned>());
  for(const auto& elem: bxCounts){
    if(static_cast<size_t>(elem.second) == selectedBxsToken_.size()){
      selectedBxs->push_back(elem.first);
    }
  }
  
  iEvent.put(std::move(selectedBxs), "SelBx");
}

void BxSelectorAND::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(BxSelectorAND);
