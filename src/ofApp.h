

#ifdef __APPLE__
#include "ofxJson.h"
#endif



#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxLearn.h"
#include "chainEvent.h"
#include "user.h"

#define MAX_USERS 3
#define NUM_DRINKS 1




class ofApp : public ofBaseApp {
    
public:
	ofVideoGrabber test;
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
    vector<user> theUsers;
    
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

        for (auto p : theUsers[0].points) {
            sample.push_back(p.x);
            sample.push_back(p.y);

        }
        return sample;
    }
    vector<vector<double>>currentPose;
    ChainEvent chainevent;
    
    int drink = -1;

    //vector<vector<user>>fakeUsers;
    vector<string>parts;
    ofFbo learnedPoses;
    bool isFrameNew = false;
    int pose = 0;

	ofFbo userFbo;
	ofFbo feedBackFbo;
	int numHumnas;
	ofTrueTypeFont font;
	vector<ofImage>poseImages;

	ofxBox2d box2d;
};
