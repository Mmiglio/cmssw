from __future__ import print_function
import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
import os, sys

options = VarParsing.VarParsing ('analysis')

options.register ('runNumber',
                  370169, # default value
                  VarParsing.VarParsing.multiplicity.singleton,
                  VarParsing.VarParsing.varType.int,
                  "Run Number")

options.register ('daqSourceMode',
                  'ScoutingRun3', # default value
                  VarParsing.VarParsing.multiplicity.singleton,
                  VarParsing.VarParsing.varType.string,
                  "DAQ source data mode")

options.register ('buBaseDir',
                  '/dev/shm', # default value
                  VarParsing.VarParsing.multiplicity.singleton,
                  VarParsing.VarParsing.varType.string,
                  "BU base directory")

options.register ('fuBaseDir',
                  '/tmp/', # default value
                  VarParsing.VarParsing.multiplicity.singleton,
                  VarParsing.VarParsing.varType.string,
                  "BU base directory")

options.register ('fffBaseDir',
                  '/dev/shm', # default value
                  VarParsing.VarParsing.multiplicity.singleton,
                  VarParsing.VarParsing.varType.string,
                  "FFF base directory")

options.register ('numThreads',
                  1, # default value
                  VarParsing.VarParsing.multiplicity.singleton,
                  VarParsing.VarParsing.varType.int,
                  "Number of CMSSW threads")

options.register ('numFwkStreams',
                  1, # default value
                  VarParsing.VarParsing.multiplicity.singleton,
                  VarParsing.VarParsing.varType.int,
                  "Number of CMSSW streams")

options.register ('lumiNumber',
                  1, # default value
                  VarParsing.VarParsing.multiplicity.singleton,
                  VarParsing.VarParsing.varType.int,
                  "Lumisection number to process")

options.register ('subLumiNumber',
                  0, # default value
                  VarParsing.VarParsing.multiplicity.singleton,
                  VarParsing.VarParsing.varType.int,
                  "Sub lumisection number to process")

options.register ('outDataDir',
                  'outData', # default value
                  VarParsing.VarParsing.multiplicity.singleton,
                  VarParsing.VarParsing.varType.string,
                  "Output data direcotry for the merged files")

options.register ('outModule',
                  'root',
                  VarParsing.VarParsing.multiplicity.singleton,
                  VarParsing.VarParsing.varType.string,
                  "Output Module [root/streamer]")

options.parseArguments()

cmsswbase = os.path.expandvars("$CMSSW_BASE/")

process = cms.Process("SCPU")
process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(-1)
)

process.options = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(True),
    numberOfThreads = cms.untracked.uint32(options.numThreads),
    numberOfStreams = cms.untracked.uint32(options.numFwkStreams),
    numberOfConcurrentLuminosityBlocks = cms.untracked.uint32(1) # ShmStreamConsumer requires synchronization at LuminosityBlock boundaries
)
process.MessageLogger = cms.Service("MessageLogger",
    cout = cms.untracked.PSet(
        threshold = cms.untracked.string( "WARNING" )
    ),
    destinations = cms.untracked.vstring( 'cout' ),
)

process.FastMonitoringService = cms.Service("FastMonitoringService",
    sleepTime = cms.untracked.int32(1)
)

process.Timing = cms.Service("Timing",
  summaryOnly = cms.untracked.bool(True),
  useJobReport = cms.untracked.bool(False)
)

process.load( "HLTrigger.Timer.FastTimerService_cfi" )
process.FastTimerService.writeJSONSummary = cms.untracked.bool(True)
process.FastTimerService.jsonFileName = cms.untracked.string('resources.json')

process.EvFDaqDirector = cms.Service("EvFDaqDirector",
    useFileBroker = cms.untracked.bool(False),
    buBaseDirsAll = cms.untracked.vstring(
        options.buBaseDir
    ),
    buBaseDirsNumStreams = cms.untracked.vint32(
        2
    ),
    fileBrokerHostFromCfg = cms.untracked.bool(True),
    fileBrokerHost = cms.untracked.string("htcp40.cern.ch"),
    runNumber = cms.untracked.uint32(options.runNumber),
    baseDir = cms.untracked.string(options.fffBaseDir+"/"+options.fuBaseDir),
    buBaseDir = cms.untracked.string(options.fffBaseDir+"/"+options.buBaseDir),
    directorIsBU = cms.untracked.bool(False),
)

try:
  os.makedirs(options.fffBaseDir+"/"+options.fuBaseDir+"/run"+str(options.runNumber).zfill(6))
except Exception as ex:
  print(str(ex))
  pass

ram_dir_path=options.buBaseDir+"/run"+str(options.runNumber).zfill(6)+"/"
# flist  = [ ram_dir_path + "run" + str(options.runNumber) + "_ls" + str(options.lumiNumber).zfill(4) + "_index" + str(idx).zfill(6) + ".raw" for idx in range(8*options.subLumiNumber,8*(options.subLumiNumber+1)) ]
flist  = [ ram_dir_path + "run" + str(options.runNumber) + "_ls" + str(options.lumiNumber).zfill(4) + "_index" + str(idx).zfill(6) + ".raw" for idx in range(1*options.subLumiNumber,1*(options.subLumiNumber+1)) ]
print(flist)

process.source = cms.Source("DAQSource",
    testing = cms.untracked.bool(True),
    dataMode = cms.untracked.string(options.daqSourceMode),
    verifyChecksum = cms.untracked.bool(False),
    useL1EventID = cms.untracked.bool(False),
    eventChunkBlock = cms.untracked.uint32(2 * 1024),
    eventChunkSize = cms.untracked.uint32(3 * 1024),
    maxChunkSize = cms.untracked.uint32(10 * 1024),
    numBuffers = cms.untracked.uint32(2),
    maxBufferedFiles = cms.untracked.uint32(2),
    fileListMode = cms.untracked.bool(True),
    fileNames = cms.untracked.vstring(*flist)
)

process.datasets = cms.PSet(
    DZeroBias = cms.vstring( 'pZeroBias' ),
    DTest1    = cms.vstring( 'pTest1' )
)

process.streams = cms.PSet(
    SZeroBias = cms.vstring( 'DZeroBias' ),
    STest1    = cms.vstring( 'DTest1' )
)

process.PrescaleService = cms.Service( "PrescaleService",
    forceDefault = cms.bool( True ),
    prescaleTable = cms.VPSet(
      cms.PSet(  pathName = cms.string( "pZeroBias" ),
        prescales = cms.vuint32( 10 )
      ),
      cms.PSet(  pathName = cms.string( "pTest1" ),
        prescales = cms.vuint32( 100 )
      )

   ),
    lvl1DefaultLabel = cms.string( "pre1" ),
    lvl1Labels = cms.vstring( "pre1" )
)

## test plugins
process.GmtUnpacker = cms.EDProducer('ScGMTRawToDigi',
  srcInputTag = cms.InputTag('rawDataCollector'),
  debug=cms.untracked.bool(False)
)
process.CaloUnpacker = cms.EDProducer('ScCaloRawToDigi',
  srcInputTag = cms.InputTag('rawDataCollector'),
  debug=cms.untracked.bool(False)
)

process.preZeroBias = cms.EDFilter( "HLTPrescaler",
    L1GtReadoutRecordTag = cms.InputTag( "rawToDigi" ),
    offset = cms.uint32(0)
)

process.preTest1 = cms.EDFilter( "HLTPrescaler",
    L1GtReadoutRecordTag = cms.InputTag( "rawToDigi" ),
    offset = cms.uint32(0)
)


outFilePath = options.outDataDir + '/' + "run" + str(options.runNumber) + '/'
if not os.path.exists(outFilePath):
    os.makedirs(outFilePath)
ofname = outFilePath + "run" + str(options.runNumber) + "_ls" + str(options.lumiNumber).zfill(4) + "_index" + str(options.subLumiNumber).zfill(6) + ".root"


if options.outModule == "root":
    process.hltOutputSZeroBias = cms.OutputModule("PoolOutputModule",
        fileName = cms.untracked.string('file:' + ofname),
        SelectEvents = cms.untracked.PSet(
            SelectEvents = cms.vstring( 'pZeroBias' )
        ),
        outputCommands = cms.untracked.vstring(
            'drop *',
            'keep *_GmtUnpacker_*_*',
            'keep *_CaloUnpacker_*_*',
            'keep edmTriggerResults_*_*_*',
            'keep triggerTriggerEvent_*_*_*'
            ),
        )

    process.hltOutputSTest1 = cms.OutputModule("PoolOutputModule",
        fileName = cms.untracked.string('file:' + ofname + 'Test1'),
        SelectEvents = cms.untracked.PSet(
            SelectEvents = cms.vstring( 'pTest1' )
        ),
        outputCommands = cms.untracked.vstring(
            'drop *',
            'keep *_GmtUnpacker_*_*',
            'keep *_CaloUnpacker_*_*',
            'keep edmTriggerResults_*_*_*',
            'keep triggerTriggerEvent_*_*_*'
            ),
        )


elif options.outModule == "streamer":
    process.hltOutputSZeroBias = cms.OutputModule( "GlobalEvFOutputModule",
        SelectEvents = cms.untracked.PSet(
            SelectEvents = cms.vstring( 'pZeroBias' )
        ),
        outputCommands = cms.untracked.vstring(
            'drop *',
            'keep *_GmtUnpacker_*_*',
            'keep *_CaloUnpacker_*_*',
            'keep edmTriggerResults_*_*_*',
            'keep triggerTriggerEvent_*_*_*'
        ),
        compression_algorithm = cms.untracked.string("ZSTD")
    )
    process.hltOutputSTest1 = cms.OutputModule( "GlobalEvFOutputModule",
        SelectEvents = cms.untracked.PSet(
            SelectEvents = cms.vstring( 'pTest1' )
        ),
        outputCommands = cms.untracked.vstring(
            'drop *',
            'keep *_GmtUnpacker_*_*',
            'keep *_CaloUnpacker_*_*',
            'keep edmTriggerResults_*_*_*',
            'keep triggerTriggerEvent_*_*_*'
        ),
        compression_algorithm = cms.untracked.string("ZSTD")
    )

else:
    print('invalid output module')
    sys.exit(1)

process.rawToDigi = cms.Sequence(process.GmtUnpacker + process.CaloUnpacker)

process.pZeroBias = cms.Path(process.rawToDigi + process.preZeroBias)  
process.pTest1 = cms.Path(process.rawToDigi + process.preTest1)  


process.ep = cms.EndPath(
    process.hltOutputSZeroBias + process.hltOutputSTest1
)
