#include "CommonTools/Utils/interface/StringCutObjectSelector.h"
#include "DataFormats/Candidate/interface/CandMatchMap.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "CustoTnP/Analyzer/src/PATUtilities.h"
#include "CustoTnP/Analyzer/src/TriggerUtilities.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/L1Trigger/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"
#include "DataFormats/PatCandidates/interface/PackedTriggerPrescales.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/Common/interface/View.h"
#include "TLorentzVector.h"

//~
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"

#include "Geometry/DTGeometry/interface/DTGeometry.h"
#include "Geometry/DTGeometry/interface/DTLayer.h"
#include "Geometry/DTGeometry/interface/DTSuperLayer.h"
#include "DataFormats/DTRecHit/interface/DTSLRecSegment2D.h"
#include "RecoLocalMuon/DTSegment/src/DTSegmentUpdator.h"
#include "RecoLocalMuon/DTSegment/src/DTSegmentCleaner.h"
#include "RecoLocalMuon/DTSegment/src/DTHitPairForFit.h"

#include "Geometry/CSCGeometry/interface/CSCGeometry.h"
#include "Geometry/CSCGeometry/interface/CSCLayer.h"
#include "DataFormats/MuonDetId/interface/CSCDetId.h"
#include "DataFormats/CSCRecHit/interface/CSCRecHit2D.h"
#include "DataFormats/CSCRecHit/interface/CSCRangeMapAccessor.h"

#include "DataFormats/DTRecHit/interface/DTRecSegment4DCollection.h"
#include "DataFormats/DTRecHit/interface/DTRecHitCollection.h"
#include "DataFormats/CSCRecHit/interface/CSCSegmentCollection.h"
// #include "DataFormats/MuonReco/interface/MuonShower.h"

#include "DataFormats/DTDigi/interface/DTDigi.h"
#include "DataFormats/DTDigi/interface/DTDigiCollection.h"
#include "DataFormats/CSCDigi/interface/CSCWireDigi.h"
#include "DataFormats/CSCDigi/interface/CSCWireDigiCollection.h"
#include "DataFormats/CSCDigi/interface/CSCStripDigi.h"
#include "DataFormats/CSCDigi/interface/CSCStripDigiCollection.h"



class CustoTnPLeptonProducer : public edm::EDProducer {
public:
  explicit CustoTnPLeptonProducer(const edm::ParameterSet&);

private:
  virtual void produce(edm::Event&, const edm::EventSetup&);

  pat::Muon*     cloneAndSwitchMuonTrack     (const pat::Muon&, const edm::Event& event)     const;

  void embedTriggerMatch(pat::Muon*, std::string, const pat::TriggerObjectStandAloneCollection&, std::vector<int>&);
  // void embedTriggerMatch_or(pat::Muon*, const std::string&, const pat::TriggerObjectStandAloneCollection&, const pat::TriggerObjectStandAloneCollection&, std::vector<int>&, std::vector<int>&);

  //~
  void embedExpectedMatchedStations(pat::Muon*, float);

  std::pair<pat::Muon*, int> doLepton(const edm::Event&, const pat::Muon&, const reco::CandidateBaseRef&,
                                          const edm::ESHandle<DTGeometry>&, const edm::ESHandle<CSCGeometry>&);

  template <typename T> edm::OrphanHandle<std::vector<T> > doLeptons( edm::Event&, const edm::InputTag&, const edm::InputTag&, const std::string&,
                                                                      const edm::ESHandle<DTGeometry>&, const edm::ESHandle<CSCGeometry>&);

  edm::InputTag muon_src;
  edm::InputTag muon_view_src;
  StringCutObjectSelector<pat::Muon> muon_selector;
  std::string muon_track_for_momentum;
  std::string muon_track_for_momentum_primary;
  std::vector<std::string> muon_tracks_for_momentum;
  edm::InputTag muon_photon_match_src;
  edm::Handle<reco::CandViewMatchMap> muon_photon_match_map;

  double trigger_match_max_dR;
  std::vector<std::string> trigger_filters;
  std::vector<std::string> trigger_path_names;
  edm::EDGetTokenT<edm::TriggerResults> triggerBits_;
  edm::EDGetTokenT<pat::TriggerObjectStandAloneCollection> trigger_summary_src_;
  edm::EDGetTokenT<pat::PackedTriggerPrescales> triggerPrescales_;

  std::vector<pat::TriggerObjectStandAloneCollection> vec_L3_muons;
  std::vector<std::vector<int>> vec_L3_muons_matched;

  edm::EDGetTokenT<l1t::MuonBxCollection> l1_src_;

  //@
  bool isAOD;
  edm::InputTag reco_muon_src;
  // edm::InputTag muonshower_src;
  edm::InputTag dtseg_src;
  edm::InputTag cscseg_src;

  bool hasRAW;
  edm::InputTag dtdigis_src;
  edm::InputTag cscwiredigis_src;
  edm::InputTag cscStripdigis_src;

  bool verboseShower;

  std::pair<bool, reco::MuonRef> getMuonRef(const edm::Event&, pat::Muon*);
  std::vector<int> countDTdigis(const edm::Event&, reco::MuonRef, const edm::ESHandle<DTGeometry>&, bool);
  std::vector<int> countCSCdigis(const edm::Event&, reco::MuonRef, const edm::ESHandle<CSCGeometry>&, bool);
  std::vector<int> countDTsegs(const edm::Event&, reco::MuonRef, bool);
  std::vector<int> countCSCsegs(const edm::Event&, reco::MuonRef, const edm::ESHandle<CSCGeometry>&, bool);
  void embedShowerInfo(const edm::Event&, pat::Muon*, reco::MuonRef, const edm::ESHandle<DTGeometry>&, const edm::ESHandle<CSCGeometry>&, bool);
};

CustoTnPLeptonProducer::CustoTnPLeptonProducer(const edm::ParameterSet& cfg)
  : muon_src(cfg.getParameter<edm::InputTag>("muon_src")),
    muon_view_src(cfg.getParameter<edm::InputTag>("muon_src")),
    muon_selector(cfg.getParameter<std::string>("muon_cuts")),
    muon_track_for_momentum(cfg.getParameter<std::string>("muon_track_for_momentum")),
    muon_track_for_momentum_primary(muon_track_for_momentum),
    muon_photon_match_src(cfg.getParameter<edm::InputTag>("muon_photon_match_src")),

    trigger_match_max_dR(cfg.getParameter<double>("trigger_match_max_dR")),
    trigger_filters(cfg.getParameter<std::vector<std::string>>("trigger_filters")),
    trigger_path_names(cfg.getParameter<std::vector<std::string>>("trigger_path_names")),

    triggerBits_(consumes<edm::TriggerResults>(cfg.getParameter<edm::InputTag>("bits"))),
    trigger_summary_src_(consumes<pat::TriggerObjectStandAloneCollection>(cfg.getParameter<edm::InputTag>("trigger_summary"))),
    triggerPrescales_(consumes<pat::PackedTriggerPrescales>(cfg.getParameter<edm::InputTag>("prescales"))),
    l1_src_(consumes<l1t::MuonBxCollection>(cfg.getParameter<edm::InputTag>("l1"))),

    isAOD(cfg.getParameter<bool>("isAOD")),
    hasRAW(cfg.getParameter<bool>("hasRAW")),
    verboseShower(cfg.getParameter<bool>("verboseShower"))
{
  consumes<edm::View<reco::Candidate>>(muon_view_src);
  consumes<pat::MuonCollection>(muon_src);
  consumes<reco::CandViewMatchMap >(muon_photon_match_src);

  if (cfg.existsAs<std::vector<std::string> >("muon_tracks_for_momentum"))
    muon_tracks_for_momentum = cfg.getParameter<std::vector<std::string> >("muon_tracks_for_momentum");

  if (muon_tracks_for_momentum.size())
    for (size_t i = 0; i < muon_tracks_for_momentum.size(); ++i)
      produces<pat::MuonCollection>(muon_tracks_for_momentum[i]);

  if(isAOD) {
    reco_muon_src  = (cfg.getParameter<edm::InputTag>("reco_muon_src"));
    // muonshower_src = (cfg.getParameter<edm::InputTag>("muonshower_src"));
    dtseg_src      = (cfg.getParameter<edm::InputTag>("dtseg_src"));
    cscseg_src     = (cfg.getParameter<edm::InputTag>("cscseg_src"));

    consumes<std::vector< reco::Muon >>(reco_muon_src);
    // consumes<edm::ValueMap<reco::MuonShower>>(muonshower_src);
    consumes<DTRecSegment4DCollection>(dtseg_src);
    consumes<CSCSegmentCollection>(cscseg_src);

    if(hasRAW) {
      dtdigis_src       = (cfg.getParameter<edm::InputTag>("dtdigis_src"));
      cscwiredigis_src  = (cfg.getParameter<edm::InputTag>("cscwiredigis_src"));
      cscStripdigis_src = (cfg.getParameter<edm::InputTag>("cscStripdigis_src"));

      consumes<DTDigiCollection>(dtdigis_src);
      consumes<CSCWireDigiCollection>(cscwiredigis_src);
      consumes<CSCStripDigiCollection>(cscStripdigis_src);
    }
  }

  produces<pat::MuonCollection>("muons");
}


pat::Muon* CustoTnPLeptonProducer::cloneAndSwitchMuonTrack(const pat::Muon& muon, const edm::Event& event) const {

  // Muon mass to make a four-vector out of the new track.

  pat::Muon* mu = muon.clone();

  // Start with null track/invalid type before we find the right one.
  reco::TrackRef newTrack;
  newTrack = muon.tunePMuonBestTrack();
  patmuon::TrackType type = patmuon::nTrackTypes;

  // If the muon has the track embedded using the UserData mechanism,
  // take it from there first. Otherwise, try to get the track the
  // standard way.

  if (muon.pt() > 100.) {
	mu->addUserInt("hasTeVMuons", 1);
  }
  else{

	 mu->addUserInt("hasTeVMuons", 0); 
  }
  if (muon.hasUserData(muon_track_for_momentum))
    newTrack = patmuon::userDataTrack(muon, muon_track_for_momentum);
  else {
 
	type = patmuon::trackNameToType(muon_track_for_momentum);
    	newTrack = patmuon::trackByType(*mu, type);
  }
  
  // If we didn't find the appropriate track, indicate failure by a
  // null pointer.
  
  if (newTrack.isNull()){
    std::cout << "No TuneP" << std::endl;
    return 0;
    
  }
  
  
  
  static const double mass = 0.10566;
  
  reco::Particle::Point vtx(newTrack->vx(), newTrack->vy(), newTrack->vz());
  reco::Particle::LorentzVector p4;

  //////////   Comment following lines to apply pt bias correction /////
    const double p = newTrack->p();  
    p4.SetXYZT(newTrack->px(), newTrack->py(), newTrack->pz(), sqrt(p*p + mass*mass));  
  //////////   Comment previous lines to apply pt bias correction ----->  Uncomment following lines /////


  ///////// uncomment following lines to apply pt bias correction -----> comment previous lines /////////
    //  float phi = newTrack->phi()*TMath::RadToDeg();

    //  float mupt = GeneralizedEndpoint().GeneralizedEndpointPt(newTrack->pt(),newTrack->charge(),newTrack->eta(),phi,-1,1); //for DATA
    //  float mupt = GeneralizedEndpoint().GeneralizedEndpointPt(newTrack->pt(),newTrack->charge(),newTrack->eta(),phi,0,1);  // for MC


    //	float px = mupt*TMath::Cos(newTrack->phi());
    //	float py = mupt*TMath::Sin(newTrack->phi());
    //	float pz = mupt*TMath::SinH(newTrack->eta());
    //	float p = mupt*TMath::CosH(newTrack->eta());
    //	p4.SetXYZT(px, py, pz, sqrt(p*p + mass*mass));

    // 	std::cout<<"my definition = "<<mupt<<std::endl;
  /////// uncomment previous lines to apply pt bias correction /////////


  mu->setP4(p4);  

  mu->setCharge(newTrack->charge());

  mu->setVertex(vtx);

  mu->addUserInt("trackUsedForMomentum", type);
  
  return mu;
}


void CustoTnPLeptonProducer::embedTriggerMatch(pat::Muon* new_mu, std::string ex, const pat::TriggerObjectStandAloneCollection& L3, std::vector<int>& L3_matched) {
  
  int best = -1;
  float defaultpTvalue = -1.;
  float best_dR = trigger_match_max_dR;
  //std::cout << "size of L3 collection: " << L3.size() << std::endl;
  for (size_t i = 0; i < L3.size(); ++i) {
    // Skip those already used.
    if (L3_matched[i])
      continue;

    const float dR = reco::deltaR(L3[i], *new_mu);
    if (dR < best_dR) {
      best = int(i);
      best_dR = dR;
    }
  }

  // if (best < 0)
  //  return;
  if (ex.length()>0) ex += "_";
  if (best >= 0) {
    const pat::TriggerObjectStandAlone& L3_mu = L3[best];
    L3_matched[best] = 1;
    
    int id = L3_mu.pdgId();
    new_mu->addUserFloat(ex + "TriggerMatchCharge", -id/abs(id));
    new_mu->addUserFloat(ex + "TriggerMatchPt",     L3_mu.pt());
    new_mu->addUserFloat(ex + "TriggerMatchEta",    L3_mu.eta());
    new_mu->addUserFloat(ex + "TriggerMatchPhi",    L3_mu.phi());
  }
  else {
    new_mu->addUserFloat(ex + "TriggerMatchPt",    defaultpTvalue);
  }

}

//~
void CustoTnPLeptonProducer::embedExpectedMatchedStations(pat::Muon* new_mu, float minDistanceFromEdge = 10.) {
  unsigned int stationMask = 0;
  for( auto& chamberMatch : new_mu->matches() )
  {
    if (chamberMatch.detector()!=MuonSubdetId::DT && chamberMatch.detector()!=MuonSubdetId::CSC) continue;
    float edgeX = chamberMatch.edgeX;
    float edgeY = chamberMatch.edgeY;
    // check we if the trajectory is well within the acceptance
    /*
    if(chamberMatch.detector()==MuonSubdetId::DT) {
      DTChamberId ID( chamberMatch.id.rawId() );
      std::cout << "\tChamberId: " << ID << std::endl;
    }
    if(chamberMatch.detector()==MuonSubdetId::CSC) {
      CSCDetId    ID( chamberMatch.id.rawId() );
      std::cout << "\tChamberId: " << ID << std::endl;
    }
    std::cout << "\t\tedgeX: " << edgeX << "\tedgeY: " << edgeY << "\t" << (edgeX<0 && fabs(edgeX)>fabs(minDistanceFromEdge) && edgeY<0 && fabs(edgeY)>fabs(minDistanceFromEdge)) <<std::endl;
    */
    if(edgeX<0 && fabs(edgeX)>fabs(minDistanceFromEdge) && edgeY<0 && fabs(edgeY)>fabs(minDistanceFromEdge))
      stationMask |= 1<<( (chamberMatch.station()-1)+4*(chamberMatch.detector()-1) );
  }
  unsigned int n = 0;
  for(unsigned int i=0; i<8; ++i)
    if (stationMask&(1<<i)) n++;

  std::string var_temp = "expectedNnumberOfMatchedStations"+std::to_string( int(minDistanceFromEdge) );
  new_mu->addUserInt(var_temp, n);
}

//@
std::pair<bool, reco::MuonRef> CustoTnPLeptonProducer::getMuonRef(const edm::Event& event, pat::Muon* new_mu) {

  edm::Handle< std::vector< reco::Muon > > recoMuons;
  event.getByLabel(reco_muon_src, recoMuons);

  bool isMatched = false;
  reco::MuonRef matchedMuRef = reco::MuonRef(recoMuons, 0);

  int imucount = 0;
  for (std::vector<reco::Muon>::const_iterator imu = recoMuons->begin(); imu != recoMuons->end(); imu++) {
    if(imu->globalTrack()->pt() == new_mu->globalTrack()->pt() &&
       imu->globalTrack()->eta() == new_mu->globalTrack()->eta() &&
       imu->globalTrack()->phi() == new_mu->globalTrack()->phi() ) {
      isMatched = true;
      matchedMuRef = reco::MuonRef(recoMuons, imucount);
      break;
    }
    imucount++;
  }

  return make_pair(isMatched, matchedMuRef);
}

std::vector<int> CustoTnPLeptonProducer::countDTdigis(const edm::Event& event, reco::MuonRef muon,
                                                      const edm::ESHandle<DTGeometry> & dtGeom,
                                                      bool verbose = false)
{

  float dXcut = 25.;

  if(verbose)  std::cout << "\n   countDTdigis: dXcut=" << dXcut << std::endl;

  edm::Handle<DTDigiCollection> dtDigis;
  if( event.getByLabel(dtdigis_src,dtDigis) ) {

    std::vector<int> stations={0,0,0,0};

    for(const auto &ch : muon->matches()) {
      if( ch.detector() != MuonSubdetId::DT )  continue;
      DTChamberId DTid( ch.id.rawId() );
      int st_tmp = ch.station();
      int ndigisPerCh = 0;

      if(verbose)  std::cout << "\t Matched chamber: " << DTid << " (" << ch.x << ", " << ch.y << ", 0)" << std::endl;

      DTDigiCollection::DigiRangeIterator dtLayerIdIt  = dtDigis->begin();
      DTDigiCollection::DigiRangeIterator dtLayerIdEnd = dtDigis->end();
      for(; dtLayerIdIt!=dtLayerIdEnd; ++dtLayerIdIt) {

        if( DTid.station() == (*dtLayerIdIt).first.station() &&
            DTid.wheel()   == (*dtLayerIdIt).first.wheel() &&
            DTid.sector()  == (*dtLayerIdIt).first.sector() ) {

          DTDigiCollection::const_iterator digiIt = (*dtLayerIdIt).second.first;
          for(;digiIt!=(*dtLayerIdIt).second.second; ++digiIt) {
            const auto topo = dtGeom->layer((*dtLayerIdIt).first)->specificTopology();
            float xWire = topo.wirePosition((*digiIt).wire());
            float dX = std::abs(ch.x - xWire);

            if(verbose)  std::cout << "\t\t new digi found: xWire=" << xWire << " SL=" << (*dtLayerIdIt).first.superLayer() << std::endl;

            if( (dX < dXcut) && ((*dtLayerIdIt).first.superLayer() == 1 || (*dtLayerIdIt).first.superLayer() == 3) ) {
              ndigisPerCh++;
              if(verbose)  std::cout << "\t\t\t --> pass " << ndigisPerCh << std::endl;
            }

          }
        }
      }

      if( stations[st_tmp-1] < ndigisPerCh ) {
        stations[st_tmp-1] = ndigisPerCh;
        if(verbose)  std::cout << "\t\t   updated # digis in station " << st_tmp << ": " << ndigisPerCh << std::endl;
      }
    }

    return stations;
  }
  else
    return { -1, -1, -1, -1 };
}

std::vector<int> CustoTnPLeptonProducer::countCSCdigis(const edm::Event& event, reco::MuonRef muon,
                                                       const edm::ESHandle<CSCGeometry> & cscGeom,
                                                       bool verbose = false)
{

  float dXcut = 25.;

  if(verbose)  std::cout << "\n   countCSCdigis: dXcut=" << dXcut << std::endl;

  edm::Handle<CSCWireDigiCollection>  cscWireDigis;
  edm::Handle<CSCStripDigiCollection> cscStripDigis;
  if( event.getByLabel(cscwiredigis_src,cscWireDigis) &&
      event.getByLabel(cscStripdigis_src,cscStripDigis) ) {

    std::vector<int> stations={0,0,0,0};
    std::map<int, int> me11DigiPerSec;

    for(const auto &ch : muon->matches()) {
      if( ch.detector() != MuonSubdetId::CSC )  continue;
      CSCDetId CSCid( ch.id.rawId() );
      int st_tmp = ch.station();
      int ndigisPerCh = 0;

      if(verbose)  std::cout << "\t Matched chamber: " << CSCid << " (" << ch.x << ", " << ch.y << ", 0)" << std::endl;

      CSCStripDigiCollection::DigiRangeIterator cscStripLayerIdIt  = cscStripDigis->begin();
      CSCStripDigiCollection::DigiRangeIterator cscStripLayerIdEnd = cscStripDigis->end();
      for(; cscStripLayerIdIt!=cscStripLayerIdEnd; ++cscStripLayerIdIt) {

        if( CSCid.zendcap() == (*cscStripLayerIdIt).first.zendcap() &&
            CSCid.station() == (*cscStripLayerIdIt).first.station() &&
            CSCid.ring()    == (*cscStripLayerIdIt).first.ring() &&
            CSCid.chamber() == (*cscStripLayerIdIt).first.chamber() ) {

          Bool_t isME11 = ( CSCid.station() == 1 && (CSCid.ring() == 1 || CSCid.ring() == 4) );

          CSCStripDigiCollection::const_iterator digiIt = (*cscStripLayerIdIt).second.first;
          for (;digiIt!=(*cscStripLayerIdIt).second.second; ++digiIt) {

            std::vector<int> adcVals = digiIt->getADCCounts();
            bool hasFired = false;
            float pedestal = 0.5*(float)(adcVals[0]+adcVals[1]);
            float threshold = 13.3;
            float diff = 0.;
            for (const auto & adcVal : adcVals) {
              diff = (float)adcVal - pedestal;
              if (diff > threshold) {
                hasFired = true; 
                break;
              }
            }
            if(!hasFired)  continue;

            const CSCLayer* layer = cscGeom->layer((*cscStripLayerIdIt).first);
            const CSCLayerGeometry* layerGeom = layer->geometry();
            Float_t xStrip = layerGeom->xOfStrip(digiIt->getStrip(), ch.y);
            Float_t dX = std::abs(ch.x - xStrip);

            if(verbose)  std::cout << "\t\t new digi found: xStrip=" << xStrip << std::endl;

            if( dX < dXcut ) {
              if(isME11) {
                if(me11DigiPerSec.find(CSCid.chamber()) == me11DigiPerSec.end())
                  me11DigiPerSec[CSCid.chamber()] = 0;
                me11DigiPerSec[CSCid.chamber()]++;
                if(verbose)  std::cout << "\t\t\t --> pass(ME11 chamber " << CSCid.chamber() << "): " << me11DigiPerSec[CSCid.chamber()] << std::endl;
              }
              else {
                ndigisPerCh++;
                if(verbose)  std::cout << "\t\t\t --> pass " << ndigisPerCh << std::endl;
              }
            }

          }
        }
      }

      if( stations[st_tmp-1] < ndigisPerCh ) {
        stations[st_tmp-1] = ndigisPerCh;
        if(verbose)  std::cout << "\t\t updated # digis in station " << st_tmp << ": " << ndigisPerCh << std::endl;
      }
    }

    for (const auto & me11DigiAndSec : me11DigiPerSec)
    {
      int nDigi = me11DigiAndSec.second;
      if(stations[0] < nDigi) {
        stations[0] = nDigi;
        if(verbose)  std::cout << "\t\t updated # digis in station 0: " << nDigi << std::endl;
      }
    }

    return stations;
  }
  else
    return { -1, -1, -1, -1 };
}

std::vector<int> CustoTnPLeptonProducer::countDTsegs(const edm::Event& event, reco::MuonRef muon,
                                                     bool verbose = false)
{

  float dXcut = 25.;

  if(verbose)  std::cout << "\n   countDTsegs: dXcut=" << dXcut << std::endl;

  edm::Handle<DTRecSegment4DCollection> dtSegments;
  if( event.getByLabel(dtseg_src, dtSegments) ) {

    std::vector<int> stations={0,0,0,0};
    std::vector<int> removed ={0,0,0,0};

    for (const auto &ch : muon->matches()) {
      if( ch.detector() != MuonSubdetId::DT )  continue;
      DTChamberId DTid( ch.id.rawId() );

      if(verbose)  std::cout << "\t Matched chamber: " << DTid << " (" << ch.x << ", " << ch.y << ", 0)" << std::endl;

      std::vector< std::pair<float,int> > nsegs_x_temp = {};

      for(auto seg = dtSegments->begin(); seg!=dtSegments->end(); ++seg) {
        DTChamberId myChamber((*seg).geographicalId().rawId());
        if (!(DTid==myChamber))  continue;
        if(!seg->hasPhi())  continue;
        LocalPoint posLocalSeg = seg->localPosition();
        int nHitsX = (seg->phiSegment()->specificRecHits()).size();

        if( fabs(posLocalSeg.x()-ch.x)<dXcut && nHitsX > 0) {
          bool found = false;
          for( auto prev_x : nsegs_x_temp) {
            if( fabs(prev_x.first - posLocalSeg.x()) < 0.005 && prev_x.second == nHitsX ) {
              found = true;
              break;
            }
          }
          if( !found ) {
            nsegs_x_temp.push_back( make_pair(posLocalSeg.x(), nHitsX) );
            if(verbose)  std::cout << "\t\t new segment:" << posLocalSeg << std::endl;
          }
        }
      }

      int nsegs_temp = (int)nsegs_x_temp.size();
      if(nsegs_temp>0)  stations[ch.station()-1] += nsegs_temp;

      //--- subtract best matched segment from given muon
      for(std::vector<reco::MuonSegmentMatch>::const_iterator matseg = ch.segmentMatches.begin(); matseg != ch.segmentMatches.end(); matseg++) {
        if(!matseg->hasPhi())  continue;
        if(!(fabs(matseg->x-ch.x)<dXcut))  continue;
        if( matseg->isMask(reco::MuonSegmentMatch::BestInChamberByDR) && matseg->isMask(reco::MuonSegmentMatch::BelongsToTrackByDR) ) {
          removed[ch.station()-1]++;
          if(verbose)  std::cout << "\t\t   arb segment:" << "(" << matseg->x << ", " << matseg->y << ", 0)" << std::endl;
        }
      }

    }

    for(int i=0; i<4; ++i) {
      if(verbose)  std::cout << "\t\t\t # ext segments in station " << i << ": "
                             << stations[i] << "-" << removed[i] << "=" << (stations[i] - removed[i]) << std::endl;
      stations[i] = stations[i] - removed[i];
    }
    return stations;
  }
  else
    return { -1, -1, -1, -1 };
}

std::vector<int> CustoTnPLeptonProducer::countCSCsegs(const edm::Event& event, reco::MuonRef muon,
                                                      const edm::ESHandle<CSCGeometry> & cscGeom,
                                                      bool verbose = false)
{

  float dXcut = 25.;

  if(verbose)  std::cout << "\n   countCSCsegs: dXcut=" << dXcut << std::endl;

  edm::Handle<CSCSegmentCollection> cscSegments;
  if( event.getByLabel(cscseg_src, cscSegments) ) {

    std::vector<int> stations={0,0,0,0};
    std::vector<int> removed ={0,0,0,0};

    for (const auto &ch : muon->matches()) {
      if( ch.detector() != MuonSubdetId::CSC )  continue;
      CSCDetId CSCid( ch.id.rawId() );

      if(verbose)  std::cout << "\t Matched chamber: " << CSCid << " (" << ch.x << ", " << ch.y << ", 0)" << std::endl;

      std::vector< std::pair<float,int> > nsegs_phi_temp = {};

      for (auto seg = cscSegments->begin(); seg!=cscSegments->end(); ++seg) {
        CSCDetId myChamber((*seg).geographicalId().rawId());
        if (!(CSCid==myChamber))  continue;
        LocalPoint posLocalSeg = seg->localPosition();
        int nHitsX = seg->nRecHits();

        const auto chamb = cscGeom->chamber(ch.id);
        float phi = chamb->toGlobal(seg->localPosition()).phi();

        if( fabs(posLocalSeg.x()-ch.x)<dXcut && nHitsX > 0) {
          bool found = false;
          for( auto prev_phi : nsegs_phi_temp) {
            if( fabs(prev_phi.first - phi) < 0.0002 && fabs(prev_phi.second - nHitsX) <= 2 ) {
              found = true;
              break;
            }
          }
          if( !found ) {
            nsegs_phi_temp.push_back( make_pair(phi, nHitsX) );
            if(verbose)  std::cout << "\t\t new segment:" << posLocalSeg << std::endl;
          }
        }
      }

      int nsegs_temp = (int)nsegs_phi_temp.size();
      if(nsegs_temp>0)  stations[ch.station()-1] += nsegs_temp;

      //--- subtract best matched segment from given muon
      for(std::vector<reco::MuonSegmentMatch>::const_iterator matseg = ch.segmentMatches.begin(); matseg != ch.segmentMatches.end(); matseg++) {
        if(!(fabs(matseg->x-ch.x)<dXcut))  continue;
        if( matseg->isMask(reco::MuonSegmentMatch::BestInChamberByDR) && matseg->isMask(reco::MuonSegmentMatch::BelongsToTrackByDR) ) {
          removed[ch.station()-1]++;
          if(verbose)  std::cout << "\t\t   arb segment: " << "(" << matseg->x << ", " << matseg->y << ", 0)" << std::endl;
        }
      }

    }

    for(int i=0; i<4; ++i) {
      if(verbose)  std::cout << "\t\t\t # ext segments in station " << i << ": "
                             << stations[i] << "-" << removed[i] << "=" << (stations[i] - removed[i]) << std::endl;
      stations[i] = stations[i] - removed[i];
    }
    return stations;
  }
  else
    return { -1, -1, -1, -1 };

}

void CustoTnPLeptonProducer::embedShowerInfo(const edm::Event& event, pat::Muon* new_mu, reco::MuonRef MuRef,
                                             const edm::ESHandle<DTGeometry> & dtGeom,
                                             const edm::ESHandle<CSCGeometry> & cscGeom,
                                             bool verbose = false) {
  // edm::Handle<edm::ValueMap<reco::MuonShower> > muonShowerInformationValueMap;
  // event.getByLabel(muonshower_src, muonShowerInformationValueMap);
  // reco::MuonShower muonShowerInformation = (*muonShowerInformationValueMap)[MuRef];

  if(verbose)  std::cout << "\n ** New muon " << new_mu->pt() << ", " << new_mu->eta() << ", " << new_mu->phi() << " **" << std::endl;

  std::vector<int> vec_dummy = {-999, -999, -999, -999};
  std::vector<int> vec_DTdigis  = hasRAW ? countDTdigis(  event, MuRef, dtGeom,  verbose) : vec_dummy;
  std::vector<int> vec_CSCdigis = hasRAW ? countCSCdigis( event, MuRef, cscGeom, verbose) : vec_dummy;
  std::vector<int> vec_DTsegs   = countDTsegs(   event, MuRef,          verbose);
  std::vector<int> vec_CSCsegs  = countCSCsegs(  event, MuRef, cscGeom, verbose);

  for(int i=0; i<4; ++i) {
    if( (hasRAW && (vec_DTdigis[i] < 0 || vec_CSCdigis[i] < 0)) || (vec_DTsegs[i] < 0  || vec_CSCsegs[i] < 0) ) {
      edm::LogError("CustoTnPLeptonProducer::embedShowerInfo") << "Shower variables are not properly counted!!!"
                                                               << "\n\t vec_DTdigis[i]:  " << vec_DTdigis[i]
                                                               << "\n\t vec_CSCdigis[i]: " << vec_CSCdigis[i]
                                                               << "\n\t vec_DTsegs[i]:   " << vec_DTsegs[i]
                                                               << "\n\t vec_CSCsegs[i]:  " << vec_CSCsegs[i];
      break;
    }
    else {
      std::string var_digis_DT  = "nDigisDT"+std::to_string( int(i+1) );
      std::string var_digis_CSC = "nDigisCSC"+std::to_string( int(i+1) );
      std::string var_segs_DT   = "nSegsDT"+std::to_string( int(i+1) );
      std::string var_segs_CSC  = "nSegsCSC"+std::to_string( int(i+1) );

      if(hasRAW)  new_mu->addUserInt(var_digis_DT,  vec_DTdigis[i]);
      if(hasRAW)  new_mu->addUserInt(var_digis_CSC, vec_CSCdigis[i]);
      new_mu->addUserInt(var_segs_DT,   vec_DTsegs[i]);
      new_mu->addUserInt(var_segs_CSC,  vec_CSCsegs[i]);

      if(verbose) {
        std::cout << "\n Stored shower variables:" << endl;
        std::cout << "\t" << var_digis_DT << ":  " << vec_DTdigis[i] << endl;
        std::cout << "\t" << var_digis_CSC << ": " << vec_CSCdigis[i] << endl;
        std::cout << "\t" << var_segs_DT << ":   " << vec_DTsegs[i] << endl;
        std::cout << "\t" << var_segs_CSC << ":  " << vec_CSCsegs[i] << endl;
      }
    }
  }
}

std::pair<pat::Muon*,int> CustoTnPLeptonProducer::doLepton( const edm::Event& event, const pat::Muon& mu, const reco::CandidateBaseRef& cand,
                                                            const edm::ESHandle<DTGeometry> & dtGeom,
                                                            const edm::ESHandle<CSCGeometry> & cscGeom )
{
  // Failure is indicated by a null pointer as the first member of the
  // pair.

  // To use one of the refit tracks, we have to have a global muon.
      
  if (!mu.isGlobalMuon())
    return std::make_pair((pat::Muon*)(0), -1);

  if (mu.pt() < 10.)
    return std::make_pair((pat::Muon*)(0), -1);

  // Copy the input muon, and switch its p4/charge/vtx out for that of
  // the selected refit track.
  
  pat::Muon* new_mu = cloneAndSwitchMuonTrack(mu, event);

  if (new_mu == 0){
    return std::make_pair(new_mu, -1);
    //std::cout << "Warning" << std::endl;
  }  

  // Simply store the photon four-vector for now in the muon as a
  // userData.
  if (muon_photon_match_map.isValid()) {
    const reco::CandViewMatchMap& mm = *muon_photon_match_map;
    if (mm.find(cand) != mm.end()) {
      new_mu->addUserData<reco::Particle::LorentzVector>("photon_p4", mm[cand]->p4());
      new_mu->addUserInt("photon_index", mm[cand].key());
    }    
  }

  //--- Trig match
  for(unsigned i_f=0; i_f<trigger_filters.size(); ++i_f)
    embedTriggerMatch(new_mu, trigger_path_names[i_f], vec_L3_muons[i_f], vec_L3_muons_matched[i_f]);

  //--- L1 match
  edm::Handle<l1t::MuonBxCollection> l1_src;
  event.getByToken(l1_src_, l1_src);

  float the_dr  = 999.;
  float the_pt  = -1.;
  float the_eta = -999.;
  float the_phi = -999.;
  int the_q     = -1;
  for(int ibx = l1_src->getFirstBX(); ibx<=l1_src->getLastBX(); ++ibx) {
    if(ibx != 0) continue;
    for(auto it=l1_src->begin(ibx); it!=l1_src->end(ibx); it++) {
      l1t::MuonRef ref_L1Mu(l1_src, distance(l1_src->begin(l1_src->getFirstBX()), it) );
      if( ref_L1Mu->pt() < 22. ) // L1 pT>22
        continue;
      float dr_temp = reco::deltaR(*ref_L1Mu, *new_mu);
      if( dr_temp > 0.3 )
        continue;
      if( dr_temp < the_dr ) {
        the_dr = dr_temp;
        the_pt = ref_L1Mu->pt();
        the_eta = ref_L1Mu->eta();
        the_phi = ref_L1Mu->phi();
        the_q = ref_L1Mu->hwQual();
      }
    }
  }
  if(the_pt > 0) {
    new_mu->addUserFloat("L122MatchPt",     the_pt);
    new_mu->addUserFloat("L122MatchEta",    the_eta);
    new_mu->addUserFloat("L122MatchPhi",    the_phi);
    new_mu->addUserInt("L122MatchQ",        the_q);
  }
  else
    new_mu->addUserFloat("L122MatchPt",     the_pt);

  //~
  embedExpectedMatchedStations(new_mu, 10.);

  //@
  if(isAOD) {
    std::pair<bool, reco::MuonRef> pair_muRef = getMuonRef(event, new_mu);
    if( pair_muRef.first ) {
      embedShowerInfo(event, new_mu, pair_muRef.second, dtGeom, cscGeom, verboseShower);
    }
    else {
      std::cout << "Reco muon ref not found..." << "  Event ID = " << event.id() << endl;
    }
  }

  int isHP = new_mu->innerTrack()->quality(reco::TrackBase::highPurity);
  int isCO = new_mu->innerTrack()->quality(reco::TrackBase::confirmed);
  new_mu->addUserInt("isHighPurity", isHP);
  new_mu->addUserInt("isConfirmed", isCO);

  // Evaluate cuts here with string object selector, and any code that
  // cannot be done in the string object selector (none so far).
  int cutFor = muon_selector(*new_mu) ? 0 : 1;

  return std::make_pair(new_mu, cutFor);
}

template <typename T>
edm::OrphanHandle<std::vector<T> > CustoTnPLeptonProducer::doLeptons( edm::Event& event, const edm::InputTag& src, const edm::InputTag& view_src,
                                                                      const std::string& instance_label,
                                                                      const edm::ESHandle<DTGeometry> & dtGeom,
                                                                      const edm::ESHandle<CSCGeometry> & cscGeom )
{
  typedef std::vector<T> TCollection;
  edm::Handle<TCollection> leptons; 
  event.getByLabel(src, leptons); 

  static std::map<std::string, bool> warned;
  if (!leptons.isValid()) {
    if (!warned[instance_label]) {
      edm::LogWarning("LeptonsNotFound") << src << " for " << instance_label << " not found, not producing anything -- not warning any more either.";
      warned[instance_label] = true;
    }
    return edm::OrphanHandle<std::vector<T> >();
  }

  edm::Handle<reco::CandidateView> lepton_view;
  event.getByLabel(view_src, lepton_view);

  // std::auto_ptr<TCollection> new_leptons(new TCollection);  // 80X
  std::unique_ptr<TCollection> new_leptons(new TCollection);  // 94X

  for (size_t i = 0; i < leptons->size(); ++i) {
    std::pair<T*,int> res = doLepton(event, leptons->at(i), lepton_view->refAt(i), dtGeom, cscGeom);
    if (res.first == 0)
      continue;
    res.first->addUserInt("cutFor", res.second);
    new_leptons->push_back(*res.first);
    delete res.first;
  }

  // return event.put(new_leptons, instance_label);  // 80X
  return event.put(std::move(new_leptons), instance_label);  // 94X
}

void CustoTnPLeptonProducer::produce(edm::Event& event, const edm::EventSetup& setup) {

  // -- Muon system geometry -- //
  edm::ESHandle<DTGeometry>  dtGeom;
  edm::ESHandle<CSCGeometry> cscGeom;

  setup.get<MuonGeometryRecord>().get(dtGeom);
  setup.get<MuonGeometryRecord>().get(cscGeom);


  // Grab the match map between PAT photons and PAT muons so we can
  // embed the photon candidates later.
  //std::cout << event.id() << std::endl;
  event.getByLabel(muon_photon_match_src, muon_photon_match_map);
  static bool warned = false;
  if (!warned && !muon_photon_match_map.isValid()) {
    edm::LogWarning("PhotonsNotFound") << muon_photon_match_src << " for photons not found, not trying to embed their matches to muons -- not warning any more either.";
    warned = true;
  }

  // Store the L3 muons for trigger match embedding, and clear the
  // vector of matched flags that indicate whether the particular L3
  // muon has been used in a match already. This means matching
  // ambiguities are resolved by original sort order of the
  // candidates; no attempt is done to find a global best
  // matching. (This is how it was done in our configuration of the
  // PATTrigger matcher previously, so why not.) We do this for both
  // the main path and the pres caled path.
  
  // CustoTnPTriggerPathsAndFilters pandf(event);
  // if (!pandf.valid)
  //   throw cms::Exception("CustoTnPLeptonProducer") << "could not determine the HLT path and filter names for this event\n";

  //-- for HLT match
  edm::Handle<edm::TriggerResults> triggerBits;
  edm::Handle<pat::TriggerObjectStandAloneCollection> trigger_summary_src;
  edm::Handle<pat::PackedTriggerPrescales> triggerPrescales;

  event.getByToken(triggerBits_, triggerBits);
  event.getByToken(trigger_summary_src_, trigger_summary_src);
  event.getByToken(triggerPrescales_, triggerPrescales);

  const edm::TriggerNames &names = event.triggerNames(*triggerBits);

  vec_L3_muons.clear();
  for(unsigned i_f=0; i_f<trigger_filters.size(); ++i_f) {
    vec_L3_muons.push_back({});
    vec_L3_muons.back().clear();
  }

  for(pat::TriggerObjectStandAlone obj : *trigger_summary_src) {
    obj.unpackPathNames(names);
    obj.unpackFilterLabels(event, *triggerBits);  // comment out for 80X
    for (unsigned h = 0; h < obj.filterLabels().size(); ++h) {

      for(unsigned i_f=0; i_f<trigger_filters.size(); ++i_f) {
        if (obj.filterLabels()[h] == trigger_filters[i_f]){ 
          vec_L3_muons[i_f].push_back(obj);
        }
      }

    }
  }

  vec_L3_muons_matched.clear();
  for(unsigned i_f=0; i_f<trigger_filters.size(); ++i_f) {
    vec_L3_muons_matched.push_back({});
    vec_L3_muons_matched.back().clear();
    vec_L3_muons_matched.back().resize(vec_L3_muons[i_f].size(), 0);
  }

  // Using the main choice for momentum assignment, make the primary
  // collection of muons, which will have branch name
  // e.g. leptons:muons.
  muon_track_for_momentum = muon_track_for_momentum_primary;
  edm::OrphanHandle<pat::MuonCollection> muons = doLeptons<pat::Muon>(event, muon_src, muon_view_src, "muons", dtGeom, cscGeom);

  // Now make secondary collections of muons using the momentum
  // assignments specified. They will come out as e.g. leptons:tpfms,
  // leptons:picky, ...
  for (size_t i = 0; i < muon_tracks_for_momentum.size(); ++i) {

    vec_L3_muons_matched.clear();
    for(unsigned i_f=0; i_f<trigger_filters.size(); ++i_f) {
      vec_L3_muons_matched.push_back({});
      vec_L3_muons_matched.back().clear();
      vec_L3_muons_matched.back().resize(vec_L3_muons[i_f].size(), 0);
    }

    muon_track_for_momentum = muon_tracks_for_momentum[i];
    doLeptons<pat::Muon>(event, muon_src, muon_view_src, muon_track_for_momentum, dtGeom, cscGeom);
  }

}

DEFINE_FWK_MODULE(CustoTnPLeptonProducer);
