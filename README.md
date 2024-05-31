# Connected cuckoo clock

## Rationale

The goal of this project is to maintain my development skills and capabilities in embedded projects, 
and to implement technologies I don't master yet but I think are worth to learn in current technology context.

These technologies are:

* EInk displays
* Field Oriented Control for brushless motor
* I2S

I have chosen to use ESP-IDF as it is a more professional oriented development plateform than Arduino, 
and it offers more flexibility in the design choices.

I also want to use a "standard" quality process for software development to prove the effectiveness of the development.

- Define functional specifications
- Develop/implement the required functionalities
- Verify the functionalities ( will be done using Unity, integrated to ISP-IDF )

## Users/system requirements

The cuckoo clock shall provide the following functionalities:

* The cuckoo clock shall be able to operate on batteries.
	* The system be saving power as far as possible, to maximize the battery charge time interval.
	* The system shall use lithum batteries to store as much energy as possible
		* Battery management shall be used to prevent battery failure
		
* The cuckoo clock shall have a front display
	* The front display shall allow visibly in all light conditions
	
* The cuckoo clock shall provide the following displays on the front display:
	* Time in:
		* Numeric format
		* Analog format
	* Date
	* Weather forcast view
		* The weather forcast of the day shall be represented with a pictogram
			* Sun
			* Partial sun / cloudy
			* Overcast
			* Rain
			* Snow
		* Along with the weather pictogram, shall be displayed the 
			* Inside and outside temperature
			* Atmospheric pressure at sea-level
			* Outside humidity		
	* The item that are displayed on the front panel shall be parameterized by the user.
	* The front display shall be lighted with a front-light for low light condition
		* The front-light intensity shall be inversely proportional to the ambient light  
	
* The cuckoo clock shall have moving figurines
    * The cuckoo clock shall have a wheel with dancers 
    * The cuckoo clock shall have a woodman chopping woodman with his axe
	
## System architecture

To cover the system requirements, the system shall have the following architecture:

* The cuckoo clock shall use 2 to 4 cells lithum batteries
    * The cuckoo clock shall allow to charge the batteries without danger
    * The battery charging circuit shall allow voltage balancing

* The cuckoo clock front display shall be an E-ink display
    * The E-ink display shall allow partial refresh to allow pleasant update of information
    
* The cuckoo clock shall use an I2C atmospheric pressure sensor to get atmospheric pressure
    * The pressure sensor shall have a temperature sensor
    * The pressure sensor shall be placed as far as possible or should be insulated from any heat source to give the most 
      accurate room temperature

## Software specifications

The following software specification cover system requirements dans design choices.

* The software shall manage the battery
    * The user shall be warned when battery level goes below 30%