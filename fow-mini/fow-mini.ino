/*
    Copyright (c) 2017 Declan Freeman-Gleason. All rights reserved.

    This file is part of Ferries Over Winslow.

    Ferries Over Winslow is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Ferries Over Winslow is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this Ferries Over Winslow.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ConnectionManager.h"
#include "DataManager.h"
#include "MotorManager.h"
#include "LightManager.h"

// null references on the heap (these should never be destroyed)
ConnectionManager* conn = NULL;
DataManager* data = NULL;
MotorManager* motors = NULL;
LightManager* lights = NULL;

void setup() {
  conn = new ConnectionManager("fow-mini");
  data = new DataManager(conn);
  lights = new LightManager();
  motors = new MotorManager(MotorManager::Modes::DOUBLE_CLOCK, data, lights);
}

void loop() {
  conn->update();
  if (conn->ready()) {
    lights->setAllModesOnce(FerryLights::Modes::RUNNING);
    data->update();
    motors->update();
  } else {
    lights->setAllModesOnce(FerryLights::Modes::DISCONNECTED);
    motors->calibrate();
  }
  lights->update();
}
