/*    SDRPortal - A generic web-based interface for SDRs
 *    Copyright (C) 2013 Ben Kempke (bpkempke@umich.edu)
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PORTAL_PROFILE_H
#define PORTAL_PROFILE_H

#include <vector>
#include <string>

//Function prototypes
bool isValidProfile(std::string profile_name);

//Class definitions
class portalProfile{
public:
	portalProfile(std::string profile_name);
	~portalProfile();
	bool acceptsCommand(std::string command_name);
	std::string sendCommand(std::string command);
private:
	std::vector<std::string> commands;
	pid_t sigproc_pid;
};

#endif
