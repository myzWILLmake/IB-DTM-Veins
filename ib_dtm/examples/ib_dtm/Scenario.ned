//
// Copyright (C) 2017 Christoph Sommer <sommer@ccs-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

package ib_dtm.examples.ib_dtm;
import org.car2x.veins.modules.mobility.traci.TraCIVehicleInserter;
import org.car2x.veins.subprojects.ib_dtm.RSU;
import org.car2x.veins.subprojects.ib_dtm.RSUInserter;
import org.car2x.veins.subprojects.ib_dtm.IBDTMSession;

network Scenario extends org.car2x.veins.nodes.Scenario
{
    submodules:
        inserter: TraCIVehicleInserter {
            @display("p=400,0;b=0,0");
        }
        rsu[0]: RSU;
        rsuInserter: RSUInserter {
            @display("p=428.064,0;b=0,0");
        }
        ibdtmSession: IBDTMSession {
            @display("p=373.96802,0;");
        }
}
