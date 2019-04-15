import time,gc
from pyb import I2C;from pyb import LED; 
i2c=I2C(3,I2C.MASTER,baudrate=10000); 
r=LED(LED.RED); g=LED(LED.GREEN); b=LED(LED.BLUE)
def i2c_conf():
	i2c.mem_write(0,0x1E,0x2A); 
	i2c.mem_write(0x01,0x1E,0x0E); 
	i2c.mem_write(0x0d,0x1E,0x2A);
i2c_conf()
sensorRange = i2c.mem_read(1,0x1E,0x0e)[0]
dataScale = 0
if sensorRange == 0x00:
	dataScale = 2
if sensorRange == 0x01:
	dataScale = 4
if sensorRange == 0x10:
	dataScale = 8
UPPER = 60
LOWER = 30
AXIES = 3;
while(True):
	# get all the accl status
	status = i2c.mem_read(2*AXIES+1,0x1E,0x00)
	# xData yield
	xData = (status[1]<<8 | status[2]) 
	print("%x\n"%xData)
	if xData & 0x8000:
		xData = xData - (1<<16)
	xData = xData / 4
	# yData yield
	yData = (status[3]<<8 | status[4])
	print("%x\n"%yData)
	if yData & 0x8000:
		yData = yData - (1<<16)
	yData = yData / 4
	# zData yield
	zData = (status[5]<<8 | status[6])
	if zData & 0x8000:
		zData = zData - (1<<16)
	zData = zData / 4     #the acc have only 8/14-bit data to store the value, no need to use all the 16bits. maybe the MSB is enough
	# The Angle group
	xAngle = xData * dataScale * 90 / 8192
	yAngle = yData * dataScale * 90 / 8192
	zAngle = zData * dataScale * 90 / 8192
	#- to +/ (0-90)
	if xAngle < 0:
		xAngle = -1 * xAngle
	if yAngle < 0:
		yAngle = -1 * yAngle
	if zAngle < 0:
		zAngle = -1 * zAngle	
	# control the led
	if xAngle > UPPER:
		r.on();
	if yAngle > UPPER:
		g.on();
	if zAngle > UPPER:
		b.on();
	if xAngle < LOWER:
		r.off();
	if yAngle < LOWER:
		g.off();
	if zAngle < LOWER:
		b.off();
	print("xAngle is %.5f, yAngle is %.5f, zAngle is %.5f"%(xAngle,yAngle,zAngle))
	time.sleep(10)
	gc.collect()
