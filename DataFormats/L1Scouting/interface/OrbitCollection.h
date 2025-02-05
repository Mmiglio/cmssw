#ifndef DataFormats_L1Scouting_OrbitCollection_h
#define DataFormats_L1Scouting_OrbitCollection_h


#include "DataFormats/Common/interface/traits.h"
#include "FWCore/Utilities/interface/GCCPrerequisite.h"

#include "DataFormats/L1Trigger/interface/Muon.h"
#include "DataFormats/L1Trigger/interface/EGamma.h"
#include "DataFormats/L1Trigger/interface/Jet.h"
#include "DataFormats/L1Trigger/interface/Tau.h"
#include "DataFormats/L1Trigger/interface/EtSum.h"
#include "DataFormats/L1TMuon/interface/L1MuKBMTCombinedStub.h"

#include <vector>

namespace scoutingRun3 {

  template <class T>
  class OrbitCollection {

    public:
      OrbitCollection(): bxIndex_(3565,0), bxData_(3565), nObjects_(0) {}

      void push_back(int bx, T &object) {
          bxData_[bx].push_back(object);
          nObjects_ ++;
      }

      void push_back_vector(int bx, std::vector<const T*> vec){
        for(const T* obj: vec){
          bxData_[bx].push_back(*obj);
          nObjects_ ++;
        }
      }

      void flatten() {
        flatData_.reserve(nObjects_);
        bxIndex_[0] = 0;
        int idx = 1;
        for (auto &bxVec: bxData_) {
            flatData_.insert(flatData_.end(), bxVec.begin(), bxVec.end());
            // increase index position
            bxIndex_[idx] = bxIndex_[idx-1] + bxVec.size();
            idx++;
        }
        //bxData_.clear();
      }

      // get the vector of objects for a specific bx
      std::vector<const T*> getBxVector(int bx) const {
        int currBxIdx = bxIndex_.at(bx);
        int nextBxIdx = bxIndex_.at(bx+1);
        std::vector<const T*> dataVector;

        for(int idx=currBxIdx; idx<nextBxIdx; idx++){
          dataVector.push_back(&flatData_[idx]);
        }
        return dataVector;
      }

      // to be removed?
      const std::vector<int>* getIndex() const { return &bxIndex_; }
      int getIndex(int i) const { return bxIndex_[i]; }

      // to be removed?
      const std::vector<T>* getFlatData() const { return &flatData_; }
      const T* getFlatData(int i) const { return &(flatData_[i]); }

      // to be removed?
      const std::vector<std::vector<T>>* getBxData() const { return &bxData_; }
      const std::vector<T>* getBxData(int i) const { return &(bxData_[i]); }

      int sizeFlatData() const { return flatData_.size(); }
      int sizeBxData() const { return bxData_.size(); }

    private:
      std::vector<int> bxIndex_;
      std::vector<T> flatData_;
      mutable std::vector<std::vector<T>> bxData_;
      int nObjects_;
  };

  typedef OrbitCollection<l1t::Muon>            MuonOrbitCollection;
  typedef OrbitCollection<l1t::Jet>             JetOrbitCollection;
  typedef OrbitCollection<l1t::EGamma>          EGammaOrbitCollection;
  typedef OrbitCollection<l1t::Tau>             TauOrbitCollection;
  typedef OrbitCollection<l1t::EtSum>           EtSumOrbitCollection;
  typedef OrbitCollection<L1MuKBMTCombinedStub> BmtfStubOrbitCollection;

}
#endif // DataFormats_L1Scouting_OrbitCollection_h
