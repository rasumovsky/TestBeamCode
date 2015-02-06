T3MAPS TestBeam Code Analysis
Andrew Hard
ahard@cern.ch
February 2015

This package is designed to analyze data for the T3MAPS chip and the FE-I4 chip 
from the SLAC test beam and correlate hits between the two. Several classes are
included, and a short description of each is provided below.


LoadT3MAPS.cxx
  This program is designed to load the T3MAPS history.txt output textfile and 
  produce a TTree that is ROOT-readable. 
  
	public methods:
	    LoadT3MAPS( string inFileName, string outFileName ); // constructor
	    int getNEvents(); // returns the number of integration periods
	    TTree* getTree(); // returns pointer to the TTree storing data.


ModuleMapping.cxx
  This program computes a geometrical mapping between the T3MAPS and FEI4 chips.
  It can be run either using input TTree data files, or it can load previously
  calculated mapping data. It also provides a tool for applying the map that can
  be accessed in other main programs.

HitMatching.cxx
  This class is designed to search for matches between hits in FEI4 and T3MAPS.
  It will start by searching for matches between individual pixels, and then 
  work up to cluster-level matching.
  
TestBeamAnalysis.cxx
  This is the main method for the analysis. It makes calls to the LoadT3MAPS, 
  ModuleMapping, and HitMatching classes.
