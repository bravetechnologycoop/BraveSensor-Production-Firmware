/*
    Delays the calling of cloud events (Particle.publish) so it does not pass the limits defined in https://docs.particle.io/reference/firmware/photon/#particle-publish-
    "NOTE: Currently, a device can publish at rate of about 1 event/sec, with bursts of up to 4 allowed in 1 second. Back to back burst of 4 messages will take 4 seconds to recover."
    
    it will record the time of last publish event, and delay the next publish event until a safe time as passed to guarantee the limits above are not hit.

*/

#include "PublishQueue.h"


PublishQueue::PublishQueue(unsigned int  intervalMillis){
    _intervalMillis=intervalMillis;
    
}

//Public Members
void PublishQueue::Publish(String eventName, String data)
{
    node my_node= {.eventName=eventName, .data=data};
    Debug("Adding to queue: "+ my_node.eventName);
    my_queue.push(my_node);
    Process();
}


void PublishQueue::Process()
{
    
    //if (my_queue.size()>0)
    //{Debug("Queue size:  "+String(my_queue.size())+" _IsReadyToProcess():"+String(_IsReadyToProcess()) + " wait="+String(_intervalMillis-(millis()-lastMillis)));}
    
    //If queue is not empty process the next event.
    if (!my_queue.empty() && _IsReadyToProcess()) {
        node my_node=my_queue.front();
        my_queue.pop();
        Debug("Publishing to CLOUD event: "+ my_node.eventName +" "+my_node.data);
        Particle.publish(my_node.eventName, my_node.data);
        lastMillis=millis(); //Update the time we last published
    }
    
}

//Private Members
bool PublishQueue::_IsReadyToProcess()
{
    return (millis()-lastMillis>_intervalMillis);
}

void PublishQueue::Debug(String str)
{
    Serial.println(str); 
    //delay(100);
}



