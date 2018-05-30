#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
#ifdef __APPLE__
    r.setup(57200);
#else
    r.setup(7000);
#endif
	ofBackground(0, 0, 0);
    
    theUser.setup();
    
    poseMap.resize(NUM_DRINKS);
   // fakeUsers.resize(NUM_DRINKS);
    
    chainevent.addEvent(10., BEGIN_LEARNING);
    chainevent.addEvent(3., LEARNING);
    chainevent.addEvent(20., TRAINING, true);
    chainevent.addEvent(2., PLAYING, true);
    chainevent.addEvent(5., HIT);
    chainevent.addEvent(5., POUR);
    chainevent.addEvent(3., RESET);
    
    parts = {"Nose", "Neck", "Right_Shoulder", "Right_Elbow", "Right_Wrist", "Left_Shoulder", "Left_Elbow", "Left_Wrist", "Right_Hip", "Right_Knee", "Right_Ankle", "Left_Hip", "Left_Knee", "Left_Ankle", "Right_Eye", "Left_Eye", "Right_Ear", "Left_Ear", "Background"};
    
    learnedPoses.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
    learnedPoses.begin();
    ofClear(0);
    learnedPoses.end();

	userFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
	userFbo.begin();
	ofClear(0);
	userFbo.end();

}


//--------------------------------------------------------------
void ofApp::update() {
    
    /*
     OFXOSC_TYPE_INT32            = 'i',
     OFXOSC_TYPE_INT64            = 'h',
     OFXOSC_TYPE_FLOAT            = 'f',
     OFXOSC_TYPE_DOUBLE           = 'd',
     OFXOSC_TYPE_STRING           = 's',
     OFXOSC_TYPE_SYMBOL           = 'S',
     OFXOSC_TYPE_CHAR             = 'c',
     OFXOSC_TYPE_MIDI_MESSAGE     = 'm',
     OFXOSC_TYPE_TRUE             = 'T',
     OFXOSC_TYPE_FALSE            = 'F',
     OFXOSC_TYPE_NONE             = 'N',
     OFXOSC_TYPE_TRIGGER          = 'I',
     OFXOSC_TYPE_TIMETAG          = 't',
     OFXOSC_TYPE_BLOB             = 'b',
     OFXOSC_TYPE_RGBA_COLOR       = 'r',
     */
    ofSetWindowTitle(ofToString(ofGetFrameRate()));
    isFrameNew = false;
#ifdef __APPLE__
    while (r.hasWaitingMessages()) {
        ofxOscMessage m;
        r.getNextMessage(m);
        
        string result = m.getArgAsString(0);
        ofxJSONElement json = result;
        const Json::Value& bodyPoints = json["results"];
        
        for(int i = 0; i<MIN(MAX_USERS, bodyPoints["humans"].size()); i++){
            int partIndx = 0;
            
            theUser.clearPoints();
            
            for(int u = 0; u<bodyPoints["humans"][i].size(); u++){
                isFrameNew = true;
                string part = bodyPoints["humans"][i][u][0].asString();
                double x = bodyPoints["humans"][i][u][1].asDouble();
                double y = bodyPoints["humans"][i][u][2].asDouble();
                
                for(int p = partIndx; p<parts.size(); p++){
                    if(parts[p]==part){
                        theUser.addPoint(p , x ,y);
                        partIndx = p;
                        break;
                    }
                }
            }
        }
    }
#else
    while (r.hasWaitingMessages()) {
        ofxOscMessage m;
        r.getNextMessage(m);
        
        // cout << m.getAddress() << endl;
        int id = ofToInt(ofSplitString(m.getAddress(), "person").back());
        if (id<MAX_USERS) {
            
            // cout << m.getNumArgs() << endl;
            int indx = 0;
            for (int i = 0; i < m.getNumArgs(); i += 3) {
                if (m.getArgType(i) == 102) {
                    theUser.addPoint(indx, m.getArgAsFloat(i), m.getArgAsFloat(i + 1));
                    indx++;
                }
            }
        }
    }
#endif
    chainevent.update();
    
    vector<double>sample;
    sample = getSample();
    
    if (sum_of_elements != 0.0) {
        
        
        switch (chainevent.getName()) {
            case BEGIN_LEARNING: {
                
                if (chainevent.getTime() > 5.0) {
                    int drinkBeingLearned = numPoses%NUM_DRINKS;
                   // fakeUsers[drinkBeingLearned].clear();
                    chainevent.next();
                }
                else cout << "get ready for next pose in: " << int(5.0 - chainevent.getTime()) << " sec!" << endl;
                break;
            }
            case LEARNING: {
                
                classifier.addSample(sample, numPoses);
                int drinkBeingLearned = numPoses%NUM_DRINKS;
                //currentPose[drinkBeingLearned].insert(currentPose[drinkBeingLearned].end(), sample.begin(), sample.end());
                if(isFrameNew){
                    //for(int i = 0; i<users.size();i++){
                        //fakeUsers[drinkBeingLearned].push_back(users[i]);
                        ofEnableAlphaBlending();
                        ofSetColor(255-30*numPoses, 30*numPoses, 20*numPoses, 150);
                        ofSetLineWidth(10);
                        learnedPoses.begin();
                        theUser.draw();
                        learnedPoses.end();
                        ofDisableAlphaBlending();
                        ofSetLineWidth(1);
                    //}
                }
                
                if (chainevent.getTime()>chainevent.getDuration() - 0.5) {
                    
                    poseMap[drinkBeingLearned] = numPoses;
                    numPoses++;
                    if (numPoses < 3)chainevent.beginEvents();
                    else {
                        classifier.beginTraining();
                        chainevent.next();
                    }
                }
                break;
            }
                
                
            case TRAINING: {
                //statements
                cout << "TRAINING" << endl;
                if (classifier.isTrained())chainevent.next();
                break;
            }
            case PLAYING: {
                
                //statements
                pose = classifier.predict(sample);
                cout << "pose "<< pose << endl;
                drink = -1;
                for (int i = 0; i < NUM_DRINKS; i++) {
                    if (pose == poseMap[i]) {
                        chainevent.next();
                        drink = i;
                    }
                }
                break;
            }
                
                
            case HIT: {
                
                pose = classifier.predict(sample);
                cout << pose << " drink: " << poseMap[drink] << endl;
                if (pose != poseMap[drink])chainevent.back();
                break;
            }
            case POUR: {
                cout << "POUR: " << drink << endl;
                // statements
                // open valve
                break;
            }
            case RESET: {
                // statements
                cout << "RESET" << endl;
                chainevent.setTo(PLAYING);
                break;
            }
            default:
                break;
        }
    }
    else {
        // i cant see anything!
        // cout << " i cant see anything!" << endl;
    }
    
    // cout << chainevent.getName() << " " <<int( chainevent.getTime()) << endl;
    theUser.update();

	ofEnableAlphaBlending();
	userFbo.begin();
	ofFill();
	ofSetColor(255, 255, 255, 40);
	ofDrawRectangle(0,0,userFbo.getWidth(), userFbo.getHeight());
	ofSetColor(255, 255, 255);
	ofNoFill();
	theUser.draw();

	userFbo.end();

}
void ofApp::reset() {
    classifier.clearTrainingInstances();
}
//--------------------------------------------------------------
void ofApp::draw() {
    
    learnedPoses.draw(0, 0);
    
    //for (auto user : users) {
        // user.print();
       // ofSetColor(255);
       // theUser.draw();
        
    //}
    ofPushMatrix();
    ofScale(5.f,5.f);
    ofDrawBitmapString("drink: "+ofToString(drink), 10, 10);
    ofDrawBitmapString("pose: "+ofToString(pose), 10, 30);
    ofPopMatrix();
    
    ofSetColor(255);
    
    switch (chainevent.getName()) {
        case BEGIN_LEARNING: {
            ofDrawRectangle(0, ofGetHeight()-50, (chainevent.getTime()/chainevent.getDuration())*ofGetWidth(), 50);
            break;
        }
        default:
            break;
    }
	userFbo.draw(0,0);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    if (key == 'l')chainevent.beginEvents();
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
    
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {
    
}
