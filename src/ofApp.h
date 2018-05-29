#define RUNWAY

#ifdef RUNWAY
#include "ofxJson.h"
#endif



#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxLearn.h"
#include "chainEvent.h"
#include "user.h"

#define MAX_USERS 1
#define NUM_DRINKS 1




class ofApp : public ofBaseApp {
    
public:
    
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void reset();
    
    
    ofxOscReceiver r;
    user theUser;
    
    bool isLearning = false;
    bool stopLearning = false;
    
    // optional: callback function for when training is done
    void callbackTrainingDone() {
        ofLog(OF_LOG_NOTICE, "Training done!!!");
    }
    
    ofxLearnSVMThreaded classifier;
    
    int numPoses = 0;
    
    vector<int> poseMap;
    
    vector<double> getSample() {
        vector<double>sample;
        sum_of_elements = 0.0;
        
        for (auto p : theUser.points) {
            sample.push_back(p.x);
            sum_of_elements += sample.back();
            sample.push_back(p.y);
            sum_of_elements += sample.back();
        }
        
        return sample;
    }
    vector<vector<double>>currentPose;
    ChainEvent chainevent;
    
    int drink = -1;
    double sum_of_elements;
    //vector<vector<user>>fakeUsers;
    vector<string>parts;
    ofFbo learnedPoses;
    bool isFrameNew = false;
    int pose = 0;
};
