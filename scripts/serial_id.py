"""Return the id of the arduino connected to the first serial port. id 0 is for Temperaturmessung, id 1 for DMSMessung
"""
import time
import serial

if __name__ == "__main__":
    with serial.Serial("/dev/ttyACM0", 9600, timeout=3) as con:
        while True:
            con.write(1)

            try:
                data = con.readline().decode("utf-8").replace('\r','').replace('\n','')
                print(int(float(data)))
                break
            except (TypeError, ValueError):
                time.sleep(1)
