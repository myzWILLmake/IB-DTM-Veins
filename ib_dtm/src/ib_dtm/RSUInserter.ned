//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package org.car2x.veins.subprojects.ib_dtm;

simple RSUInserter {
	parameters:
	    @class(ib_dtm::RSUInserter);
	    
	    bool isGrid = default(true);
	    int xGridSize = default(3);
	    int yGridSize = default(3);
	    double roadLength = default(100.0);
		int maliciousNum = default(0);
}