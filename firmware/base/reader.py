import serial
import re
import requests
import logging
import json
import argparse

log = logging.getLogger('')
log.setLevel(logging.INFO)

log_format = logging.Formatter('%(asctime)s - %(levelname)-8s - %(message)s')

fh = logging.FileHandler('lora.log')
fh.setFormatter(log_format)
log.addHandler(fh)

ch = logging.StreamHandler()
ch.setFormatter(log_format)
log.addHandler(ch)

log.info("starting")

parser = argparse.ArgumentParser(description="read from feather & post to phant")
parser.add_argument('port', help="port to read from")
args = parser.parse_args()

serial_port=serial.Serial()
serial_port.port=args.port
serial_port.open()

with open("keys.json") as fh:
    keys = json.load(fh)

while True:
    try:
        line = serial_port.readline()
        log.debug(line)
        #Got: BAT:4055 PAN:0 UP:3137
        m = re.match("^Got: BAT:(\d+) PAN:(\d+) UP:(\d+) RSSI: (-?\d+)", line)
        if m is not None:
            batt =  int(m.group(1))
            panel = int(m.group(2))
            uptime = int(m.group(3))
            rssi = int(m.group(4))
            log.info("batt = %d mv, panel = %d mv, uptime = %d ms rssi = %d" % (batt, panel, uptime, rssi))


            r = requests.post(keys["inputUrl"], params = { "batt": batt, "panel": panel, "uptime": uptime, "rssi": rssi, "private_key": keys["privateKey"] })
            log.debug(r.url)
            log.debug(r.status_code)
            log.debug(r.text)
            log.info("posted")
        

    except KeyboardInterrupt:
        break
    except Exception as e:
        log.error(e)
