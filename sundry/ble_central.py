from bluepy.btle import Peripheral, UUID
from bluepy.btle import Scanner, DefaultDelegate

import struct

class ScanDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)

    def handleDiscovery(self, dev, isNewDev, isNewData):
        if isNewDev:
            print "Discovered device", dev.addr
        elif isNewData:
            print "Received new data from", dev.addr

scanner = Scanner().withDelegate(ScanDelegate())
devices = scanner.scan(10.0)
n=0
for dev in devices:
    print "%d: Device %s (%s), RSSI=%d dB" % (n, dev.addr, dev.addrType, dev.rssi)
    for (adtype, desc, value) in dev.getScanData():
        if value == "FuckYouDontConnect":
            number = n
            print "======================\n\n\tI'm here\n\n======================\n"
    #    print " %s = %s" % (desc, value)
    n += 1

#number = input('Enter your device number: ')
print('Device', number)
print(devices[number].addr)

print "Connecting..."
dev = Peripheral(devices[number].addr, 'random')
print("Services...")
for svc in dev.services:
    print(str(svc))

try:
    ButtonService = dev.getServiceByUUID(UUID(0xa000))
    IDService = dev.getServiceByUUID(UUID(0xb000))
    LEDService = dev.getServiceByUUID(UUID(0xc000))
    ch1 = dev.getCharacteristics(uuid=UUID(0xa001))[0]
    ch2 = dev.getCharacteristics(uuid=UUID(0xb001))[0]
    ch3 = dev.getCharacteristics(uuid=UUID(0xc001))[0]
    while 1:
        if (dev.waitForNotifications(0.5)):
            print "notice !!\n"
        print ('Button Value: ', ch1.read())
        if(ch1.read() == '\x01'):
            print "You pressed the button!"
        print ('ID Value: ', ch2.read())
        LED_value = str(input('Enter LED state: '))
        if LED_value == '1':
            ch3.write(struct.pack('<B', 0x01))
        elif LED_value == '0':
            ch3.write(struct.pack('<B', 0x00))
        else:
            print("Wrong input for LED!")
        print('LED value: ', ch3.read())
finally:
    dev.disconnect() 