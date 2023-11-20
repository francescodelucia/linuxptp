/**
 * @file ptp4l.c
 * @brief PTP Boundary Clock or Transparent Clock main program
 * @note Copyright (C) 2011 Richard Cochran <richardcochran@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "clock.h"
#include "config.h"
#include "ntpshm.h"
#include "pi.h"
#include "print.h"
#include "raw.h"
#include "sk.h"
#include "transport.h"
#include "udp6.h"
#include "uds.h"
#include "util.h"
#include "version.h"
#include "mod_udpServer.h"
#include "mod_webServer.h"
#include "mod_udpClient.h"

static void usage(char *progname)
{
	fprintf(stderr,
		"\nusage: %s [options]\n\n"
		" Delay Mechanism\n\n"
		" -A        Auto, starting with E2E\n"
		" -E        E2E, delay request-response (default)\n"
		" -P        P2P, peer delay mechanism\n\n"
		" Network Transport\n\n"
		" -2        IEEE 802.3\n"
		" -4        UDP IPV4 (default)\n"
		" -6        UDP IPV6\n\n"
		" Time Stamping\n\n"
		" -H        HARDWARE (default)\n"
		" -S        SOFTWARE\n"
		" -L        LEGACY HW\n\n"
		" Other Options\n\n"
		" -f [file] read configuration from 'file'\n"
		" -i [dev]  interface device to use, for example 'eth0'\n"
		"           (may be specified multiple times)\n"
		" -p [dev]  Clock device to use, default auto\n"
		"           (ignored for SOFTWARE/LEGACY HW time stamping)\n"
		" -s        client only synchronization mode (overrides configuration file)\n"
		" -l [num]  set the logging level to 'num'\n"
		" -m        print messages to stdout\n"
		" -q        do not print messages to the syslog\n"
		" -v        prints the software version and exits\n"
		" -h        prints this message and exits\n"
		" -w		enable webserver diagnostic on port 8000 and UDP server 8090\n"
		" -W [port] enable webserver diagnostic on different web port\n"
		" -U [port] set UDP port server for diagnostic mode\n"
		" -u        enable sending data for diagnostic mode on port 8090\n"
		" -R [ip]   set ip remote srever diferent from PTP signal\n"

		"\n",
		progname);
}

int main(int argc, char *argv[])
{
	char *config = NULL, *req_phc = NULL, *progname;
	enum clock_type type = CLOCK_TYPE_ORDINARY;
	int c, err = -1, index, print_level;
	struct clock *clock = NULL;
	struct option *opts;
	struct config *cfg;
	
	struct mod_config m_conf = get_base_mod_config();	
	
	if (handle_term_signals())
		return -1;

	cfg = config_create();
	if (!cfg) {
		return -1;
	}
	opts = config_long_options(cfg);

	/* Process the command line arguments. */
	progname = strrchr(argv[0], '/');
	progname = progname ? 1+progname : argv[0];
	while (EOF != (c = getopt_long(argc, argv, "AEP246HSLf:i:p:sl:mqvhwW:U:uR:",
				       opts, &index))) {		
		switch (c) {
		case 0:
			if (config_parse_option(cfg, opts[index].name, optarg))
				goto out;
			break;
		case 'A':
			if (config_set_int(cfg, "delay_mechanism", DM_AUTO))
				goto out;
			break;
		case 'E':
			if (config_set_int(cfg, "delay_mechanism", DM_E2E))
				goto out;
			break;
		case 'P':
			if (config_set_int(cfg, "delay_mechanism", DM_P2P))
				goto out;
			break;
		case '2':
			if (config_set_int(cfg, "network_transport",
					    TRANS_IEEE_802_3))
				goto out;
			break;
		case '4':
			if (config_set_int(cfg, "network_transport",
					    TRANS_UDP_IPV4))
				goto out;
			break;
		case '6':
			if (config_set_int(cfg, "network_transport",
					    TRANS_UDP_IPV6))
				goto out;
			break;
		case 'H':
			if (config_set_int(cfg, "time_stamping", TS_HARDWARE))
				goto out;
			break;
		case 'S':
			if (config_set_int(cfg, "time_stamping", TS_SOFTWARE))
				goto out;
			break;
		case 'L':
			if (config_set_int(cfg, "time_stamping", TS_LEGACY_HW))
				goto out;
			break;
		case 'f':
			config = optarg;
			break;
		case 'i':
			if (!config_create_interface(optarg, cfg)){				
				goto out;
			}else{				
				sprintf(m_conf.iface,"%s",optarg);
			}
			break;
		case 'p':
			req_phc = optarg;
			break;
		case 's':
			if (config_set_int(cfg, "clientOnly", 1)) {
				goto out;
			}
			break;
		case 'l':
			if (get_arg_val_i(c, optarg, &print_level,
					  PRINT_LEVEL_MIN, PRINT_LEVEL_MAX))
				goto out;
			config_set_int(cfg, "logging_level", print_level);
			break;
		case 'm':
			config_set_int(cfg, "verbose", 1);
			break;
		case 'q':
			config_set_int(cfg, "use_syslog", 0);
			break;
		case 'v':
			version_show(stdout);
			return 0;
		case 'h':
			usage(progname);
			return 0;
		case 'w':
			m_conf.mode = MODE_SERVER;
			break;
		case 'W':
			m_conf.port_www = atoi(optarg);
			m_conf.mode = MODE_SERVER;			
			break;
		case 'U':
			m_conf.udp_port = atoi(optarg);
			m_conf.mode = MODE_SERVER;
			break;
		case 'u':
			m_conf.mode = MODE_CLIENT;
			break;
		case 'R':									
			if(mod_parse_ip(optarg,strlen(optarg),&(m_conf.remoteIP))!=0){
				goto out;
			}
			m_conf.mode = MODE_CLIENT;					
			break;
		case '?':
			usage(progname);
			goto out;
		default:
			usage(progname);
			goto out;
		}
	}	
	if(m_conf.mode!=NO_MODE){
		
		if(m_conf.mode == MODE_SERVER){
			if(udpServer(m_conf.udp_port)>0){		
    			webServer(m_conf.port_www);  				  			
			}
			set_Config((void*)&m_conf);			
			printf(C_BLUE"\nSystem Monitoring Web by De Lucia Franceso\n"C_RESET);
			printf(C_MAGENTA "############### MODE_SERVER #############\n"C_RESET);
			printf(C_YELLOW"www port[" C_GREEN "%i" C_YELLOW "]\nudp port[" C_GREEN "%i" C_YELLOW "]\nmode [" C_GREEN "%i" C_YELLOW "]\n",m_conf.port_www,m_conf.udp_port ,m_conf.mode) ;			
			printf(C_MAGENTA "#########################################\n"C_RESET);  
		}else{
			set_Config((void*)&m_conf);
			udpClient(NULL);
			printf(C_BLUE"\nSystem Monitoring Web by De Lucia Franceso\n"C_RESET);
			printf(C_MAGENTA"###############NO_MODE#############\n"C_RESET);
			printf(C_YELLOW"www port["C_GREEN "%i"C_YELLOW "]\nudp port["C_GREEN "%i"C_YELLOW "]\nmode ["C_GREEN "%i"C_YELLOW "]\n",m_conf.port_www,m_conf.udp_port ,m_conf.mode) ;			
			printf(C_MAGENTA"#######################################\n"C_RESET);  
		}			
	}
	if (config && (c = config_read(config, cfg))) {
		return c;
	}

	print_set_progname(progname);
	print_set_tag(config_get_string(cfg, NULL, "message_tag"));
	print_set_verbose(config_get_int(cfg, NULL, "verbose"));
	print_set_syslog(config_get_int(cfg, NULL, "use_syslog"));
	print_set_level(config_get_int(cfg, NULL, "logging_level"));

	assume_two_step = config_get_int(cfg, NULL, "assume_two_step");
	sk_check_fupsync = config_get_int(cfg, NULL, "check_fup_sync");
	sk_tx_timeout = config_get_int(cfg, NULL, "tx_timestamp_timeout");
	sk_hwts_filter_mode = config_get_int(cfg, NULL, "hwts_filter");

	if (config_get_int(cfg, NULL, "clock_servo") == CLOCK_SERVO_NTPSHM) {
		config_set_int(cfg, "kernel_leap", 0);
		config_set_int(cfg, "sanity_freq_limit", 0);
	}

	if (STAILQ_EMPTY(&cfg->interfaces)) {
		fprintf(stderr, "no interface specified\n");
		usage(progname);
		goto out;
	}

	type = config_get_int(cfg, NULL, "clock_type");
	switch (type) {
	case CLOCK_TYPE_ORDINARY:
		if (cfg->n_interfaces > 1) {
			type = CLOCK_TYPE_BOUNDARY;
		}
		break;
	case CLOCK_TYPE_BOUNDARY:
		if (cfg->n_interfaces < 2) {
			fprintf(stderr, "BC needs at least two interfaces\n");
			goto out;
		}
		break;
	case CLOCK_TYPE_P2P:
		if (cfg->n_interfaces < 2) {
			fprintf(stderr, "TC needs at least two interfaces\n");
			goto out;
		}
		if (DM_P2P != config_get_int(cfg, NULL, "delay_mechanism")) {
			fprintf(stderr, "P2P_TC needs P2P delay mechanism\n");
			goto out;
		}
		break;
	case CLOCK_TYPE_E2E:
		if (cfg->n_interfaces < 2) {
			fprintf(stderr, "TC needs at least two interfaces\n");
			goto out;
		}
		if (DM_E2E != config_get_int(cfg, NULL, "delay_mechanism")) {
			fprintf(stderr, "E2E_TC needs E2E delay mechanism\n");
			goto out;
		}
		break;
	case CLOCK_TYPE_MANAGEMENT:
		goto out;
	}

	clock = clock_create(type, cfg, req_phc);
	if (!clock) {
		fprintf(stderr, "failed to create a clock\n");
		goto out;
	}

	err = 0;

	while (is_running()) {
		if (clock_poll(clock))
			break;
	}
out:
	if (clock)
		clock_destroy(clock);
	config_destroy(cfg);
	return err;
}
