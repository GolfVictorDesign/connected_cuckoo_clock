# Connected cuckoo clock

## Rationale

The goal of this project is to maintain my development skills and capabilities in embedded projects, 
and to implement technologies I don't master yet but I think are worth to learn in current tecnology context.

These technologies are:

* EInk displays
* Field Oreineted Control for brusless motor
* I2S

I have chosen to use ESP-IDF as it is a more professional oriented developement plateform than Arduino, 
and it offers more flexibilty in the design choises.

I also want to use a "standard" quality process for software development to prove the effectiveness of the development.

- Define functional specifications
- Develop/implement the required functionalities
- Verify the functionalities

## Users/system requirements

The cuckoo clock shall provide the following functionalities:

* The cuckoo clock shall be able to operate on batteries.
	* The system be saving power as far as possible, to maximize the battery charge time interval.
	* The system shall use lithum bateries to store as much energy as possible
		* Battery management shall be used to prevent battery failure
		
* The cuckoo clock shall use an E-ink display to save power 
	* The E-ink display shall allow partial refresh to allow pleasant update of information
	
* The cuckoo clock shall provide the following displays on the front display:
	* Time in:
		* Numeric format
		* Analog format
	* Date
	* Weather forcast view
		* The weather forcast of the day shall be reprensented with a pictogram
			* Sun
			* Partial sun / cloudy
			* Overcast
			* Rain
			* Snow
		* Along with the weather pictogram, shall be displayed the 
			* Inside and outside temperature
			* Atmospheric presure at sea-level
			* Outside humidity		
	* The item that are displayed on the front pannel shall be parameterable by the user.
	* The front display shall be lighted with a front-light for low light condition
		* The front-light intesity shall be inversaly proportional to the ambiant light  
	
* The cuckoo clock shall have mooving figurines
	