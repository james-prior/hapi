#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
HAPI Smart Module v2.1.2
Release: April 2017 Beta Milestone

Copyright 2016 Maya Culpa, LLC

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
"""

from __future__ import print_function
import os
import sys
import time
import datetime
import subprocess
import socket
import codecs
from multiprocessing import Process
import urllib2
import json
import sqlite3
import log
import schedule
import communicator
from influxdb import InfluxDBClient
from status import SystemStatus
import asset_interface
import rtc_interface
from alert import Alert
import utilities
from zeroconf import ServiceBrowser, ServiceStateChange, Zeroconf

reload(sys)

class Asset(object):
    """Hold Asset (sensor) information."""
    def __init__(self):
        self.id = "1"
        self.name = "Indoor Temperature"
        self.unit = "C"
        self.type = "wt"
        self.virtual = 0
        self.context = "Environment"
        self.system = "Test"
        self.enabled = True
        self.value = None
        self.alert = None

    def __str__(self):
        """Return Asset information in JSON."""
        return str([{"id": self.id, "name": self.name, "value": self.value}])

    def load_asset_info(self):
        """Load asset information based on dabase."""
        pass

class SmartModule(object):
    """Represents a HAPI Smart Module (Implementation).

    Attributes:
        id: ID of the site
        name: Name of the site
        wunder_key: Weather Underground key to be used
        operator: Name of the primary site operator
        email: Email address of the primary site operator
        phone: Phone number of the primary site operator
        location: Location or Address of the site
    """

    def __init__(self):
        self.mock = True
        self.comm = communicator.Communicator(self)
        self.data_sync = DataSync()
        self.id = ""
        self.name = ""
        self.wunder_key = ""
        self.operator = ""
        self.email = ""
        self.phone = ""
        self.location = ""
        self.longitude = ""
        self.latitude = ""
        self.twilio_acct_sid = ""
        self.twilio_auth_token = ""
        self.scheduler = None
        self.hostname = ""
        self.last_status = ""
        self.ifconn = InfluxDBClient("127.0.0.1", 8086, "root", "root")
        self.log = log.Log("smartmodule.log")
        self.rtc = rtc_interface.RTCInterface()
        self.rtc.power_on_rtc()
        self.launch_time = self.rtc.get_datetime()
        self.asset = Asset()
        self.asset.id = self.rtc.get_id()
        self.asset.alert = Alert(1)
        self.asset.context = self.rtc.get_context()
        self.asset.type = self.rtc.get_type()
        self.ai = asset_interface.AssetInterface(self.asset.type, self.rtc.mock)
        self.rtc.power_off_rtc()

    def become_broker(self):
        """If no broker found. SM performs operation to become the broker."""
        try:
            # We will change it soon!
            os.system("sudo systemctl start avahi-daemon.service")
        except Exception as excpt:
            self.log.info("Error trying to become the Broker: %s.", excpt)

    def find_service(self, zeroconf, service_type, name, state_change):
        """Check for published MQTT. If it finds port 1883 of type '_mqtt', update broker name."""
        # Get the service we want (port 1883 and type '_mqtt._tcp.local.'
        info = zeroconf.get_service_info(service_type, name)
        if not (info.port == 1883 and service_type == "_mqtt._tcp.local."):
            return

        if state_change is ServiceStateChange.Added:
            # If this is our service, update mqtt broker name and ip on self.comm (Communicator)
            self.comm.broker_name = info.server
            self.comm.broker_ip = str(socket.inet_ntoa(info.address))
        elif state_change is ServiceStateChange.Removed:
            # Implement way to handle removed MQTT service
            # It only makes sense if leave zeroconf connection opened. It could be interesting.
            pass

    def find_broker(self, zeroconf):
        """Create zeroconf object, if needed, and browser for services."""
        browser = ServiceBrowser(zeroconf, "_mqtt._tcp.local.", handlers=[self.find_service])

    def discover(self):
        print("{status} Smart Module hosting asset {asset_id} {asset_type} {asset_context}.".format(
            status="Mock" if self.rtc.mock else "Real",
            asset_id=self.asset.id,
            asset_type=self.asset.type,
            asset_context=self.asset.context))

        try:
            # Using two different call to 'time.sleep()'. We should consider a better approach.
            # I'm totally open for new ideas on this.
            max_sleep_time = 3
            zeroconf = Zeroconf()
            self.log.info("Performing Broker discovery...")
            self.find_broker(zeroconf)
            time.sleep(max_sleep_time)
            if self.comm.broker_name or self.comm.broker_ip:
                self.log.info("MQTT Broker: {broker_name} IP: {broker_ip}.".format(
                    broker_name=self.comm.broker_name,
                    broker_ip=self.comm.broker_ip))
            else:
                self.log.info("Broker not found. Becoming the broker.")
                self.become_broker()
            time.sleep(max_sleep_time)
            self.comm.connect()
        except Exception as excpt:
            self.log.exceptions("[Exiting] Trying to find or become the broker.");
        finally:
            self.log.info("Closing Zeroconf connection.")
            zeroconf.close()

        t_end = time.time() + 10
        while (time.time() < t_end) and not self.comm.is_connected:
            time.sleep(1)

        self.comm.subscribe("SCHEDULER/RESPONSE")
        self.comm.send("SCHEDULER/QUERY", "Where are you?")
        # Just wait for reply... We must review it.
        time.sleep(2)

        self.hostname = socket.gethostname()
        self.comm.send("ANNOUNCE", self.hostname + " is online.")

        t_end = time.time() + 2
        while (time.time() < t_end) and not self.comm.is_connected:
            time.sleep(1)

        if not self.comm.scheduler_found:
            # Loading scheduled jobs
            try:
                self.log.info("No Scheduler found. Becoming the Scheduler.")
                self.scheduler = Scheduler()
                self.scheduler.smart_module = self
                self.scheduler.prepare_jobs(self.scheduler.load_schedule())
                self.comm.scheduler_found = True
                self.comm.subscribe("SCHEDULER/QUERY")
                self.comm.unsubscribe("SCHEDULER/RESPONSE")
                self.comm.subscribe("STATUS/RESPONSE")
                self.comm.subscribe("ASSET/RESPONSE" + "/#")
                self.comm.subscribe("ALERT")
                self.comm.send("SCHEDULER/RESPONSE", self.hostname)
                self.comm.send("ANNOUNCE", self.hostname + " is running the Scheduler.")
                self.log.info("Scheduler program loaded.")
            except Exception as excpt:
                self.log.exception("Error initializing scheduler. %s.", excpt)

    def load_site_data(self):
        field_names = '''
            id
            name
            wunder_key
            operator
            email
            phone
            location
            longitude
            latitude
            twilio_acct_sid
            twilio_auth_token
        '''.split()
        try:
            sql = 'SELECT {fields} FROM site LIMIT 1;'.format(
                fields=', '.join(field_names))
            database = sqlite3.connect(utilities.DB_CORE)
            db_elements = database.cursor().execute(sql)
            for row in db_elements:
                for field_name, field_value in zip(field_names, row):
                    setattr(self, field_name, field_value)
            self.log.info("Site data loaded.")
        except Exception as excpt:
            self.log.exception("Error loading site data: %s.", excpt)
        finally:
            database.close()

    def connect_influx(self, database_name):
        """Connect to database named database_name on InfluxDB server.
        Create database if it does not already exist.
        Return the connection to the database."""

        databases = self.ifconn.get_list_database()
        for db in databases:
            if database_name in db.values():
                break
        else:
            self.ifconn.create_database(database_name)

        self.ifconn.switch_database(database_name)
        return self.ifconn

    def push_sysinfo(self, asset_context, information):
        """Push System Status (stats) to InfluxDB server."""
        timestamp = datetime.datetime.now()
        conn = self.connect_influx(asset_context)
        cpuinfo = [{"measurement": "cpu", "tags": {"asset": self.name}, "time": timestamp,
                    "fields": {
                        "unit": "percentage",
                        "load": information.cpu["percentage"]
                    }
                   }]
        meminfo = [{"measurement": "memory", "tags": {"asset": self.name}, "time": timestamp,
                    "fields": {
                        "unit": "bytes",
                        "free": information.memory["free"],
                        "used": information.memory["used"],
                        "cached": information.memory["cached"]
                    }
                   }]
        netinfo = [{"measurement": "network", "tags": {"asset": self.name}, "time": timestamp,
                    "fields": {
                        "unit": "packets",
                        "packet_recv": information.network["packet_recv"],
                        "packet_sent": information.network["packet_sent"]
                    }
                   }]
        botinfo = [{"measurement": "boot", "tags": {"asset": self.name}, "time": timestamp,
                    "fields": {
                        "unit": "timestamp",
                        "date": information.boot
                    }
                   }]
        diskinf = [{"measurement": "disk", "tags": {"asset": self.name}, "time": timestamp,
                    "fields": {
                        "unit": "bytes",
                        "total": information.disk["total"],
                        "free": information.disk["free"],
                        "used": information.disk["used"]
                    }
                   }]
        tempinf = [{"measurement": "internal", "tags": {"asset": self.name}, "time": timestamp,
                    "fields": {
                    "unit": "C",
                    "unit temp": str(self.rtc.get_temp()),
                    }
                   }]

        ctsinfo = [{"measurement": "clients", "tags": {"asset": self.name}, "time": timestamp,
                    "fields": {
                        "unit": "integer",
                        "clients": information.clients
                    }
                   }]
        json = cpuinfo + meminfo + netinfo + botinfo + diskinf + tempinf + ctsinfo
        conn.write_points(json)

    def get_status(self, brokerconnections):
        """Fetch system information (stats)."""
        try:
            sysinfo = SystemStatus(update=True)
            sysinfo.clients = brokerconnections
            return sysinfo
        except Exception as excpt:
            self.log.exception("Error getting System Status: %s.", excpt)

    def on_query_status(self):
        """It'll be called by the Scheduler to ask for System Status information."""
        self.comm.send("STATUS/QUERY", "How are you?")

    def on_check_alert(self):
        """It'll called by the Scheduler to ask for Alert Conditions."""
        self.comm.send("ASSET/QUERY/" + self.asset.id, "Is it warm here?")

    def get_asset_data(self):
        try:
            value = str(self.ai.read_value())
        except Exception as excpt:
            self.log.exception("Error getting asset data: %s.", excpt)
            value = -1000

        return value

    def log_sensor_data(self, data, virtual):
        if not virtual:
            try:
                self.push_data(self.asset.name, self.asset.context, self.asset.value,
                               self.asset.unit)
            except Exception as excpt:
                self.log.exception("Error logging sensor data: %s.", excpt)
        else:
            # For virtual assets, assume that the data is already parsed JSON
            unit_symbol = {
                'temp_c': 'C',
                'relative_humidity': '%',
                'pressure_mb': 'mb',
            }
            try:
                for factor in ('temp_c', 'relative_humidity', 'pressure_mb'):
                    value = str(data[factor]).replace("%", "")
                    self.push_data(factor, "Environment", value, unit_symbol[factor])

            except Exception as excpt:
                self.log.exception("Error logging sensor data: %s.", excpt)

    def push_data(self, asset_name, asset_context, value, unit):
        try:
            conn = self.connect_influx(asset_context)
            json_body = [
                {
                    "measurement": asset_context,
                    "tags": {
                        "site": self.name,
                        "asset": asset_name
                    },
                    "time": str(datetime.datetime.now()),
                    "fields": {
                        "value": value,
                        "unit": unit
                    }
                }
            ]
            conn.write_points(json_body)
            self.log.info("Wrote to analytic database: %s." % json_body)
        except Exception as excpt:
            self.log.exception("Error writing to analytic database: %s.", excpt)

    def get_weather(self):
        response = ""
        url = (
            'http://api.wunderground.com/'
            'api/{key}/conditions/q/{lat},{lon}.json'
        ).format(
            key=self.wunder_key,
            lat=self.latitude,
            lon=self.longitude,
        )

        try:
            f = urllib2.urlopen(url)
            json_string = f.read()
            parsed_json = json.loads(json_string)
            response = parsed_json['current_observation']
            f.close()
        except Exception as excpt:
            self.log.exception("Error getting weather data: %s.", excpt)
        return response

    def log_command(self, job, result):
        try:
            now = str(datetime.datetime.now())
            command = '''
                INSERT INTO command_log (timestamp, command, result)
                VALUES (?, ?, ?)
            ''', (now, job.name, result)
            self.log.info("Executed %s.", job.name)
            database = sqlite3.connect(utilities.DB_HIST)
            database.cursor().execute(*command)
            database.commit()
        except Exception as excpt:
            self.log.exception("Error logging command: %s.", excpt)
        finally:
            database.close()

    def get_env(self):
        now = datetime.datetime.now()
        uptime = now - self.launch_time
        days = uptime.days
        minutes, seconds = divmod(uptime.seconds, utilities.SECONDS_PER_MINUTE)
        hours, minutes = divmod(minutes, utilities.MINUTES_PER_HOUR)
        s = ('''
            Smart Module Status
              Software Version v{version}
              Running on: {platform}
              Encoding: {encoding}
              Python Information
               - Executable: {executable}
               - v{sys_version}
               - location: {executable}
              Timestamp: {timestamp}
              Uptime: This Smart Module has been online for '''
                  '''{days} days, {hours} hours, {minutes} minutes and {seconds} seconds.
        ''').format(
            version=utilities.VERSION,
            platform=sys.platform,
            encoding=sys.getdefaultencoding(),
            executable=sys.executable,
            sys_version=sys.version.split()[0],
            timestamp=now.strftime('%Y-%m-%d %H:%M:%S'),
            days=days,
            hours=hours,
            minutes=minutes,
            seconds=seconds,
        )
        s = utilities.trim(s) + '\n'
        try:
            self.comm.send("ENV/RESPONSE", s)
        except Exception as excpt:
            self.log.exception("Error getting environment data: %s.", excpt)

class Scheduler(object):
    def __init__(self):
        self.running = True
        self.smart_module = None
        self.processes = []
        self.log = log.Log("scheduler.log")

    class Job(object):
        def __init__(self):
            self.id = -1
            self.name = ""
            self.asset_id = ""
            self.command = ""
            self.time_unit = ""
            self.interval = -1
            self.at_time = ""
            self.enabled = False
            self.sequence = ""
            self.timeout = 0.0
            self.virtual = False

    def process_sequence(self, seq_jobs, job, job_rtu, seq_result):
        for row in seq_jobs:
            name, command, step_name, timeout = row
            seq_result.put(
                'Running {name}:{step_name}({command}) on {id} at {address}.'.
                format(
                    name=name,
                    step_name=step_name,
                    command=job.command,
                    id=job.rtuid,
                    address=job_rtu.address,
                )
            )
            command = command.encode("ascii")
            timeout = int(timeout)
            self.site.comm.send("COMMAND/" + job.rtuid, command)
            time.sleep(timeout)

    def load_schedule(self):
        jobs = []
        self.log.info("Loading Schedule Data...")
        field_names = '''
            id
            name
            asset_id
            command
            time_unit
            interval
            at_time
            enabled
            sequence
            virtual
        '''.split()
        try:
            sql = 'SELECT {fields} FROM schedule;'.format(
                fields=', '.join(field_names))
            database = sqlite3.connect(utilities.DB_CORE)
            db_jobs = database.cursor().execute(sql)
            for row in db_jobs:
                job = Scheduler.Job()
                for field_name, field_value in zip(field_names, row):
                    setattr(job, field_name, field_value)
                jobs.append(job)
            self.log.info("Schedule Data Loaded.")
        except Exception as excpt:
            self.log.exception("Error loading schedule. %s.", excpt)
        finally:
            database.close()

        return jobs

    def prepare_jobs(self, jobs):
        # It still has space for improvements.
        suffixed_names = {
            # 'year': 'yearly',  # Not supported by schedule library.
            # 'month': 'monthly',  # Not supported by schedule library.
            'week': 'weekly',
            'day': 'daily',
            'hour': 'hourly',
            'minute': 'minutes',  # What would be better? Need to change log string also?
            'second': 'seconds',  # What would be better? Need to change log string also?
        }
        for job in jobs:
            if not job.enabled:
                continue

            interval_name = job.time_unit.lower()
            if job.interval > -1:  # What does -1 mean?
                plural_interval_name = interval_name + 's'
                d = getattr(schedule.every(job.interval), plural_interval_name)
                d.do(self.run_job, job)
                self.log.info("  Loading %s job: %s.", suffixed_names[interval_name], job.name)
            elif interval_name == 'day':
                schedule.every().day.at(job.at_time).do(self.run_job, job)
                self.log.info("  Loading time-based job: " + job.name)
            else:
                d = getattr(schedule.every(), interval_name)
                d.do(self.run_job, job)
                self.log.info("  Loading %s job: %s", interval_name, job.name)

    def run_job(self, job):
        if not self.running or not job.enabled:
            return

        response = ""
        job_rtu = None

        if job.sequence is None:
            job.sequence = ""

        if job.virtual:
            print('Running virtual job:', job.name, job.command)
            try:
                response = eval(job.command)
                self.smart_module.log_sensor_data(response, True)
            except Exception as excpt:
                self.log.exception("Error running job. %s.", excpt)
        else:
            try:
                if job.sequence != "":
                    print('Running sequence', job.sequence)
                    command = '''
                        SELECT name, command, step_name, timeout
                        FROM sequence
                        WHERE name=?
                        ORDER BY step ;
                    ''', (job.sequence,)
                    database = sqlite3.connect(utilities.DB_CORE)
                    seq_jobs = self.database.cursor().execute(*command)
                    #print('len(seq_jobs) =', len(seq_jobs))
                    p = Process(target=self.process_sequence, args=(seq_jobs, job, job_rtu,
                                                                    seq_result,))
                    p.start()
                    database.close()
                else:
                    print('Running command', job.command)
                    # Check pre-defined jobs
                    if job.name == "Log Data":
                        self.site.comm.send("QUERY/#", "query")
                        # self.site.log_sensor_data(response, False, self.logger)

                    elif job.name == "Log Status":
                        self.site.comm.send("REPORT/#", "report")

                    else:
                        eval(job.command)
                        # if job_rtu is not None:  #??? job_rtu is always None. Bug?
                        #     self.site.comm.send("COMMAND/" + job.rtuid, job.command)

                    #self.log_command(job, "")

            except Exception as excpt:
                self.log.exception("Error running job: %s.", excpt)

class DataSync(object):
    log = log.Log("datasync.log")
    @staticmethod
    def read_db_version():
        version = ""
        try:
            database = sqlite3.connect(utilities.DB_CORE)
            sql = "SELECT data_version FROM db_info;"
            data = database.cursor().execute(sql)
            for element in data:
                version = element[0]
            DataSync.log.info("Read database version: %s.", version)
            return version
        except Exception as excpt:
            DataSync.log.exception("Error reading database version: %s.", excpt)
        finally:
            database.close()

    @staticmethod
    def write_db_version():
        try:
            version = datetime.datetime.now().isoformat()
            command = "UPDATE db_info SET data_version = ?;", (version,)
            database = sqlite3.connect(utilities.DB_CORE)
            database.cursor().execute(*command)
            database.commit()
            DataSync.log.info("Wrote database version: %s.", version)
        except Exception as excpt:
            DataSync.log.exception("Error writing database version: %s.", excpt)
        finally:
            database.close()

    @staticmethod
    def publish_core_db(comm):
        try:
            output = "output.sql"
            command = "sqlite3 {database} .dump > {output}".format(
                database=utilities.DB_CORE,
                output=output,
            )
            subprocess.call(command, shell=True)
            f = codecs.open(output, 'r', encoding='ISO-8859-1')
            data = f.read()
            byteArray = bytearray(data.encode('utf-8'))
            comm.unsubscribe("SYNCHRONIZE/DATA")
            comm.send("SYNCHRONIZE/DATA", byteArray)
            #comm.subscribe("SYNCHRONIZE/DATA")
            DataSync.log.info("Published database.")
        except Exception as excpt:
            DataSync.log.exception("Error publishing database: %s.", excpt)

    def synchronize_core_db(self, data):
        try:
            incoming = "incoming.sql"
            with codecs.open(incoming, "w") as fd:
                fd.write(data)

            command = 'sqlite3 -init {file} hapi_new.db ""'.format(file=incoming)
            subprocess.call(command, shell=True)

            DataSync.log.info("Synchronized database.")
        except Exception as excpt:
            DataSync.log.exception("Error synchronizing database: %s.", excpt)

def main():
    #max_log_size = 1000000

    # # Setup Logging
    # logger_level = logging.DEBUG
    # logger = logging.getLogger(utilities.SM_LOGGER)
    # logger.setLevel(logger_level)
    #
    # # create logging file handler
    # file_handler = logging.FileHandler('hapi_sm.log', 'a')
    # file_handler.setLevel(logger_level)
    #
    # # create logging console handler
    # console_handler = logging.StreamHandler()
    # console_handler.setLevel(logger_level)
    #
    # #Set logging format
    # formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    # file_handler.setFormatter(formatter)
    # console_handler.setFormatter(formatter)
    # logger.addHandler(file_handler)
    # logger.addHandler(console_handler)

    logging = log.Log("smartmodule.log")
    try:
        smart_module = SmartModule()
        smart_module.discover()
        smart_module.load_site_data()

    except Exception as excpt:
        logging.exception("Error initializing Smart Module. %s.", excpt)

    while 1:
        try:
            time.sleep(0.5)
            schedule.run_pending()

        except Exception as excpt:
            logging.exception("Error in Smart Module main loop. %s.", excpt)
            break

if __name__ == "__main__":
    main()
