# Qucik test file to open TSY01 connection and reads its temp
# 20160819 IMD
#
#


import TSYS01

sensor=TSYS01.TSYS01(0x66)
sensor1=TSYS01.TSYS01(0x67)
sensor2=TSYS01.TSYS01(0x78)
sensor3=TSYS01.TSYS01(0x79)
print 'Temperature = %3.3f C' % sensor.readTemp()
print 'Temperature1 = %3.3f C' % sensor1.readTemp()
print 'Temperature2 = %3.3f C' % sensor2.readTemp()
print 'Temperature3 = %3.3f C' % sensor3.readTemp()