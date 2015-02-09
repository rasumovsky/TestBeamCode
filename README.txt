T3MAPS TestBeam Code Analysis
Andrew Hard
ahard@cern.ch
February 2015

This package is designed to analyze data for the T3MAPS chip and the FE-I4 chip 
from the SLAC test beam and correlate hits between the two. Several classes are
included, and a short description of each is provided below.

  
TestBeamAnalysis.cxx
  This is the main method for the analysis. All loops over the TTrees take place
  within this main method. It makes calls to the LoadT3MAPS, to get the T3MAPS
  TTree, and it loads the FEI4 TTree. It makes either direct or indirect calls
  to the following classes: ChipDimension, LoadT3MAPS, MatchMaker, ModuleMapping
  PixelCluster, PixelHit.
  
  Outline:
  - LoadT3MAPS and load the FEI4 TTree
  - Initialize several ModuleMapping instances
  - Iterate over the two trees in tandem
  - add data to the ModuleMapping objects
  - finish loop
  - Calculate maps for each ModuleMapping object
  - Average the results and create a new ModuleMapping object, and save
  - Loop over TTrees in tandem again
  - in each event, run the MatchMaker class.
  - be sure FEI4 hits have same event number and are within T3MAPS time.
  - get results...
  

ChipDimension.cxx
  This is a very basic container that stores the dimensions of the FEI4 and 
  T3MAPS chips. It has methods to check whether hits are inside or outside the 
  chip area.

LoadT3MAPS.cxx
  This program is designed to load the T3MAPS history.txt output textfile and 
  produce and save a TTree that is ROOT-readable. 

MatchMaker.cxx
  This class is designed to search for matches between hits in FEI4 and T3MAPS.
  It starts by searching for matches between individual pixel hits, then builds
  clusters that are either matched or not matched.

ModuleMapping.cxx
  This program computes a geometrical mapping between the T3MAPS and FEI4 chips.
  It can load previously calculated mapping data. It can also be called during 
  a loop over TTrees to add events, and then create a new mapping. It also 
  provides an interface for accessing map data from other classes.

  ** Note: methods for calculating uncertainty are based on application of +1
  sigma values of mapping constaints. This should be revisited when real data
  are available and the actual spreads can be determined. 

PixelCluster.cxx
  This class stores a list of hits that have been associated as a cluster. It
  it also provides methods useful for merging other hits and clusters into a
  single cluster.

PixelHit.cxx
  This class stores the basic information associated with a single pixel hit
  (row, column, whether it is matched). 
