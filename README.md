# T3MAPS test-beam analysis

### Introduction
This package is designed to analyze data for the T3MAPS chip and the FE-I4 chip 
from the SLAC test beam and correlate hits between the two chips. Several 
classes are included, and a short description of each is provided below.

### Main Classes
  
##### FormatT3MAPS.cxx
  This program is designed to load the T3MAPS data from text file and 
  efficiently convert it into a ROOT TTree. 

  Outline:
  - splits the text file into many smaller parts (SplitT3MAPS)
  - creates a TTree for each of the smaller text files (LoadT3MAPS)
  - combines the TTrees via hadd.

##### LocateT3MAPS.cxx
  This program uses the MapParameters class to find the location in FEI4
  corresponding to T3MAPS.

##### TestBeamOverview.cxx
  This program looks at the test beam data and identifies characteristics for
  defining quality cuts on pixel hits.  

##### TestBeamScanner.cxx
  This program is similar to TestBeamTracks, except it scans the value of the
  map error to see how the efficiency changes with map uncertainty. 

##### TestBeamStudies.cxx
  This program applies quality cuts to the FEI4 and T3MAPS data and then 
  constructs a mapping between hits in the two chips. It can also scan the time
  offset for the two chips in case the relative clock times are unknown. NOTE:
  for the scanning feature, the TimingScan.cxx implementation is preferred. 
  Better to use this with the option "NoScan".

##### TestBeamTracks.cxx
  This program applies quality cuts to the FEI4 and T3MAPS data and then
  computes a track-by-track efficiency measurement based on the map constructed
  in TestBeamStudies.

##### TimingScan.cxx
  This program just runs TestBeamTracks multiple times with different timing
  offsets between the two chips in order to validate the timing. It identifies
  the timing giving the maximum FEI4 efficiency.

### Supporting Classes

##### ChipDimension.cxx
  This is a very basic container that stores the dimensions of the FEI4 and 
  T3MAPS chips. It has methods to check whether hits are inside or outside the 
  chip area.

##### LoadT3MAPS.cxx
  This program is designed to load the T3MAPS history.txt output textfile and 
  produce and save a TTree that is ROOT-readable. 

##### MatchMaker.cxx
  This class is designed to search for matches between hits in FEI4 and T3MAPS.
  It starts by searching for matches between individual pixel hits, then builds
  clusters that are either matched or not matched.

##### MapParameters.cxx
  This program computes a geometrical mapping between the T3MAPS and FEI4 chips.
  It can load previously calculated mapping data. It can also be called during 
  a loop over TTrees to add events, and then create a new mapping. It also 
  provides an interface for accessing map data from other classes.

##### PixelCluster.cxx
  This class stores a list of hits that have been associated as a cluster. It
  it also provides methods useful for merging other hits and clusters into a
  single cluster.

##### PixelHit.cxx
  This class stores the basic information associated with a single pixel hit
  (row, column, whether it is matched). 

##### PlotUtil.cxx
  This class stores plotting utilities for the analysis. It initializes a canvas
  and provides default formatting options for output histograms.

