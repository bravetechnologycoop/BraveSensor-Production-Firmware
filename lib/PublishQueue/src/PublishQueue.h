#ifndef __PUBLISH_QUEUE_H_
#define __PUBLISH_QUEUE_H_

#include "application.h"
#include <queue>

typedef struct node {
    String eventName;
    String data;
    struct node *next;
};


class PublishQueue {
public:
    /**
    * Constructor.
    */
    PublishQueue(unsigned int  intervalMillis); //Time 

    /**
    * Publish an event
    */    
    void Publish(String eventName, String data);
    void Process(); //Process any pending request that was not previously processed due to limits. This method must be called regularly.
    void SerialDumpList();
    void Debug(String str);
    
private:
    std::queue<node> my_queue;
    unsigned int _intervalMillis; //the minimum interval between publishing events
    unsigned long lastMillis; //last time we published
    bool _IsReadyToProcess();

};
#endif /* __PUBLISH_QUEUE_H_ */
