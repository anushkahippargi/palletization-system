07.08.2026
-Replace in PalletStatistics: palletId with number of used pallets. Add statistics for the last pallet which will not be full.
-In RowAlgorithm - choose which box size is bettter to use in arrangment. You are not fixed in using width of box for width of pallet and so on.
-Calculate maximum possible box number in each direction of pallet before starting the arrangement, so you will have three "for" cicles for each size. 
-Think about 3D view client app for visualization of your pallet algoritms

31.07.2026
-About class Placement: It will be better to replace x,y and rotation with a full 3D matrix of position and otrientation of lower left corner of the current box according to lower left corner of the pallet.
Also I do not know wy you need  double length and double width of the box for placement. All of this could be replaced by the 3D position matrix.

_About class Statistics: I suppose this class is per algorithm type and should contain information also about number of used pallets and used volume per pallet. So replace area with volume and add number of used pallets.
Because all boxes have to be arranged on pallets to have  "int boxesRemaining"  does not make sense.