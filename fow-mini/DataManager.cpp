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

#include "DataManager.h"

DataManager::DataManager(ConnectionManager* conn) : connection(conn) {
  Serial.begin(115200);
  delay(10);
}

void DataManager::update() {
  if (millis() - lastUpdated < refreshRate) {
    return;
  }

  Serial.println("Requesting fresh data from the server...");

  std::vector<String> compositeResponse;
  compositeResponse = split(connection->get(), ':');
  lastUpdated = millis();

  char *end;

  endDurationAhead = strtoul(compositeResponse.back().c_str(), &end, 10);
  if (endDurationAhead == -1) {
    // -1 means that the data on the server is stale, so we set it up to check back soon
    lastUpdated += refreshRate * 0.8;
    return;
  }
  progresses.clear();

  compositeResponse.pop_back(); // All valid responses end with a ":" so it splits that, and we want to remove it

  if (compositeResponse.size() != 2)
    Serial.printf("Suspect size of %i when splitting composite response.\n", compositeResponse.size());

  for (auto const& responseString : compositeResponse) {
    std::vector<String> response;
    response = split(responseString, ',');
    if (response.size() != 3) {
      Serial.printf("Invalid size of %i when splitting non-composite response.\n", response.size());
      return;
    }

    Progress progress;

    progress.start = atof(response.at(0).c_str());
    progress.end = atof(response.at(1).c_str());
    // startTimeOffset is how long ago (in milliseconds) the first number was valid
    progress.startTimeOffset = strtoul(response.at(2).c_str(), &end, 10);
    progresses.push_back(progress);
  }
}

double DataManager::getProgress(int i) {
  if (i >= progresses.size()) {
    return 0.0;
  }
  Progress progress = progresses.at(i);
  if (progress.startTimeOffset == 0)
    return progress.start; // If the offset is zero, then the ferry is docked
  long double percentPerMsec = (progress.end - progress.start) / endDurationAhead;
  long double result = ((millis() - (lastUpdated - progress.startTimeOffset)) * percentPerMsec) + progress.start;
  return result;
}

std::vector<String> DataManager::split(const String &text, char sep) {
  std::vector<String> tokens;
  size_t start = 0, end = 0;
  while ((end = text.indexOf(sep, start)) != -1) {
    if (end != start) {
      tokens.push_back(text.substring(start, end));
    }
    start = end + 1;
  }
  if (end != start) {
    tokens.push_back(text.substring(start));
  }
  return tokens;
}
