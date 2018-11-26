import math
import FWCore.ParameterSet.Config as cms

PI = math.pi

from CustoTnP.Analyzer.hltTriggerMatch_cfi import trigger_match_2018

# -- For both Tag and Probe -- ##
default_cut = 'isGlobalMuon && ' \
              'abs(eta) < 2.4' # && '
              # 'isTrackerMuon && ' \
              # 'isolationR03.sumPt / innerTrack.pt < 0.05 && ' \
              # 'isolationR03.sumPt < 30'

tight_cut = ''  #trigger_match


# -- Customized cuts -- #
tag_custo_cut =   'isGlobalMuon && ' \
                  'pt > 53 && ' \
                  'abs(eta) < 2.4 && ' \
                  'abs(dB) < 0.2 && ' \
                  'isolationR03.sumPt / innerTrack.pt < 0.05 && ' \
                  'isolationR03.sumPt < 30 && ' \
                  'innerTrack.hitPattern.trackerLayersWithMeasurement > 5 && ' \
                  'innerTrack.hitPattern.numberOfValidPixelHits >= 1 && ' \
                  'globalTrack.hitPattern.numberOfValidMuonHits > 0 && ' \
                  'numberOfMatchedStations > 1'
                  # '( (globalTrack.hitPattern.numberOfValidMuonHits > 0) || (tunePMuonBestTrack.hitPattern.numberOfValidMuonHits > 0) ) && ' \
                  # 'isTrackerMuon && ' \
                  # '( ( numberOfMatchedStations>1 ) || ( isTrackerMuon && numberOfMatchedStations==1 && ( userInt("expectedNnumberOfMatchedStations10")<2 || !(stationMask==1 || stationMask==16) || numberOfMatchedRPCLayers>2 ) ) )'

custo_cut_nopt =  'isGlobalMuon && ' \
                  'abs(eta) < 2.4 && ' \
                  'abs(dB) < 0.2 && ' \
                  'innerTrack.hitPattern.trackerLayersWithMeasurement > 5 && ' \
                  'innerTrack.hitPattern.numberOfValidPixelHits >= 1 && ' \
                  '( (globalTrack.hitPattern.numberOfValidMuonHits > 0) || (tunePMuonBestTrack.hitPattern.numberOfValidMuonHits > 0) ) && ' \
                  '( ( numberOfMatchedStations>1 ) || ( isTrackerMuon && numberOfMatchedStations==1 && ( userInt("expectedNnumberOfMatchedStations10")<2 || !(stationMask==1 || stationMask==16) || numberOfMatchedRPCLayers>2 ) ) )'
                  # 'isolationR03.sumPt / innerTrack.pt < 0.05 && ' \
                  # 'isolationR03.sumPt < 30 && ' \

Pair_Cut = ''

# -- For Tag -- #
Tag_cut = tag_custo_cut + ' && ' + trigger_match_2018  # kill Trigger bias with Tag
Tag_dpt_over_pt_max = 0.3
Tag_dz_max = 0.5

# -- For Probe -- #
#--- no pT cut for eff vs pT, and apply pT cut separately
Probe_cut = 'isGlobalMuon && ' \
            'abs(eta) < 2.4 && ' \
            'isolationR03.sumPt / innerTrack.pt < 0.05 && ' \
            'isolationR03.sumPt < 30'
            # 'isTrackerMuon && ' \
Probe_dpt_over_pt_max = 1e9
Probe_dz_max = 1e9
Probe_pt_cut = 53.0

TnP_deltaR_min = 0.4

#--- For efficiency vs nShowers
Probe_veto_other_dphi_min = 0.6

#--- Minium # matched segments in each station
nshowers_threshold_min = 2

# -- For Passing Probe -- #
Passing_probe_cut = custo_cut_nopt
Passing_probe_dpt_over_pt_max = 0.3
Passing_probe_dz_max = 0.5

# -- For comparison probe -- #
Comparison_probe_cut = ''
Comparison_probe_dpt_over_pt_max = 1e9
Comparison_probe_dz_max = 1e9

# -- Binnings (for ID effificney) -- #
width = PI / 20.0
pt_bins   = [0, 20, 25, 30, 40, 50, 60, 120, 200, 400, 1e6]
absp_bins = [0, 1e6]
eta_bins  = [-2.4, -2.1, -1.6, -1.2, -0.9, -0.3, -0.2, 0.0, 0.2, 0.3, 0.9, 1.2, 1.6, 2.1, 2.4]
phi_bins  = [ -20*width, -19*width, -18*width, -17*width, -16*width,
              -15*width, -14*width, -13*width, -12*width, -11*width,
              -10*width, -9*width, -8*width, -7*width, -6*width,
              -5*width, -4*width, -3*width, -2*width, -1*width,
              0*width, 1*width, 2*width, 3*width, 4*width,
              5*width, 6*width, 7*width, 8*width, 9*width,
              10*width, 11*width, 12*width, 13*width, 14*width,
              15*width, 16*width, 17*width, 18*width, 19*width,
              20*width
]
vtx_bins  = [ 2.5, 4.5, 6.5, 8.5, 10.5, 12.5, 14.5, 16.5, 18.5, 20.5,
              22.5, 24.5, 26.5, 28.5, 30.5, 32.5, 34.5, 36.5, 38.5, 40.5,
              42.5, 44.5, 46.5, 48.5, 50.5, 52.5, 54.5, 56.5, 58.5, 60.5,
              62.5, 64.5, 66.5, 68.5, 70.5, 72.5, 74.5, 76.5, 78.5, 80.5,
              82.5, 84.5, 86.5, 88.5, 90.5, 92.5, 94.5, 96.5, 98.5, 100.5
]

allDimuons = cms.EDProducer('CustoTnPCombiner',
                            decay = cms.string('leptons:muons@+ leptons:muons@-'),
                            cut = cms.string(''),
                            loose_cut = cms.string(default_cut),
                            tight_cut = cms.string(tight_cut)
)

dimuons = cms.EDProducer('CustoTnPPairSelector',
                         src = cms.InputTag('allDimuons'),
                         muon_src = cms.InputTag('slimmedMuons'),
                         vertex_src = cms.InputTag('offlineSlimmedPrimaryVertices'),
                         cut = cms.string(Pair_Cut),                                  # simple cuts for dilepton pair, Pair_Cut
                         tag_cut = cms.string(Tag_cut),                               # Tag lepton selection, Tag_cut
                         tag_dpt_over_pt_max = cms.double(Tag_dpt_over_pt_max),       # Tag dpT/pT
                         tag_dz_max = cms.double(Tag_dz_max),                         # Tag dz
                         probe_cut = cms.string(Probe_cut),                           # Probe lepton selection, Probe_cut
                         probe_dpt_over_pt_max = cms.double(Probe_dpt_over_pt_max),   # Probe dpT/pT
                         probe_dz_max = cms.double(Probe_dz_max),                     # Probe dz

                         back_to_back_cos_angle_min = cms.double(-0.9998), # this corresponds to the angle (pi - 0.02) rad = 178.9 deg
                         vertex_chi2_max = cms.double(20),
                         pt_ratio_max = cms.double(3.0),
                         dil_deltaR_min = cms.double(TnP_deltaR_min),  #0.4

                         veto_others_dphi_min = cms.double(Probe_veto_other_dphi_min),  #0.6

                         veto_multi_pair_with_Z = cms.bool(True),

                         samePV = cms.bool( False ),

                         max_candidates = cms.uint32(1),
                         sort_by_pt = cms.bool(True),
                         do_remove_overlap = cms.bool(True),
                         ShutUp = cms.bool(True)  #True
)

HistosForTnP = cms.EDAnalyzer('CustoTnPHistosForTnP',
                               dilepton_src = cms.InputTag('dimuons'),
                               beamspot_src = cms.InputTag('offlineBeamSpot'),
                               vertex_src = cms.InputTag('offlineSlimmedPrimaryVertices'),
                               use_bs_and_pv = cms.bool(True),
                               useMadgraphWeight = cms.bool(True),

                               # bin width = 1, starting from 0
                               pileup_src = cms.InputTag('slimmedAddPileupInfo'),

                               # minBias Xsec = ??? #
                               vec_PileUpWeight = cms.vdouble( 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 ),


                               probe_pt_min = cms.double(Probe_pt_cut),

                               passing_probe_cut = cms.string(Passing_probe_cut),
                               passing_probe_dpt_over_pt_max = cms.double(Passing_probe_dpt_over_pt_max),
                               passing_probe_dz_max = cms.double(Passing_probe_dz_max),

                               comparison_probe_cut = cms.string(Comparison_probe_cut),
                               comparison_probe_dpt_over_pt_max = cms.double(Comparison_probe_dpt_over_pt_max),
                               comparison_probe_dz_max = cms.double(Comparison_probe_dz_max),

                               minMass = cms.double(0),
                               maxMass = cms.double(10000),
                                                                                        #probe pT cut
                               vec_PtBins = cms.vdouble( pt_bins ),
                               vec_AbsPBins = cms.vdouble( absp_bins ),
                               vec_EtaBins = cms.vdouble( eta_bins ),
                               vec_PhiBins = cms.vdouble( phi_bins ),
                               vec_VtxBins = cms.vdouble( vtx_bins ),

                               ShutUp = cms.bool(True)  #True
)




#--- tags for AOD
  # reco_muon_src = cms.InputTag('muons'),
  # muonshower_src = cms.InputTag('muons', 'muonShowerInformation', 'RECO'),
  # dtseg_src = cms.InputTag('dt4DSegments'),
  # cscseg_src = cms.InputTag('cscSegments'),

  # vertex_src = cms.InputTag('offlinePrimaryVertices'),

HistosForTnPAOD = cms.EDAnalyzer('CustoTnPHistosForTnP_AOD',
                               dilepton_src = cms.InputTag('dimuonsAOD'),
                               beamspot_src = cms.InputTag('offlineBeamSpot'),
                               vertex_src = cms.InputTag('offlinePrimaryVertices'),
                               use_bs_and_pv = cms.bool(True),
                               useMadgraphWeight = cms.bool(True),

                               # bin width = 1, starting from 0
                               pileup_src = cms.InputTag('addPileupInfo'),

                               # minBias Xsec = ??? #
                               vec_PileUpWeight = cms.vdouble( 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 ),


                               probe_pt_min = cms.double(Probe_pt_cut),

                               passing_probe_cut = cms.string(Passing_probe_cut),
                               passing_probe_dpt_over_pt_max = cms.double(Passing_probe_dpt_over_pt_max),
                               passing_probe_dz_max = cms.double(Passing_probe_dz_max),

                               nshowers_threshold_min = cms.int32(nshowers_threshold_min),

                               comparison_probe_cut = cms.string(Comparison_probe_cut),
                               comparison_probe_dpt_over_pt_max = cms.double(Comparison_probe_dpt_over_pt_max),
                               comparison_probe_dz_max = cms.double(Comparison_probe_dz_max),

                               minMass = cms.double(0),
                               maxMass = cms.double(10000),
                                                                                        #probe pT cut
                               vec_PtBins = cms.vdouble( pt_bins ),
                               vec_AbsPBins = cms.vdouble( absp_bins ),
                               vec_EtaBins = cms.vdouble( eta_bins ),
                               vec_PhiBins = cms.vdouble( phi_bins ),
                               vec_VtxBins = cms.vdouble( vtx_bins ),

                               vec_NSegsBins = cms.vdouble( -0.5, 0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5, 10.5, 13.5, 20.5, 40.5, 100.5, 500.5 ),
                               vec_NShowersBins = cms.vdouble( -0.5, 0.5, 1.5, 2.5, 3.5, 4.5 ),

                               ShutUp = cms.bool(True)  #True
)

