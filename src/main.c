/**
 * @file main.c
 * @brief Sample program to demostrate use of console API
 * @version 0.1
 * @date 2022-03-03
 * 
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <lib.h>
#include <ril.h>
#include <os_api.h>
#include <command.h>
#include <console.h>

static char device_name[50], device_ip[100];
static uint32_t device_id;

/**
 * URC Handler
 * @param param1	URC Code
 * @param param2	URC Parameter
 */
static void urc_callback(unsigned int param1, unsigned int param2)
{
	switch (param1) {
	case URC_SYS_INIT_STATE_IND:
		if (param2 == SYS_STATE_SMSOK) {
			/* Ready for SMS */
		}
		break;
	case URC_SIM_CARD_STATE_IND:
		switch (param2) {
		case SIM_STAT_NOT_INSERTED:
			debug(DBG_OFF, "SYSTEM: SIM card not inserted!\n");
			break;
		case SIM_STAT_READY:
			debug(DBG_INFO, "SYSTEM: SIM card Ready!\n");
			break;
		case SIM_STAT_PIN_REQ:
			debug(DBG_OFF, "SYSTEM: SIM PIN required!\n");
			break;
		case SIM_STAT_PUK_REQ:
			debug(DBG_OFF, "SYSTEM: SIM PUK required!\n");
			break;
		case SIM_STAT_NOT_READY:
			debug(DBG_OFF, "SYSTEM: SIM card not recognized!\n");
			break;
		default:
			debug(DBG_OFF, "SYSTEM: SIM ERROR: %d\n", param2);
		}
		break;
	case URC_GSM_NW_STATE_IND:
		debug(DBG_OFF, "SYSTEM: GSM NW State: %d\n", param2);
		break;
	case URC_GPRS_NW_STATE_IND:
		break;
	case URC_CFUN_STATE_IND:
		break;
	case URC_COMING_CALL_IND:
		debug(DBG_OFF, "Incoming voice call from: %s\n", ((struct ril_callinfo_t *)param2)->number);
		/* Take action here, Answer/Hang-up */
		break;
	case URC_CALL_STATE_IND:
		switch (param2) {
		case CALL_STATE_BUSY:
			debug(DBG_OFF, "The number you dialed is busy now\n");
			break;
		case CALL_STATE_NO_ANSWER:
			debug(DBG_OFF, "The number you dialed has no answer\n");
			break;
		case CALL_STATE_NO_CARRIER:
			debug(DBG_OFF, "The number you dialed cannot reach\n");
			break;
		case CALL_STATE_NO_DIALTONE:
			debug(DBG_OFF, "No Dial tone\n");
			break;
		default:
			break;
		}
		break;
	case URC_NEW_SMS_IND:
		debug(DBG_OFF, "SMS: New SMS (%d)\n", param2);
		/* Handle New SMS */
		break;
	case URC_MODULE_VOLTAGE_IND:
		debug(DBG_INFO, "VBatt Voltage: %d\n", param2);
		break;
	case URC_ALARM_RING_IND:
		break;
	case URC_FILE_DOWNLOAD_STATUS:
		break;
	case URC_FOTA_STARTED:
		break;
	case URC_FOTA_FINISHED:
		break;
	case URC_FOTA_FAILED:
		break;
	case URC_STKPCI_RSP_IND:
		break;
	default:
		break;
	}
}

/**
 * @brief User authentication function. password can be stored as hashed
 * and checked more securely
 *
 * @param username Username passed from terminal
 * @param pass Password from terminal
 * @return auth status
 */
static int check_user_auth(const char *username, const char *pass)
{
	/**
	 * Simple text based authentication, if username is admin and password is
	 * adminpass, authentication is passed as Admin user.
	 */
	if (!strcmp(username, "admin") && !strcmp(pass, "adminpass"))
		return AUTH_ADMIN;

	/**
	 * User with any username but password as pass with be authenticated as a user
	 */
	if (!strcmp(pass, "pass"))
		return AUTH_USER;

	/* rest all will be considered as failed authentication */	
	return AUTH_FAIL;
}

/**
 * @brief Name command handler
 * 
 * @param argc Argument count passed
 * @param argv list of command line arguments
 * @param info Command information
 * @return return result code
 */
static int do_name(int argc, const char **argv, struct cmdinfo_t *info)
{
	if (argc == 1) {
		/* read command */
		printf("Current name set is: %s\n", device_name);
	} else {
		/* set a new name */
		if (strlen(argv[1]) >= sizeof(device_name)) {
			printf("Length too long\n");
			return CMD_RET_FAILURE;
		}
		strcpy(device_name, argv[1]);
		printf("New name set to: %s\n", device_name);
	}

	return CMD_RET_SUCCESS;
}

static int do_setid(int argc, const char **argv, struct cmdinfo_t *info)
{
	if (argc == 1) {
		printf("ID is %ld\n", device_id);
	} else {
		device_id = strtoul(argv[1], NULL, 10);
		printf("New ID is %ld\n", device_id);
	}

	return CMD_RET_SUCCESS;
}

static int do_setip(int argc, const char **argv, struct cmdinfo_t *info)
{
	if (argc == 1) {
		printf("IP is %s\n", device_ip);
	} else {
		if (strlen(argv[1]) >= sizeof(device_ip)) {
			printf("Length too long\n");
			return CMD_RET_FAILURE;
		}

		strcpy(device_ip, argv[1]);
		printf("IP set to %s\n", device_ip);
	}

	return CMD_RET_SUCCESS;
}

/**
 * @brief Example command name.
 *
 * Command type is set to default and will show in help menu and autocomplete
 * A user with admin/user privilege will be able to execute this command.
 *
 * To set device name:
 * name new_name
 *
 * To get device name:
 * name
 *
 */
CMD_ADD(name, 1, 2, do_name, "Set/get device name", "name [value]", CMD_TYPE_DEFAULT);

/**
 * @brief Example command to Set ID.
 * 
 * This command is set as Hidden command (CMD_TYPE_HIDDEN), so it will not show in
 * help menu or autocomplete. Users with admin privilege will be able to execute
 * command.
 * 
 * To set device ID value:
 * setid 123
 * 
 * To get current device ID:
 * setid
 * 
 */
CMD_ADD(setid, 1, 2, do_setid, "Set/get device ID", "setid [value]", CMD_TYPE_HIDDEN);

/**
 * @brief Example command to set server IP or domain
 * 
 * This command is configured as a hidden but with allowed execution via console.
 * Since it's defined as hidden it will not show in help menu or autocomplete.
 * 
 * This command can only be executed by user with admin privilege or from console only
 * by any user.
 * 
 */
CMD_ADD(setip, 1, 2, do_setip, "set/get current IP", "setip [IP/domain]", CMD_TYPE_HIDDEN | CMD_ALLOW_CONSOLE);


/**
 * Application main entry point
 */
int main(int argc, char *argv[])
{
	/*
	 * Initialize library and Setup STDIO
	 */
	logicrom_init("/dev/ttyS0", urc_callback);

	/* setup console authentication function */
	cli_set_authfn(check_user_auth);

	printf("\n\nConsole Example\nType help to get list of commands available\n\n");

	while (1) {
		/* Main task */
		sleep(1);
	}
}

