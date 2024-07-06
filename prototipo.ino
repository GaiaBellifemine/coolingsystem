#include <OneWire.h>
#include <DallasTemperature.h>
#include <FanController.h>

#define ONE_WIRE_BUS 3 // PIN DATA del sensore di temperatura DS18B20
#define MOTOR_PIN_ENA 5  // PIN ENA del driver motore L298N
#define MOTOR_IN1 7 // PIN IN1 del driver motore L298N
#define MOTOR_IN2 8 // PIN IN2 del driver motore L298N
#define MOTOR_PWM 64 // Inizializzazione a PWM=64 riscaldare driver (e di conseguenza i resistori di potenza)
#define FAN_PWM 9 // PIN PWM ventola Noctua 12V
#define FAN_TACHIMETRO 2 // PIN TACHIMETRO ventola Noctua 12V (RPM Speed Signal - E30 - B26)
#define SENSOR_THRESHOLD 500 // THRESHOLD lettura velocita' (ogni quanto tempo in ms viene letta la velocità dalla ventola)

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensor(&oneWire);
FanController fan(FAN_TACHIMETRO, SENSOR_THRESHOLD, FAN_PWM);

void setup(void)
{
	/* Inizializzazione porta seriale */
	Serial.begin(9600); 
	
	/* Configurazione pin */
	pinMode(FAN_PWM, OUTPUT);
	pinMode(MOTOR_IN1, OUTPUT);
	pinMode(MOTOR_IN2, OUTPUT);
    pinMode(MOTOR_PIN_ENA, OUTPUT);
	
	/* Applicazione PWM ai resistori di potenza per riscaldarli */
	analogWrite(MOTOR_PIN_ENA, MOTOR_PWM);
	
	/* Applicazione PWM alla ventola in modo che parte da spenta */
	analogWrite(FAN_PWM, 0);
	
	/* Quando IN1 è LOW e IN2 HIGH, il motore ruota in senso orario. 
	In questo modo direzioniamo il "verso" della corrente, permettendo il surriscaldamento del driver */
	digitalWrite(MOTOR_IN1, LOW);
	digitalWrite(MOTOR_IN2, HIGH);
	
	/* Inizializzazione del sensore e della ventola. */
	sensor.begin();
    fan.begin();
	delay(1000);
}

void loop(void)
{ 
	delay(500);

	/* Acquisizione e stampa/invio su LV della temperatura dal sensore */
	getTemperatureByDS18B20();

	if(Serial.available()>0) // si verifica che la seriale abbia ricevuto un numero di bytes superiori allo 0
	{
		// se la seriale e' disponibile, allora: 
		
		/* Acquisizione del valore di PWM calcolato da LV */
		float fan_PWM_LV = getPWMfromLV();

		if(fan_PWM_LV!=999.00) // si controlla se LV ha inviato un PWM diverso da 999
		{
			// se il PWM e' diverso da 999, allora: 

			/* il PWM acquisito da LV viene applicato alla ventola per alterarne la velocita' */
			analogWrite(FAN_PWM, map(fan_PWM_LV,0,100,0,255));
			unsigned int fan_RPM = fan.getSpeed();
			Serial.println(fan_RPM);
		} 
		else 
		{
			// se il PWM e' uguale da 999, allora: 
			
			/* Si blocca il processo di surriscaldamento dei resistori e si ferma la ventola */
			analogWrite(MOTOR_PIN_ENA,0);
			analogWrite(FAN_PWM,0);
		}
	}
}

/* Metodo per l'acquisizione e stampa/invio su LV della temperatura dal sensore */
void getTemperatureByDS18B20()
{
	sensor.requestTemperatures();
	float temperature = sensor.getTempCByIndex(0);
	
	if(temperature != DEVICE_DISCONNECTED_C) 
	{
		Serial.println(temperature, 4);
	} 
}

/* Metodo per l'acquisizione del valore di PWM calcolato da LV */
float getPWMfromLV()
{
	float fan_PWM_LV = Serial.parseFloat(); 
	return fan_PWM_LV;
}