import serial
import re
import requests
import logging
import json

log = logging.getLogger('')
log.setLevel(logging.DEBUG)

log_format = logging.Formatter('%(asctime)s - %(levelname)-8s - %(message)s')

fh = logging.FileHandler('lora.log')
fh.setFormatter(log_format)
log.addHandler(fh)

ch = logging.StreamHandler()
ch.setFormatter(log_format)
log.addHandler(ch)

log.info("starting")

serial_port=serial.Serial()
serial_port.port='/dev/ttyACM0'
serial_port.open()

with open("keys.json") as fh:
    keys = json.load(fh)

while True:
    line = serial_port.readline()
    log.debug(line)
    #Got: BAT:4055 PAN:0 UP:3137
    m = re.match("^Got: BAT:(\d+) PAN:(\d+) UP:(\d+)", line)
    if m is not None:
        batt =  int(m.group(1))
        panel = int(m.group(2))
        uptime = int(m.group(3))
        log.info("batt = %d mv, panel = %d mv, uptime = %d ms" % (batt, panel, uptime))


        log.info("posting to phant")
        r = requests.post(keys["inputUrl"], params = { "batt": batt, "panel": panel, "uptime": uptime, "private_key": keys["privateKey"] })
        log.info(r.url)
        log.info(r.status_code)
        log.info(r.text)
        log.info("done")


