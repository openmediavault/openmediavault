#!/bin/bash

set -e

. /usr/share/openmediavault/scripts/helper-functions

if ! omv_config_exists "/config/services/bcache"; then
	omv_config_add_node "/config/services" "bcache"
	omv_config_add_node "/config/services/bcache" "caches"
	omv_config_add_node "/config/services/bcache" "backings"
fi
