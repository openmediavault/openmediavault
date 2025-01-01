#!/usr/bin/env dash
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2025 Volker Theile
#
# OpenMediaVault is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# OpenMediaVault is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with OpenMediaVault. If not, see <http://www.gnu.org/licenses/>.

set -e

########################################################################
# Update the configuration.
# <config>
#   <services>
#		<ftp>
#			<enable>0</enable>
#			<port>21</port>
#			<maxclients>5</maxclients>
#			<maxconnectionsperhost>2</maxconnectionsperhost>
#			<maxloginattempts>1</maxloginattempts>
#			<timeoutidle>1200</timeoutidle>
#			<displaylogin></displaylogin>
#			<allowforeignaddress>0</allowforeignaddress>
#			<allowrestart>0</allowrestart>
#			<identlookups>0</identlookups>
#			<usereversedns>0</usereversedns>
#			<rootlogin>0</rootlogin>
#			<masqueradeaddress></masqueradeaddress>
#			<dynmasqrefresh>0</dynmasqrefresh>
#			<usepassiveports>0</usepassiveports>
#			<minpassiveports>49152</minpassiveports>
#			<maxpassiveports>65534</maxpassiveports>
#			<limittransferrate>0</limittransferrate>
#			<maxuptransferrate>0</maxuptransferrate>
#			<maxdowntransferrate>0</maxdowntransferrate>
#			<anonymous>0</anonymous>
#			<requirevalidshell>1</requirevalidshell>
#			<transferlog>0</transferlog>
#			<homesenable>0</homesenable>
#			<extraoptions></extraoptions>
#			<shares>
#				<!--
#				<share>
#					<uuid>xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx</uuid>
#					<sharedfolderref>xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx</sharedfolderref>
#					<extraoptions></extraoptions>
#					<comment>xxx</comment>
#				</share>
#				-->
#			</shares>
#			<modules>
#				<mod_ban>
#					<!--
#					<rule>
#						<uuid>xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx</uuid>
#						<event>AnonRejectPasswords|ClientConnectRate|MaxClientsPerClass|MaxClientsPerHost|MaxClientsPerUser|MaxConnectionsPerHost|MaxHostsPerUser|MaxLoginAttempts|TimeoutIdle|TimeoutNoTransfer</event>
#						<occurrence>xxx</occurrence>
#						<timeinterval>hh:mm:ss</timeinterval>
#						<expire>hh:mm:ss</expire>
#					</rule>
#					-->
#				</mod_ban>
#				<mod_tls>
#					<enable>0</enable>
#					<required>0</required>
#					<sslcertificateref></sslcertificateref>
#					<nosessionreuserequired>0</nosessionreuserequired>
#					<useimplicitssl>0</useimplicitssl>
#					<extraoptions></extraoptions>
#				</mod_tls>
#			</modules>
#		</ftp>
#   </services>
# </config>
########################################################################
if ! omv-confdbadm exists "conf.service.ftp"; then
	omv-confdbadm read --defaults "conf.service.ftp" | omv-confdbadm update "conf.service.ftp" -
fi

exit 0
