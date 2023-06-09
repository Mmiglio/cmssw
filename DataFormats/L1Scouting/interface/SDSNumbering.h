#ifndef L1Scouting_SDSNumbering_h
#define L1Scouting_SDSNumbering_h

/**
  *
  * This class holds the Scouting Data Source (SDS)
  * numbering scheme for the Level 1 scouting system
  *
  */

class SDSNumbering {
  public:
    static constexpr int lastSDSId() { return MAXSDSID; }

    enum {
      NOT_A_SDSID = -1,
      MAXSDSID = 21,
      GmtSDSID = 0,
      CaloSDSID = 1,
      BmtfMinSDSID = 10,
      BmtfMaxSDSID = 21
    };
};

#endif // L1Scouting_SDSNumbering_h