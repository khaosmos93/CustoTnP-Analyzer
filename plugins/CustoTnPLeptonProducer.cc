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

#include "Geometry/CSCGeometry/interface/CSCLayer.h"
#include "DataFormats/MuonDetId/interface/CSCDetId.h"
#include "DataFormats/CSCRecHit/interface/CSCRecHit2D.h"
#include "DataFormats/CSCRecHit/interface/CSCRangeMapAccessor.h"

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

  std::pair<pat::Muon*,     int> doLepton(const edm::Event&, const pat::Muon&,     const reco::CandidateBaseRef&);

  template <typename T> edm::OrphanHandle<std::vector<T> > doLeptons(edm::Event&, const edm::InputTag&, const edm::InputTag&, const std::string&);

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
    triggerPrescales_(consumes<pat::PackedTriggerPrescales>(cfg.getParameter<edm::InputTag>("prescales")))
{
  consumes<edm::View<reco::Candidate>>(muon_view_src);
  consumes<pat::MuonCollection>(muon_src);
  consumes<reco::CandViewMatchMap >(muon_photon_match_src);

  if (cfg.existsAs<std::vector<std::string> >("muon_tracks_for_momentum"))
    muon_tracks_for_momentum = cfg.getParameter<std::vector<std::string> >("muon_tracks_for_momentum");

  if (muon_tracks_for_momentum.size())
    for (size_t i = 0; i < muon_tracks_for_momentum.size(); ++i)
      produces<pat::MuonCollection>(muon_tracks_for_momentum[i]);

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

// void CustoTnPLeptonProducer::embedTriggerMatch_or(pat::Muon* new_mu, const std::string& ex, const pat::TriggerObjectStandAloneCollection& L3, const pat::TriggerObjectStandAloneCollection& L3_or, std::vector<int>& L3_matched, std::vector<int>& L3_matched_2) {
//     int best_1 = -1;
//     int best_2 = -1;
//     float defaultpTvalue = 20.;
//     float best_dR_1 = trigger_match_max_dR;
//     float best_dR_2 = trigger_match_max_dR;
//     //    std::cout<<"embedded trigger function"<<std::endl;
//     for (size_t i = 0; i < L3.size(); ++i) {
//         // Skip those already used.
//         if (L3_matched[i])
//         continue;
        
//         const float dR = reco::deltaR(L3[i], *new_mu);
//         if (dR < best_dR_1) {
//             best_1 = int(i);
//             best_dR_1 = dR;
//         }
//     }
//     //std::cout<<"filtro2"<<std::endl;
//     for (size_t i = 0; i < L3_or.size(); ++i) {
//         // Skip those already used.
//         if (L3_matched_2[i])
//         continue;
        
//         const float dR = reco::deltaR(L3_or[i], *new_mu);
//         if (dR < best_dR_2) {
//             best_2 = int(i);
//             best_dR_2 = dR;
//         }
//     }
//     //    std::cout<<" best1 "<<best_1<<" best2 "<<best_2<<" best_dR_1 "<<best_dR_1<<" best_dR_2 "<<best_dR_2<<std::endl;
//     //    if (best_1 < 0 && best_2 < 0 )
//     //    return;
    
//     if (best_2 <0 && best_1 >= 0){
//         const pat::TriggerObjectStandAlone& L3_mu = L3[best_1];
//         L3_matched[best_1] = 1;
        
//         int id = L3_mu.pdgId();
//         new_mu->addUserFloat(ex + "TriggerMatchCharge", -id/abs(id));
//         new_mu->addUserFloat(ex + "TriggerMatchPt",     L3_mu.pt());
//         new_mu->addUserFloat(ex + "TriggerMatchEta",    L3_mu.eta());
//         new_mu->addUserFloat(ex + "TriggerMatchPhi",    L3_mu.phi());
// //        std::cout<<"ex + trigger match = "<<ex<<"...TriggerMatchPt = "<<L3_mu.pt()<<std::endl;
// //        std::cout<<"TriggerMatchPt muon producer = "<<new_mu->hasUserFloat(ex + "TriggerMatchPt")<<std::endl;
//     }
//     else if (best_1 <0 && best_2 >= 0){
//         const pat::TriggerObjectStandAlone& L3_mu = L3_or[best_2];
//         L3_matched_2[best_2] = 1;
        
//         int id = L3_mu.pdgId();
//         new_mu->addUserFloat(ex + "TriggerMatchCharge", -id/abs(id));
//         new_mu->addUserFloat(ex + "TriggerMatchPt",     L3_mu.pt());
//         new_mu->addUserFloat(ex + "TriggerMatchEta",    L3_mu.eta());
//         new_mu->addUserFloat(ex + "TriggerMatchPhi",    L3_mu.phi());
//     }
//     else if (best_1 >=0 && best_2 >=0 && best_dR_1 <= best_dR_2){
//         const pat::TriggerObjectStandAlone& L3_mu = L3[best_1];
//         L3_matched[best_1] = 1;
        
//         int id = L3_mu.pdgId();
//         new_mu->addUserFloat(ex + "TriggerMatchCharge", -id/abs(id));
//         new_mu->addUserFloat(ex + "TriggerMatchPt",     L3_mu.pt());
//         new_mu->addUserFloat(ex + "TriggerMatchEta",    L3_mu.eta());
//         new_mu->addUserFloat(ex + "TriggerMatchPhi",    L3_mu.phi());
//     }
//     else if (best_1 >=0 && best_2 >=0 && best_dR_1 > best_dR_2){
//         const pat::TriggerObjectStandAlone& L3_mu = L3_or[best_2];
//         L3_matched_2[best_2] = 1;
        
//         int id = L3_mu.pdgId();
//         new_mu->addUserFloat(ex + "TriggerMatchCharge", -id/abs(id));
//         new_mu->addUserFloat(ex + "TriggerMatchPt",     L3_mu.pt());
//         new_mu->addUserFloat(ex + "TriggerMatchEta",    L3_mu.eta());
//         new_mu->addUserFloat(ex + "TriggerMatchPhi",    L3_mu.phi());
//     }
//     else {
//         // std::cout<<"embedded trigger function 4"<<std::endl;
//         new_mu->addUserFloat(ex + "TriggerMatchPt",    defaultpTvalue);
//         //std::cout<<"TriggerMatchPt muon producer own = "<<new_mu->hasUserFloat(ex + "TriggerMatchPt")<<std::endl;
//     }
// }

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


std::pair<pat::Muon*,int> CustoTnPLeptonProducer::doLepton(const edm::Event& event, const pat::Muon& mu, const reco::CandidateBaseRef& cand) {
  // Failure is indicated by a null pointer as the first member of the
  // pair.

  // To use one of the refit tracks, we have to have a global muon.
      
  if (!mu.isGlobalMuon())
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

  //~
  // std::cout << "Muon pT= " << new_mu->pt() << "\teta= " << new_mu->eta() << "\tphi= " << new_mu->phi() << std::endl;
  embedExpectedMatchedStations(new_mu, 5.);
  embedExpectedMatchedStations(new_mu, 10.);
  embedExpectedMatchedStations(new_mu, 15.);
  embedExpectedMatchedStations(new_mu, 20.);
  // std::cout << "\texpectedNnumberOfMatchedStations: " << new_mu->userInt("expectedNnumberOfMatchedStations") << std::endl;

  // Evaluate cuts here with string object selector, and any code that
  // cannot be done in the string object selector (none so far).
  int cutFor = muon_selector(*new_mu) ? 0 : 1;

  return std::make_pair(new_mu, cutFor);
}

template <typename T>
edm::OrphanHandle<std::vector<T> > CustoTnPLeptonProducer::doLeptons(edm::Event& event, const edm::InputTag& src, const edm::InputTag& view_src, const std::string& instance_label) {
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

  std::auto_ptr<TCollection> new_leptons(new TCollection);

  for (size_t i = 0; i < leptons->size(); ++i) {
    std::pair<T*,int> res = doLepton(event, leptons->at(i), lepton_view->refAt(i));
    if (res.first == 0)
      continue;
    res.first->addUserInt("cutFor", res.second);
    new_leptons->push_back(*res.first);
    delete res.first;
  }

  return event.put(new_leptons, instance_label);
}


void CustoTnPLeptonProducer::produce(edm::Event& event, const edm::EventSetup& setup) {
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
  
  CustoTnPTriggerPathsAndFilters pandf(event);
  if (!pandf.valid)
    throw cms::Exception("CustoTnPLeptonProducer") << "could not determine the HLT path and filter names for this event\n";

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
    // obj.unpackFilterLabels(event, *triggerBits); // for 2017~
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
  edm::OrphanHandle<pat::MuonCollection> muons = doLeptons<pat::Muon>(event, muon_src, muon_view_src, "muons");

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
    doLeptons<pat::Muon>(event, muon_src, muon_view_src, muon_track_for_momentum);
  }

}

DEFINE_FWK_MODULE(CustoTnPLeptonProducer);
