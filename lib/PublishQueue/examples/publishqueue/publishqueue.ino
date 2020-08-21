// This #include statement was automatically added by the Particle IDE.
#include "PublishQueue.h"

// This #include statement was automatically added by the Particle IDE.
#include "elapsedMillis/elapsedMillis.h"

//Create a queue that only calls a publish event once per second
PublishQueue pubQueue(1000);

void setup()
{
    Serial.begin(9600);
    
	//inject several events in the queue and then check that they are called in intervals of 1s
    pubQueue.Publish("EventName1","11111");
    pubQueue.Publish("EventName2","22222");
    pubQueue.Publish("EventName3","33333");
    pubQueue.Publish("EventName4","44444");
    pubQueue.Publish("EventName5","55555");
    pubQueue.Publish("EventName6","66666");
    pubQueue.Publish("EventName7","77777");
    pubQueue.Publish("EventName8","88888");
    pubQueue.Publish("EventName9","99999");
    pubQueue.Publish("EventName10","99990");
    pubQueue.Publish("EventName11","99991");
    pubQueue.Publish("EventName12","99992");
    pubQueue.Publish("EventName13","99993");
    pubQueue.Publish("EventName14","99994");
    pubQueue.Publish("EventName15","99995");
    pubQueue.Publish("EventName16","99996");
}

void loop()
{
    pubQueue.Process();
}
