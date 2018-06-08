





#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "chainEvent.h"
#include "user.h"
#include "ofxGui.h"
#include "learner.h"
#include "feedback.h"
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
    void exit();
    
    
    
    
    float getAngle(ofVec2f p1, ofVec2f p2){
        return float(atan2(p2.y - p1.y, p2.x - p1.x) * 180 / PI);
    }
    ofVec2f getMean(ofVec2f p1, ofVec2f p2){
        return (p1+p2)/2.f;
    }
    GRT::VectorFloat getSample(user u) {
        GRT::VectorFloat sample(18*2);
        int indx = 0;
        for (auto p : u.points) {
            sample[indx] = p.x;
            sample[indx+1] = p.y;
            indx+=2;
        }
        return sample;
    }


    
    ofxPanel gui;
    ofParameter<float>left, right, top, bottom, bellyThreshold, prababilityThreshold;
    ofParameter<int>yellow_box, red_box;
    ofParameter<bool>clearSample, addSamples, train;
    ofParameterGroup bodyGroup;
    ofParameter<ofVec2f>torso, head;
    ofParameter<float>s_torso, s_head;
    
    vector<ofParameter<ofVec2f>> pos_parts;
    vector<ofParameter<float>> s_parts;
    vector<user> theUsers;
    vector<int> drinkSequence;
    vector<ofImage>poseImages;
    vector<string>parts;
    vector<vector<ofVec2f>>averagePoses;
    vector<ofImage> bodyPartImages;
    ofImage logo, backgound, speech, speech_red;
    
    user bartender;
    GestureLearner classifier;
    ofxOscReceiver r;
    ChainEvent chainevent;
    Feedback feedback;
    
    void drawUserWithPngs(vector<ofVec2f> p, int pngs);
	vector<vector<int>> indxes = { { 4,3,11 },{ 3,2,0 },{ 5,6,0 },{ 7,6,12 },{ 8,9,0 },{ 10,9,9 },{ 11,12,0 },{ 13,12,10 } };

	void pose(int _user, int _posenum);

    bool drawUserOrbartender;
    bool debug = false;
    bool isFrameNew = false;
    
    
    ofFbo learnedPoses;
    ofFbo render;
    
    ofxBox2d box2d;

    int currentDrinkSequence = 0;
    int numPoses = 0;
    int numHumans;
    int numHumansInView;
    int numSamples;
    int session = 0;
    
	bool jumpToNext = false;

    ofShader yellowShader;
    
    // Serial
    ofSerial    serial;
    bool        bSendSerialMessage;            // a flag for sending serial
    char        bytesRead[3];                // data from serial, we will be trying to read 3
    char        bytesReadString[4];            // a string needs a null terminator, so we need 3 + 1 bytes
    int            nBytesRead;                    // how much did we read?
    int            nTimesRead;                    // how many times did we read?
    float        readTime;                    // when did we last read?
    
    void readArduino();
    void echoArduino();
    bool echo = false;
    double echoTimer = 0.0;
    int deviceCount = 0;
    
    
};
