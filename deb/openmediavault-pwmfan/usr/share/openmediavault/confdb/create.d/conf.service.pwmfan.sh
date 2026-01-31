#!/bin/sh

set -e

. /usr/share/openmediavault/scripts/helper-functions

SERVICE_XPATH_NAME="pwmfan"
SERVICE_XPATH="/config/services/${SERVICE_XPATH_NAME}"

if ! omv_config_exists "${SERVICE_XPATH}"; then
    omv_config_add_node "/config/services" "${SERVICE_XPATH_NAME}"
    omv_config_add_key "${SERVICE_XPATH}" "enable" "0"
    omv_config_add_key "${SERVICE_XPATH}" "temp_source" "hdd"
    omv_config_add_key "${SERVICE_XPATH}" "hdd_drives" "/dev/sda,/dev/sdb"
    omv_config_add_key "${SERVICE_XPATH}" "hdd_standby" "1"
    omv_config_add_key "${SERVICE_XPATH}" "cpu_temp_path" "/sys/class/thermal/thermal_zone0/temp"
    omv_config_add_key "${SERVICE_XPATH}" "pwm_chip" "/sys/class/pwm/pwmchip0"
    omv_config_add_key "${SERVICE_XPATH}" "pwm_channel" "0"
    omv_config_add_key "${SERVICE_XPATH}" "pwm_period" "40000"
    omv_config_add_key "${SERVICE_XPATH}" "update_interval" "60"
    omv_config_add_key "${SERVICE_XPATH}" "temp1" "28"
    omv_config_add_key "${SERVICE_XPATH}" "temp2" "31"
    omv_config_add_key "${SERVICE_XPATH}" "temp3" "33"
    omv_config_add_key "${SERVICE_XPATH}" "speed1" "25"
    omv_config_add_key "${SERVICE_XPATH}" "speed2" "50"
    omv_config_add_key "${SERVICE_XPATH}" "speed3" "75"
    omv_config_add_key "${SERVICE_XPATH}" "speed4" "100"
    omv_config_add_key "${SERVICE_XPATH}" "failsafe_speed" "25"
fi

exit 0
