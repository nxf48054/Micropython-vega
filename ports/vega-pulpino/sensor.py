from pyb import I2C;from pyb import LED; import gc;import time; i2c=I2C(3,I2C.MASTER,baudrate=10000); r=LED(LED.RED); g=LED(LED.GREEN); b=LED(LED.BLUE);i2c.mem_write(0,0x1E,0x2A);i2c.mem_write(0x01,0x1E,0x0E); i2c.mem_write(0x0d,0x1E,0x2A);sensorRange = i2c.mem_read(1,0x1E,0x0e);dataScale = 4;gc.disable()
while(True):
	status = i2c.mem_read(5,0x1E,0x00);xData = (status[1]<<8 | status[2])>>2;yData = (status[3]<<8 | status[4])>>2;xAngle = xData * dataScale * 90 / (65536);yAngle = yData * dataScale * 90 / (65536);
	if xAngle < 0:
		xAngle = -1 * xAngle;
	if yAngle < 0:
		yAngle = -1 * xAngle;
    if xAngle > 70:
		r.on();
	if yAngle > 70:
		g.on();
	if xAngle < 10:
		r.off();
	if yAngle < 10:
		g.off();
    gc.collect()
    
    
	
