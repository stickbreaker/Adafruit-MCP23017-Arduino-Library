/*************************************************** 
 This is a library for the MCP23017 i2c port expander

 These displays use I2C to communicate, 2 pins are required to
 interface
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 Written by Limor Fried/Ladyada for Adafruit Industries.
 BSD license, all text above must be included in any redistribution
 ****************************************************/

#ifdef __AVR
  #include <avr/pgmspace.h>
#elif defined(ESP8266)
  #include <pgmspace.h>
#endif
#include "Adafruit_MCP23017.h"

#include "Arduino.h"

/**
 * Bit number associated to a give Pin
 */
uint8_t Adafruit_MCP23017::bitForPin(uint8_t pin){
	return pin%8;
}

/**
 * Register address, port dependent, for a given PIN
 */
uint8_t Adafruit_MCP23017::regForPin(uint8_t pin, uint8_t portAaddr, uint8_t portBaddr){
	return(pin<8) ?portAaddr:portBaddr;
}

/**
 * Reads a given register
 */
uint8_t Adafruit_MCP23017::readRegister(uint8_t addr){
	// read the current GPINTEN
	_Wire->beginTransmission(MCP23017_ADDRESS | i2caddr);
	_Wire->write(addr);
	_Wire->endTransmission();
    if (_Wire->lastError() == 0) {
        _Wire->requestFrom(MCP23017_ADDRESS | i2caddr, 1);
        if(_Wire->lastError() ==0) {
            return _Wire->read();
        }
        else {
            log_e("Read (0x%02x) Failed reg(%d) %d(%s)\n",MCP23017_ADDRESS | i2caddr,addr, _Wire->lastError(),_Wire->getErrorText(_Wire->lastError()));
            return 0;
        }
    }
    else {
        log_e("Read (0x%02x) set Register reg(%d) %d(%s)\n",MCP23017_ADDRESS | i2caddr,addr, _Wire->lastError(),_Wire->getErrorText(_Wire->lastError()));
        return 0;
    }
}


/**
 * Writes a given register
 */
void Adafruit_MCP23017::writeRegister(uint8_t regAddr, uint8_t regValue){
	// Write the register
	_Wire->beginTransmission(MCP23017_ADDRESS | i2caddr);
	_Wire->write(regAddr);
	_Wire->write(regValue);
	_Wire->endTransmission();
    if(_Wire->lastError() != 0){
        log_e("failed(0x%x) reg(0x%x)=%d error=%d(%s)\n",MCP23017_ADDRESS | i2caddr, regaddr,regvalue,_Wire->lastError(),_Wire->getErrorText(_Wire->lastError()));
    }
}


/**
 * Helper to update a single bit of an A/B register.
 * - Reads the current register value
 * - Writes the new register value
 */
void Adafruit_MCP23017::updateRegisterBit(uint8_t pin, uint8_t pValue, uint8_t portAaddr, uint8_t portBaddr) {
	uint8_t regValue;
	uint8_t regAddr=regForPin(pin,portAaddr,portBaddr);
	uint8_t bit=bitForPin(pin);
	regValue = readRegister(regAddr);

	// set the value for the particular bit
	bitWrite(regValue,bit,pValue);

	writeRegister(regAddr,regValue);
}

////////////////////////////////////////////////////////////////////////////////

/**
 * Initializes the MCP23017 given its HW selected address, see datasheet for Address selection.
 */
void Adafruit_MCP23017::begin(uint8_t addr, TwoWire * xWire) {
    i2caddr = addr & 0x7; //restrict, but allow 0x20 .. 0x27 or 0x0 .. 0x7
    _Wire = xWire;
	_Wire->begin();

	// set defaults!
	// all inputs on port A and B
	writeRegister(MCP23017_IODIRA,0xff);
	writeRegister(MCP23017_IODIRB,0xff);
}
/**
 * Initializes the MCP23017 given its HW selected address, see datasheet for Address selection.
 */
void Adafruit_MCP23017::begin(uint8_t addr) {
    begin(addr,&Wire);
}

/**
 * Initializes the default MCP23017, with 000 for the configurable part of the address
 */
void Adafruit_MCP23017::begin(void) {
	begin(0,&Wire);
}

/**
 * Sets the pin mode to either INPUT or OUTPUT
 */
void Adafruit_MCP23017::pinMode(uint8_t p, uint8_t d) {
	updateRegisterBit(p,(d==INPUT),MCP23017_IODIRA,MCP23017_IODIRB);
}

/**
 * Reads all 16 pins (port A and B) into a single 16 bits variable.
 */
uint16_t Adafruit_MCP23017::readGPIOAB() {
	uint16_t ba = 0;
	uint8_t a;

	// read the current GPIO output latches
	_Wire->beginTransmission(MCP23017_ADDRESS | i2caddr);
	_Wire->write(MCP23017_GPIOA);
	_Wire->endTransmission();
    if(_Wire->lastError()==0){
        _Wire->requestFrom(MCP23017_ADDRESS | i2caddr, 2);
        if(_Wire->lastError()==0){
            a = _Wire->read();
            ba = _Wire->read();
            ba <<= 8;
            ba |= a;
        }
        else {
            log_e("request From(0x%02x) failed %d(%s)\n",MCP23017_ADDRESS | i2caddr,_Wire->lastError(),_Wire->getErrorText(_Wire->lastError()));
        }
    }
    else {
        log_e("request From(0x%02x) set reg failed %d(%s)\n",MCP23017_ADDRESS | i2caddr,_Wire->lastError(),_Wire->getErrorText(_Wire->lastError()));
    }
    
	return ba;
}

/**
 * Read a single port, A or B, and return its current 8 bit value.
 * Parameter b should be 0 for GPIOA, and 1 for GPIOB.
 */
uint8_t Adafruit_MCP23017::readGPIO(uint8_t b) {

	// read the current GPIO output latches
	_Wire->beginTransmission(MCP23017_ADDRESS | i2caddr);
	if (b == 0)
		_Wire->write(MCP23017_GPIOA);
	else {
		_Wire->write(MCP23017_GPIOB);
	}
	_Wire->endTransmission();
    if(_Wire->lastError()==0){
        _Wire->requestFrom(MCP23017_ADDRESS | i2caddr, 1);
        if(_Wire->lastError()==0){
            return _Wire->read();
        }
        else {
            log_e("request gpio(0x%02x):%d failed %d(%s)\n",MCP23017_ADDRESS | i2caddr,b,_Wire->lastError(),_Wire->getErrorText(_Wire->lastError()));
            return 0;
        }
    }
    else {
        log_e("request gpio(0x%02x):d set reg failed %d(%s)\n",MCP23017_ADDRESS | i2caddr,b,_Wire->lastError(),_Wire->getErrorText(_Wire->lastError()));
        return 0;
    }
}

/**
 * Writes all the pins in one go. This method is very useful if you are implementing a multiplexed matrix and want to get a decent refresh rate.
 */
void Adafruit_MCP23017::writeGPIOAB(uint16_t ba) {
	_Wire->beginTransmission(MCP23017_ADDRESS | i2caddr);
	_Wire->write(MCP23017_GPIOA);
	_Wire->write(ba & 0xFF);
	_Wire->write(ba >> 8);
	_Wire->endTransmission();
    if(_Wire->lastError()!=0){
        log_e("writeGPIOAB(0x%02x):0x%04x failed %d(%s)\n",MCP23017_ADDRESS | i2caddr,ba,_Wire->lastError(),_Wire->getErrorText(_Wire->lastError()));
    }
}

void Adafruit_MCP23017::digitalWrite(uint8_t pin, uint8_t d) {
	uint8_t gpio;
	uint8_t bit=bitForPin(pin);


	// read the current GPIO output latches
	uint8_t regAddr=regForPin(pin,MCP23017_OLATA,MCP23017_OLATB);
	gpio = readRegister(regAddr);

	// set the pin and direction
	bitWrite(gpio,bit,d);

	// write the new GPIO
	regAddr=regForPin(pin,MCP23017_GPIOA,MCP23017_GPIOB);
	writeRegister(regAddr,gpio);
}

void Adafruit_MCP23017::pullUp(uint8_t p, uint8_t d) {
	updateRegisterBit(p,d,MCP23017_GPPUA,MCP23017_GPPUB);
}

uint8_t Adafruit_MCP23017::digitalRead(uint8_t pin) {
	uint8_t bit=bitForPin(pin);
	uint8_t regAddr=regForPin(pin,MCP23017_GPIOA,MCP23017_GPIOB);
	return (readRegister(regAddr) >> bit) & 0x1;
}

/**
 * Configures the interrupt system. both port A and B are assigned the same configuration.
 * Mirroring will OR both INTA and INTB pins.
 * Opendrain will set the INT pin to value or open drain.
 * polarity will set LOW or HIGH on interrupt.
 * Default values after Power On Reset are: (false, false, LOW)
 * If you are connecting the INTA/B pin to arduino 2/3, you should configure the interupt handling as FALLING with
 * the default configuration.
 */
void Adafruit_MCP23017::setupInterrupts(uint8_t mirroring, uint8_t openDrain, uint8_t polarity){
	// configure the port A
	uint8_t ioconfValue=readRegister(MCP23017_IOCONA);
	bitWrite(ioconfValue,6,mirroring);
	bitWrite(ioconfValue,2,openDrain);
	bitWrite(ioconfValue,1,polarity);
	writeRegister(MCP23017_IOCONA,ioconfValue);

	// Configure the port B
	ioconfValue=readRegister(MCP23017_IOCONB);
	bitWrite(ioconfValue,6,mirroring);
	bitWrite(ioconfValue,2,openDrain);
	bitWrite(ioconfValue,1,polarity);
	writeRegister(MCP23017_IOCONB,ioconfValue);
}

/**
 * Set's up a pin for interrupt. uses arduino MODEs: CHANGE, FALLING, RISING.
 *
 * Note that the interrupt condition finishes when you read the information about the port / value
 * that caused the interrupt or you read the port itself. Check the datasheet can be confusing.
 *
 */
void Adafruit_MCP23017::setupInterruptPin(uint8_t pin, uint8_t mode) {

	// set the pin interrupt control (0 means change, 1 means compare against given value);
	updateRegisterBit(pin,(mode!=CHANGE),MCP23017_INTCONA,MCP23017_INTCONB);
	// if the mode is not CHANGE, we need to set up a default value, different value triggers interrupt

	// In a RISING interrupt the default value is 0, interrupt is triggered when the pin goes to 1.
	// In a FALLING interrupt the default value is 1, interrupt is triggered when pin goes to 0.
	updateRegisterBit(pin,(mode==FALLING),MCP23017_DEFVALA,MCP23017_DEFVALB);

	// enable the pin for interrupt
	updateRegisterBit(pin,HIGH,MCP23017_GPINTENA,MCP23017_GPINTENB);

}

uint8_t Adafruit_MCP23017::getLastInterruptPin(){
	uint8_t intf;

	// try port A
	intf=readRegister(MCP23017_INTFA);
	for(int i=0;i<8;i++) if (bitRead(intf,i)) return i;

	// try port B
	intf=readRegister(MCP23017_INTFB);
	for(int i=0;i<8;i++) if (bitRead(intf,i)) return i+8;

	return MCP23017_INT_ERR;

}
uint8_t Adafruit_MCP23017::getLastInterruptPinValue(){
	uint8_t intPin=getLastInterruptPin();
	if(intPin!=MCP23017_INT_ERR){
		uint8_t intcapreg=regForPin(intPin,MCP23017_INTCAPA,MCP23017_INTCAPB);
		uint8_t bit=bitForPin(intPin);
		return (readRegister(intcapreg)>>bit) & (0x01);
	}

	return MCP23017_INT_ERR;
}


