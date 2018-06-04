





#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "chainEvent.h"
#include "user.h"
#include "ofxGui.h"
#include "learner.h"
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
    
    int session = 0;
    void drawUserWithPngs(user* u);
	vector<ofImage> bodyPartImages;
	
    
    float getAngle(ofVec2f p1, ofVec2f p2){
        return float(atan2(p2.y - p1.y, p2.x - p1.x) * 180 / PI);
    }
    ofVec2f getMean(ofVec2f p1, ofVec2f p2){
        return (p1+p2)/2.f;
    }

    GestureLearner classifier;
    ofxOscReceiver r;
    vector<user> theUsers;
    ofxPanel gui;
    ofParameter<float>left, right, top, bottom, scale, bellyThreshold, prababilityThreshold;
    ofParameter<bool>clearSample, addSamples, train;


     GRT::VectorFloat getSample(user u) {

        GRT::VectorFloat sample;
        int indx = 0;
        for (auto p : u.points) {
            sample[indx] = p.x;
            sample[indx+1] = p.y;
            indx+=2;
        }
        return sample;
    }

    ChainEvent chainevent;
    
    vector<int> drinkSequence;
    int currentDrinkSequence = 0;
    int numPoses = 0;
    int numHumans;
    int numHumansInView;
    vector<ofImage>poseImages;
    
    vector<string>parts;
    ofFbo learnedPoses;
    bool isFrameNew = false;

	ofFbo userFbo;
	ofFbo feedBackFbo;
	
	ofTrueTypeFont font;

	ofxBox2d box2d;
    
    vector<vector<double>>averagePoses;
    int numSamples;
    
};
