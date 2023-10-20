#ifndef DataFormats_L1Scouting_AnalysisTable_h
#define DataFormats_L1Scouting_AnalysisTable_h

#include "DataFormats/Common/interface/traits.h"
#include "FWCore/Utilities/interface/GCCPrerequisite.h"

#include <vector>

namespace scoutingRun3 {
    class AnalysisTable{
        private:
            std::vector<int> bxList_;
            //Maybe store ID?
            //TODO: add arrays to store additional infos
        public:
            AnalysisTable() {};
            ~AnalysisTable() {};
            
            void addBx(int bx){
                bxList_.push_back(bx);
            };

            const std::vector<int>* getBxList() const {
                return &bxList_;
            }; 
    };
}

#endif 
