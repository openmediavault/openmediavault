#!/usr/bin/env bats
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

. ./../../scripts/helper-functions

OMV_CONFIG_FILE="./../data/config.xml"

@test "should be a number [1]" {
  run omv_isnumber 1
  [ "$status" -eq 0 ]
}

@test "should be a number [2]" {
  run omv_isnumber "2"
  [ "$status" -eq 0 ]
}

@test "should not be a number [1]" {
  run omv_isnumber "a"
  [ "$status" -eq 1 ]
}

@test "should be an UUID [1]" {
  run omv_isuuid "78b669c1-9183-4ca3-a32c-80a4e2c61e2d"
  [ "$status" -eq 0 ]
}

@test "should not be a UUID [1]" {
  run omv_isuuid "foo"
  [ "$status" -eq 1 ]
}

@test "should be a FS UUID [1]" {
  run omv_isfsuuid "78b669c1-9183-4ca3-a32c-80a4e2c61e2d"
  [ "$status" -eq 0 ]
}

@test "should be a FS UUID [2]" {
  run omv_isfsuuid "7A48-BA97"
  [ "$status" -eq 0 ]
}

@test "should be a FS UUID [3]" {
  run omv_isfsuuid "2ED43920D438EC29"
  [ "$status" -eq 0 ]
}

@test "should be a FS UUID [4]" {
  run omv_isfsuuid "2015-01-13-21-48-46-00"
  [ "$status" -eq 0 ]
}

@test "should not be a FS UUID [1]" {
  run omv_isfsuuid "/dev/sde1"
  [ "$status" -eq 1 ]
}

@test "should be a device file [1]" {
  run omv_is_devicefile "/dev/sda1"
  [ "$status" -eq 0 ]
}

@test "should be a device file [2]" {
  run omv_is_devicefile "/dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0-part2"
  [ "$status" -eq 0 ]
}

@test "should not be a device file [1]" {
  run omv_is_devicefile "foo bar baz"
  [ "$status" -eq 1 ]
}

@test "should not be a device file [2]" {
  run omv_is_devicefile "/devfoo/sda"
  [ "$status" -eq 1 ]
}

@test "should be a block device [1]" {
  run omv_is_block "/dev/sda"
  [ "$status" -eq 0 ]
}

@test "should not be a block device [1]" {
  run omv_is_block "/sys"
  [ "$status" -eq 1 ]
}

@test "should trim the string [1]" {
  run omv_trim " foo	"
  [ "$status" -eq 0 ]
  [ "$output" = "foo" ]
}

@test "should trim the string [2]" {
  run omv_trim -c "a" "aafooaa"
  [ "$status" -eq 0 ]
  [ "$output" = "foo" ]
}

@test "should trim the string [3]" {
  run omv_trim -c "a" -l "aafooaa"
  [ "$status" -eq 0 ]
  [ "$output" = "fooaa" ]
}

@test "should trim the string [4]" {
  run omv_trim -c "a" -r "aafooaa"
  [ "$status" -eq 0 ]
  [ "$output" = "aafoo" ]
}

@test "should trim the string [5]" {
  run omv_trim -c " " -l "  foo   "
  [ "$status" -eq 0 ]
  [ "$output" = "foo   " ]
}

@test "should trim the string [6]" {
  run omv_trim -c " " -r "  foo   "
  [ "$status" -eq 0 ]
  [ "$output" = "  foo" ]
}

@test "should not trim the string [1]" {
  run omv_trim -c "a" "bfoobb"
  [ "$status" -eq 0 ]
  [ "$output" = "bfoobb" ]
}

@test "should rtrim the string [1]" {
  run omv_rtrim " foo	"
  [ "$status" -eq 0 ]
  [ "$output" = " foo" ]
}

@test "should rtrim the string [2]" {
  run omv_rtrim -c "a" "fooaaaa"
  [ "$status" -eq 0 ]
  [ "$output" = "foo" ]
}

@test "should ltrim the string [1]" {
  run omv_ltrim -c "c" "ccfoo	"
  [ "$status" -eq 0 ]
  [ "$output" = "foo	" ]
}

@test "should be true [1]" {
  run omv_checkyesno "1"
  [ "$status" -eq 0 ]
}

@test "should be true [2]" {
  run omv_checkyesno 1
  [ "$status" -eq 0 ]
}

@test "should be true [3]" {
  run omv_checkyesno "y"
  [ "$status" -eq 0 ]
}

@test "should be true [4]" {
  run omv_checkyesno "yes"
  [ "$status" -eq 0 ]
}

@test "should be true [5]" {
  run omv_checkyesno "True"
  [ "$status" -eq 0 ]
}

@test "should be true [6]" {
  run omv_checkyesno "on"
  [ "$status" -eq 0 ]
}

@test "should not be true [1]" {
  run omv_checkyesno "0"
  [ "$status" -eq 1 ]
}

@test "should not be true [2]" {
  run omv_checkyesno 0
  [ "$status" -eq 1 ]
}

@test "should not be true [3]" {
  run omv_checkyesno "n"
  [ "$status" -eq 1 ]
}

@test "should not be true [4]" {
  run omv_checkyesno "No"
  [ "$status" -eq 1 ]
}

@test "should not be true [5]" {
  run omv_checkyesno "false"
  [ "$status" -eq 1 ]
}

@test "should log [1]" {
  run omv_log "foo bar"
  [ "$status" -eq 0 ]
  [ "$output" = "foo bar" ]
}

@test "should log [2]" {
  run omv_log "foo" 2 "bar"
  [ "$status" -eq 0 ]
  [ "$output" = "foo 2 bar" ]
}

@test "should log info [1]" {
  run omv_info "foo bar"
  [ "$status" -eq 0 ]
  [ "$output" = "INFO: foo bar" ]
}

@test "should log warning [1]" {
  run omv_warning "foo bar"
  [ "$status" -eq 0 ]
  [ "$output" = "WARNING: foo bar" ]
}

@test "should log error [1]" {
  run omv_error "foo bar"
  [ "$status" -eq 0 ]
  [ "$output" = "ERROR: foo bar" ]
}

@test "should display message [1]" {
  run omv_msg "foo bar"
  [ "$status" -eq 0 ]
  [ "$output" = "foo bar" ]
}

@test "should display message [2]" {
  run omv_msg "foo" "bar"
  [ "$status" -eq 0 ]
  [ "$output" = "foo bar" ]
}

@test "should display message [3]" {
  run omv_msg foo bar test 1 2 3
  [ "$status" -eq 0 ]
  [ "$output" = "foo bar test 1 2 3" ]
}

@test "should log debug [1]" {
  export OMV_DEBUG_SCRIPT=yes
  run omv_debug foo bar baz
  [ "$status" -eq 0 ]
  [ "$output" = "DEBUG: foo bar baz" ]
}

@test "should not log debug [1]" {
  run omv_debug foo bar baz
  [ "$status" -eq 0 ]
  [ "$output" = "" ]
}

@test "should not log debug [2]" {
  export OMV_DEBUG_SCRIPT=no
  run omv_debug foo bar baz
  [ "$status" -eq 0 ]
  [ "$output" = "" ]
}

@test "should quote string [1]" {
  run omv_quotemeta "foo/bar"
  [ "$status" -eq 0 ]
  [ "$output" = "foo\/bar" ]
}

@test "should quote string [2]" {
  run omv_quotemeta "foo\bar"
  [ "$status" -eq 0 ]
  [ "$output" = "foo\\\\bar" ]
}

@test "should quote string [3]" {
  run omv_quotemeta "foo&bar&"
  [ "$status" -eq 0 ]
  [ "$output" = "foo\&bar\&" ]
}

@test "should convert mask to CIDR [1]" {
  run omv_mask2cidr "255.255.255.0"
  [ "$status" -eq 0 ]
  [ "$output" = "24" ]
}

@test "should convert CIDR to mask [1]" {
  run omv_cidr2mask 24
  [ "$status" -eq 0 ]
  [ "$output" = "255.255.255.0" ]
}

@test "should be an IPv4 address [1]" {
  run omv_is_ipaddr4 "192.172.16.1"
  [ "$status" -eq 0 ]
}

@test "should not be an IPv4 address [1]" {
  run omv_is_ipaddr4 "::1"
  [ "$status" -eq 1 ]
}

@test "should not be an IPv4 address [2]" {
  run omv_is_ipaddr4 "foo"
  [ "$status" -eq 1 ]
}

@test "should be an IPv4 netmask [1]" {
  run omv_is_netmask4 "255.255.255.0"
  [ "$status" -eq 0 ]
}

@test "should not be an IPv4 netmask [1]" {
  run omv_is_netmask4 "192.172.16.1"
  [ "$status" -eq 1 ]
}

@test "should not be an IPv4 netmask [2]" {
  run omv_is_netmask4 "172.16.1"
  [ "$status" -eq 1 ]
}

@test "should not be an IPv4 netmask [3]" {
  run omv_is_netmask4 "baz"
  [ "$status" -eq 1 ]
}

@test "should be an IPv4 gateway [1]" {
  run omv_is_gateway4 "192.168.168.8"
  [ "$status" -eq 0 ]
}

@test "should not be an IPv4 gateway [1]" {
  run omv_is_gateway4 "foo"
  [ "$status" -eq 1 ]
}

@test "should not be an IPv4 gateway [2]" {
  run omv_is_gateway4 "192.172.16"
  [ "$status" -eq 1 ]
}

@test "should be an IPv6 address [1]" {
  run omv_is_ipaddr6 "::1"
  [ "$status" -eq 0 ]
}

@test "should be an IPv6 address [2]" {
  run omv_is_ipaddr6 "e7d8:ec0c:1d0d:4b4d:cd74:e06f:3be6:2d1e"
  [ "$status" -eq 0 ]
}

@test "should not be an IPv6 address [1]" {
  run omv_is_ipaddr6 "192.172.16.1"
  [ "$status" -eq 1 ]
}

@test "should not be an IPv6 address [2]" {
  run omv_is_ipaddr6 "foo"
  [ "$status" -eq 1 ]
}

@test "should be an IPv6 netmask [1]" {
  run omv_is_netmask6 128
  [ "$status" -eq 0 ]
}

@test "should be an IPv6 netmask [2]" {
  run omv_is_netmask6 "64"
  [ "$status" -eq 0 ]
}

@test "should not be an IPv6 netmask [1]" {
  run omv_is_netmask6 "225"
  [ "$status" -eq 1 ]
}

@test "should be an IPv6 gateway [1]" {
  run omv_is_gateway6 "fe80::2b7:4aff:fe34:09e4"
  [ "$status" -eq 0 ]
}

@test "should not be an IPv6 gateway [1]" {
  run omv_is_gateway6 "foo"
  [ "$status" -eq 1 ]
}

@test "should not be an IPv6 gateway [2]" {
  run omv_is_gateway6 "192.172.16.1"
  [ "$status" -eq 1 ]
}

@test "should be a WLAN device [1]" {
  run omv_is_wlan "wlan1"
  [ "$status" -eq 0 ]
}

@test "should be a WLAN device [2]" {
  run omv_is_wlan "wlp3s0"
  [ "$status" -eq 0 ]
}

@test "should not be a WLAN device [1]" {
  run omv_is_wlan "virbr4"
  [ "$status" -eq 1 ]
}

@test "should generate mount dir [1]" {
  OMV_MOUNT_DIR="/srv" run omv_build_mount_dir "6c5be784-50a8-440c-9d25-aab99b9c6fb1"
  [ "$status" -eq 0 ]
  [ "$output" = "/srv/6c5be784-50a8-440c-9d25-aab99b9c6fb1" ]
}

@test "should generate mount dir [2]" {
  OMV_MOUNT_DIR="/srv" run omv_build_mount_dir "/dev/disk/by-id/wwn-0x5000cca211cc703c-part1"
  [ "$status" -eq 0 ]
  [ "$output" = "/srv/_dev_disk_by-id_wwn-0x5000cca211cc703c-part1" ]
}

@test "should get number of elements [1]" {
  run omv_config_get_count "//notification/notifications/notification"
  [ "$status" -eq 0 ]
  [ "$output" -eq 8 ]
}

@test "should config exist [1]" {
  run omv_config_exists "/config/services/ssh"
  [ "$status" -eq 0 ]
}

@test "should not config exist [1]" {
  run omv_config_exists "/config/services/foo"
  [ "$status" -eq 1 ]
}

@test "should get config [1]" {
  run omv_config_get "/config/services/ssh/enable"
  [ "$status" -eq 0 ]
  [ "$output" -eq 1 ]
}

@test "should not get config [1]" {
  run omv_config_get "/config/services/ssh/foo"
  [ "$status" -eq 0 ]
  [ "$output" = "" ]
}

@test "should calc MD5 sum [1]" {
  run omv_md5 "foo"
  [ "$status" -eq 0 ]
  [ ${#output} -eq 32 ]
}

@test "should check for user ID [1]" {
  run omv_user_id_exists "root"
  [ "$status" -eq 0 ]
}

@test "should check for group ID [1]" {
  run omv_group_id_exists "root"
  [ "$status" -eq 0 ]
}

@test "should get iface [1]" {
  run omv_get_if "auto"
  [ "$status" -eq 0 ]
  [ ${#output} -ne 0 ]
}

@test "should get iface [2]" {
  run omv_get_if "eth0"
  [ "$status" -eq 0 ]
  [ "$output" = "eth0" ]
}

@test "should add/update/remove config [1]" {
  run omv_config_exists "/config/system/foo"
  [ "$status" -eq 1 ]
  run omv_config_add_node "/config/system" "foo"
  [ "$status" -eq 0 ]
  run omv_config_exists "/config/system/foo"
  [ "$status" -eq 0 ]
  run omv_config_exists "/config/system/foo/bar"
  [ "$status" -eq 1 ]
  run omv_config_get "/config/system/foo/bar"
  [ "$status" -eq 0 ]
  [ "$output" = "" ]
  run omv_config_add_key "/config/system/foo" "bar" "baz"
  [ "$status" -eq 0 ]
  run omv_config_exists "/config/system/foo/bar"
  [ "$status" -eq 0 ]
  run omv_config_get "/config/system/foo/bar"
  [ "$status" -eq 0 ]
  [ "$output" = "baz" ]
  run omv_config_rename "/config/system/foo/bar" "abc"
  [ "$status" -eq 0 ]
  run omv_config_exists "/config/system/foo/bar"
  [ "$status" -eq 1 ]
  run omv_config_exists "/config/system/foo/abc"
  [ "$status" -eq 0 ]
  run omv_config_update "/config/system/foo/abc" "zab"
  [ "$status" -eq 0 ]
  run omv_config_get "/config/system/foo/abc"
  [ "$status" -eq 0 ]
  [ "$output" = "zab" ]
  run omv_config_delete "/config/system/foo"
  [ "$status" -eq 0 ]
  run omv_config_exists "/config/system/foo/abc"
  [ "$status" -eq 1 ]
  run omv_config_exists "/config/system/foo"
  [ "$status" -eq 1 ]
}
