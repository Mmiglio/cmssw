#include "EventFilter/L1ScoutingRawToDigi/plugins/ScBMTFRawToDigi.h"

ScBMTFRawToDigi::ScBMTFRawToDigi(const edm::ParameterSet& iConfig) {
  using namespace edm;
  srcInputTag = iConfig.getParameter<InputTag>("srcInputTag");
  debug_ = iConfig.getUntrackedParameter<bool>("debug", false);

  // initialize orbit buffer for BX 1->3564;
  orbitBuffer_ = std::vector<std::vector<l1ScoutingRun3::BMTFStub>>(3565);
  for (auto& bxVec : orbitBuffer_) {
    bxVec.reserve(32);
  }
  nStubsOrbit_ = 0;

  produces<l1ScoutingRun3::BMTFStubOrbitCollection>().setBranchAlias("BMTFStubOrbitCollection");
  rawToken = consumes<SDSRawDataCollection>(srcInputTag);
}

ScBMTFRawToDigi::~ScBMTFRawToDigi(){};

void ScBMTFRawToDigi::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;

  Handle<SDSRawDataCollection> ScoutingRawDataCollection;
  iEvent.getByToken(rawToken, ScoutingRawDataCollection);

  std::unique_ptr<l1ScoutingRun3::BMTFStubOrbitCollection> unpackedStubs(new l1ScoutingRun3::BMTFStubOrbitCollection);

  for (unsigned int sdsId=SDSNumbering::BmtfMinSDSID; sdsId<SDSNumbering::BmtfMaxSDSID; sdsId++) {
    // get data and orbit size from i^th source
    const FEDRawData& sourceRawData = ScoutingRawDataCollection->FEDData(sdsId);
    size_t orbitSize = sourceRawData.size();

    if((sourceRawData.size()==0) && debug_){
      std::cout << "No raw data for BMTF FED " << sdsId << std::endl;
    }
    
    // unpack current orbit and store data into the orbitBufferr
    unpackOrbit(sourceRawData.data(), orbitSize, sdsId);
  }

  // fill orbit collection and clear the Bx buffer vector
  unpackedStubs->fillAndClear(orbitBuffer_, nStubsOrbit_);

  // store collection in the event
  iEvent.put(std::move(unpackedStubs));
}

void ScBMTFRawToDigi::unpackOrbit(const unsigned char* buf, size_t len, unsigned int sdsId) {
  using namespace l1ScoutingRun3;

  // reset counters
  nStubsOrbit_ = 0;

  size_t pos = 0;

  while (pos < len) {
    assert(pos + 4 <= len);

    // get BX header
    uint32_t header = *((uint32_t*)(buf + pos));
    pos += 4;
    // decode header
    uint32_t bx     = (header & 0xffff0000) >> 16;
    uint32_t sCount = (header & 0x000000ff);
    // decode orbit number
    uint32_t orbit = *((uint32_t*)(buf + pos));
    pos += 4;
    orbit &= 0x7FFFFFFF;

    // declare block to read
    //bmtf::block *bl = (bmtf::block *)(buf + pos);
    pos += sCount*8;
    assert(pos <= len);

    if (debug_){
      std::cout  << " BMTF #" << sdsId << " Orbit " << orbit << ", BX -> "<< bx << ", nStubs -> " << sCount << std::endl;
    }
    

  }  // end orbit while loop
}

void ScBMTFRawToDigi::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(ScBMTFRawToDigi);
