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

simple IBDTMSession {
	parameters:
	    @class(ib_dtm::IBDTMSession);

	    int committeeSize = default(16);

		double initEffectiveStake = default(48);
		double effectiveStakeUpperBound = default(72);
		double effectiveStakeLowerBound = default(24);
		int initITSstake = default(512);
		int numVehicles = default(8000);

		double baseReward = default(0.39774758);
		double penaltyFactor = default(16);

		double epochTickInterval = default(5);
		int epochSlots = default(50);
		int traceBackEpoches = default(150);
		bool enableIBDTM = default(true);

	gates:
		input rsuInputs[];
		output rsuOutputs[];
}